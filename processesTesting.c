#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROCESSES 1024

typedef struct {
    int pid;
    char name[256];
    unsigned long utime;
    unsigned long stime;
} Process;

int is_number(const char *s) {
    for (int i = 0; s[i]; i++) {
        if (!isdigit(s[i])) return 0;
    }
    return 1;
}

int compare_cpu(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    unsigned long cpu1 = p1->utime + p1->stime;
    unsigned long cpu2 = p2->utime + p2->stime;
    return cpu2 - cpu1;
}

int main() {
    DIR *proc = opendir("/proc");
    struct dirent *entry;

    Process processes[MAX_PROCESSES];
    int count = 0;

    while ((entry = readdir(proc)) != NULL && count < MAX_PROCESSES) {
        if (!is_number(entry->d_name)) continue;

        int pid = atoi(entry->d_name);
        char stat_path[256];
        sprintf(stat_path, "/proc/%d/stat", pid);

        FILE *stat_file = fopen(stat_path, "r");
        if (!stat_file) continue;

        Process p;
        p.pid = pid;
        fscanf(stat_file, "%*d (%[^)]) %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
               p.name, &p.utime, &p.stime);

        fclose(stat_file);
        processes[count++] = p;
    }

    closedir(proc);

    qsort(processes, count, sizeof(Process), compare_cpu);

    printf("%-6s %-25s %s\n", "PID", "COMMAND", "CPU TIME (jiffies)");
    for (int i = 0; i < 10 && i < count; i++) {
        printf("%-6d %-25s %lu\n", processes[i].pid, processes[i].name,
               processes[i].utime + processes[i].stime);
    }

    return 0;
}
