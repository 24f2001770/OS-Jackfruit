#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// ---------------- USAGE ----------------
static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s start <id> <rootfs> <command>\n"
            "  %s ps\n"
            "  %s stop <id>\n"
            "  %s schedule\n",
            prog, prog, prog, prog);
}

// ---------------- START ----------------
static int cmd_start(int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr,
                "Usage: %s start <id> <rootfs> <command>\n",
                argv[0]);
        return 1;
    }

    char *id = argv[2];
    char *rootfs = argv[3];
    char *cmd = argv[4];

    pid_t pid = fork();

    if (pid == 0) {
        if (chroot(rootfs) != 0) {
            perror("chroot failed");
            exit(1);
        }

        chdir("/");
        execl(cmd, cmd, NULL);

        perror("exec failed");
        exit(1);
    }
    else if (pid > 0) {
        printf("Started container %s with PID %d\n", id, pid);

        FILE *f = fopen("/tmp/containers.txt", "a");
        if (f) {
            fprintf(f, "%s %d\n", id, pid);
            fclose(f);
        }

        FILE *log = fopen("runtime.log", "a");
        if (log) {
            fprintf(log, "START %s PID %d\n", id, pid);
            fclose(log);
        }
    }
    else {
        perror("fork failed");
        return 1;
    }

    return 0;
}

// ---------------- PS ----------------
static int cmd_ps(void)
{
    FILE *f = fopen("/tmp/containers.txt", "r");

    if (!f) {
        printf("No containers running\n");
        return 0;
    }

    char id[100];
    int pid;

    while (fscanf(f, "%s %d", id, &pid) == 2) {
        printf("Container: %s | PID: %d\n", id, pid);
    }

    fclose(f);
    return 0;
}

// ---------------- STOP ----------------
static int cmd_stop(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s stop <id>\n", argv[0]);
        return 1;
    }

    char *target = argv[2];

    FILE *f = fopen("/tmp/containers.txt", "r");
    if (!f) {
        printf("No containers\n");
        return 1;
    }

    char id[100];
    int pid;

    while (fscanf(f, "%s %d", id, &pid) == 2) {
        if (strcmp(id, target) == 0) {
            kill(pid, SIGKILL);
            printf("Stopped container %s (PID %d)\n", id, pid);

            FILE *log = fopen("runtime.log", "a");
            if (log) {
                fprintf(log, "STOP %s PID %d\n", id, pid);
                fclose(log);
            }
        }
    }

    fclose(f);
    return 0;
}

// ---------------- PRIORITY FUNCTION ----------------
int get_priority(char *id)
{
    if (strcmp(id, "c1") == 0) return 3;
    if (strcmp(id, "c2") == 0) return 2;
    if (strcmp(id, "c3") == 0) return 1;
    return 1; // default
}

// ---------------- SCHEDULER ----------------
static int cmd_schedule(void)
{
    FILE *f = fopen("/tmp/containers.txt", "r");
    if (!f) {
        printf("No containers to schedule\n");
        return 1;
    }

    char id[100];
    int pid;

    printf("Starting priority scheduler...\n");

    while (1) {
        rewind(f);

        while (fscanf(f, "%s %d", id, &pid) == 2) {

            int priority = get_priority(id);

            printf("Running %s (PID %d) priority %d\n",
                   id, pid, priority);

            kill(pid, SIGCONT);     // resume
            sleep(priority);        // higher priority → longer run
            kill(pid, SIGSTOP);     // pause
        }
    }

    fclose(f);
    return 0;
}

// ---------------- MAIN ----------------
int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "start") == 0)
        return cmd_start(argc, argv);

    if (strcmp(argv[1], "ps") == 0)
        return cmd_ps();

    if (strcmp(argv[1], "stop") == 0)
        return cmd_stop(argc, argv);

    if (strcmp(argv[1], "schedule") == 0)
        return cmd_schedule();

    usage(argv[0]);
    return 1;
}
