#include "config.h"
#ifdef ENABLE_CMD_KILL
#include "custom_header.h"

// kill 명령 구현
void cmd_kill(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: kill [option] <PID>\n");
        return;
    }
    int sig = atoi(argv[1]) * (-1);
    pid_t pid = atoi(argv[2]);

    if (kill(pid, sig) == -1) {
        perror("kill");
    }
}

#endif
