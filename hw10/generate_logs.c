#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_entries> <output_file> [seed]\n", argv[0]);
        return 1;
    }
    int num = atoi(argv[1]);
    unsigned seed = argc > 3 ? atoi(argv[3]) : time(NULL);
    srand(seed);
    FILE *f = fopen(argv[2], "w");
    if (!f) return 1;
    const char *severities[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
    const char *events[] = {"LOGIN", "LOGOUT", "QUERY", "UPDATE", "ERROR"};
    for (int i = 0; i < num; i++) {
        time_t t = time(NULL) - rand() % 86400;
        struct tm *tm = localtime(&t);
        char ts[64];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
        fprintf(f, "%s %s %s 192.168.1.%d Sample message %d\n",
                ts, severities[rand()%5], events[rand()%5], rand()%255, i);
    }
    fclose(f);
    printf("Generated %d entries in %s\n", num, argv[2]);
    return 0;
}