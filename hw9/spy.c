#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <capstone/capstone.h>
#include <elfutils/libdwfl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

static pid_t child_pid = -1;
static char *child_exec = NULL;
static char **child_args = NULL;

static volatile sig_atomic_t interrupt_flag = 0;

/* Async-safe message for signal handler */
static void safe_print(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void sigint_handler(int sig) {
    (void)sig;
    interrupt_flag = 1;
    safe_print("[SIGINT] Handler fired - setting flag\n");
}

static ssize_t read_tracee_memory(pid_t pid, uintptr_t addr, void *buf, size_t len) {
    size_t i = 0;
    while (i < len) {
        errno = 0;
        long word = ptrace(PTRACE_PEEKTEXT, pid, (void *)(addr + i), NULL);
        if (errno != 0) return -1;
        size_t to_copy = (len - i < sizeof(long)) ? (len - i) : sizeof(long);
        memcpy((char *)buf + i, &word, to_copy);
        i += to_copy;
    }
    return (ssize_t)len;
}
static void print_function_name(pid_t pid, uintptr_t addr) {
    Dwfl *dwfl = NULL;
    Dwfl_Module *module = NULL;
    const char *func_name = NULL;
    const char *module_name = "unknown";

    static Dwfl_Callbacks callbacks = {
        .find_elf        = dwfl_linux_proc_find_elf,
        .find_debuginfo  = dwfl_standard_find_debuginfo
    };

    dwfl = dwfl_begin(&callbacks);
    if (!dwfl)
        goto out;

    /* Attach to the live process and report all loaded modules */
    if (dwfl_linux_proc_attach(dwfl, pid, true) != 0 ||
        dwfl_linux_proc_report(dwfl, pid) != 0) {
        goto out;
    }

    module = dwfl_addrmodule(dwfl, addr);
    if (module) {
        func_name = dwfl_module_addrname(module, addr);
        dwfl_module_info(module, NULL, NULL, NULL, NULL, NULL, &module_name, NULL);
    }

    if (func_name) {
        printf("Current function: %s (0x%lx) in %s\n",
               func_name, (unsigned long)addr, module_name);
    } else {
        printf("Current function: <unknown> (0x%lx) in %s\n",
               (unsigned long)addr, module_name);
    }

out:
    if (dwfl)
        dwfl_end(dwfl);
}

static void print_disassembly(pid_t pid, uintptr_t rip) {
    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        fprintf(stderr, "Capstone init failed\n");
        return;
    }
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);

    const size_t window = 256;
    uintptr_t start_addr = (rip > window / 2) ? rip - window / 2 : 0;
    uint8_t code[256];
    memset(code, 0, sizeof(code));

    if (read_tracee_memory(pid, start_addr, code, window) < 0) {
        fprintf(stderr, "Failed to read tracee memory\n");
        cs_close(&handle);
        return;
    }

    cs_insn *insn;
    size_t count = cs_disasm(handle, code, window, start_addr, 0, &insn);
    if (count == 0) {
        fprintf(stderr, "Disassembly failed\n");
        cs_close(&handle);
        return;
    }

    printf("\nDisassembly around 0x%lx:\n", (unsigned long)rip);
    int printed = 0;
    for (size_t i = 0; i < count && printed < 25; ++i) {
        if (insn[i].address > rip + 96) break;
        if (insn[i].address + insn[i].size < rip - 80) continue;

        printf("0x%016lx: %-30s %s\n",
               (unsigned long)insn[i].address,
               insn[i].mnemonic, insn[i].op_str);
        printed++;
    }

    cs_free(insn, count);
    cs_close(&handle);
}

static void run_tracer(void) {
    int status;
    struct user_regs_struct regs;

    /* Force initial stop */
    if (ptrace(PTRACE_INTERRUPT, child_pid, 0, 0) < 0)
        perror("PTRACE_INTERRUPT (initial)");

    printf("initial interrupt done\n");
    if (waitpid(child_pid, &status, 0) < 0) {
        perror("waitpid initial");
        return;
    }
    printf("waitpid passed\n");    
    // if (ptrace(PTRACE_SETOPTIONS, child_pid, 0,
    //            PTRACE_O_TRACEEXEC) < 0)
    //     perror("PTRACE_SETOPTIONS");

    if (ptrace(PTRACE_CONT, child_pid, 0, 0) < 0) {
        perror("PTRACE_CONT initial");
        return;
    }

    struct sigaction sa = { .sa_handler = sigint_handler, .sa_flags = SA_RESTART };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    printf("[DEBUG] SIGINT handler installed\n");

    printf("Spy attached (PID %d). Child is running freely.\n", child_pid);
    printf("Press Ctrl-C to inspect.\n\n");

    while (1) {
        /* Heartbeat so we know the loop is alive */
        static time_t last_beat = 0;
        time_t now = time(NULL);
        if (now != last_beat) {
            printf("[HEARTBEAT] Spy still running...\n");
            last_beat = now;
        }

        if (interrupt_flag) {
            printf("[DEBUG] Main loop saw interrupt_flag == 1\n");
            interrupt_flag = 0;

            if (ptrace(PTRACE_INTERRUPT, child_pid, 0, 0) < 0)
                perror("PTRACE_INTERRUPT");

            printf("did the interrupt\n");
            if (waitpid(child_pid, &status, 0) < 0) {
                perror("waitpid after INTERRUPT");
                break;
            }
            printf("waitpid done\n");

            if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGINT)
                printf("(SIGINT suppressed for child)\n");

            if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == 0) {
                uintptr_t rip = regs.rip;
                print_function_name(child_pid, rip);
                print_disassembly(child_pid, rip);
            } else {
                perror("PTRACE_GETREGS");
            }

            printf("\n--- Resuming child ---\n\n");
            ptrace(PTRACE_CONT, child_pid, 0, 0);
        }

        if (waitpid(child_pid, &status, WNOHANG) > 0) {
            if (WIFEXITED(status)) {
                printf("Child exited with status %d\n", WEXITSTATUS(status));
                break;
            }
            if (WIFSIGNALED(status)) {
                printf("Child terminated by signal %d\n", WTERMSIG(status));
                break;
            }
        }

        usleep(200000);   /* 0.2 s for responsive heartbeat */
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-h] <executable> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        if (opt == 'h') {
            printf("Usage: %s [-h] <executable> [args...]\n", argv[0]);
            return EXIT_SUCCESS;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: no executable specified\n");
        return EXIT_FAILURE;
    }

    child_exec = argv[optind];
    child_args = &argv[optind];

    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        setpgid(0, 0);
        execvp(child_exec, child_args);
        perror("execvp");
        _exit(1);
    }

    printf("trying to seize\n");

    if (ptrace(PTRACE_SEIZE, child_pid, 0, 0) < 0) {
        perror("PTRACE_SEIZE");
        return EXIT_FAILURE;
    }
    printf("seized\n");

    run_tracer();
    return EXIT_SUCCESS;
}