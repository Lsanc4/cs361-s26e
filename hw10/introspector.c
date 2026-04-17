#define _GNU_SOURCE
#include <linux/perf_event.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int fd = -1;

static void perf_signal_handler(int sig, siginfo_t *info, void *ucontext) {
    if (info->si_code == POLL_HUP) {
        printf("Instruction count reached limit and event disabled.\n");
        // Optionally re-enable or exit
    } else if (info->si_code == POLL_IN) {
        printf("1,000,000 instructions reached (overflow signal).\n");
        // Re-arm for next interval if desired
        ioctl(info->si_fd, PERF_EVENT_IOC_REFRESH, 1);
    }
}

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    return syscall(SYS_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main(void) {
    struct perf_event_attr pe = {0};
    struct sigaction sa = {0};

    // Install signal handler
    sa.sa_sigaction = perf_signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGIO, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Configure the instructions event
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.sample_period = 100000000ULL;   // Overflow after ~1M instructions
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    // pe.wakeup_events = 1;         // Optional: force notification every overflow

    fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
        perror("perf_event_open");
        exit(EXIT_FAILURE);
    }

    // Enable asynchronous signal delivery on this fd
    fcntl(fd, F_SETFL, O_NONBLOCK | O_ASYNC);
    fcntl(fd, F_SETSIG, SIGIO);      // Deliver SIGIO on overflow
    fcntl(fd, F_SETOWN, getpid());   // Signal goes to this process

    // Reset and start counting
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);  // Enable + set initial refresh count

    printf("Monitoring started. Executing workload...\n");

    // Your workload here — the signal will interrupt when ~1M instructions complete
    volatile long long sum = 0;
    for (long long i = 0; i < 50000000LL; i++) {  // Long-running example
        sum += i;
        if(i%1000000==0) {
            printf("Got to %lld\n",i);
        }
    }

    // Cleanup (optional)
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    close(fd);

    return 0;
}