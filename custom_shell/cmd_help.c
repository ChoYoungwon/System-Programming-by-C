# include "custom_header.h"

/* help 명렁 실행 함수*/
/* help 명령 실행 함수 */
void cmd_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Available commands:\n");
    printf("---------------------------------------------------------------\n");
    printf("cd <path>               : Change to the specified directory.\n");
    printf("mkdir [-m mode] <directory> : Create a new directory at the specified path.\n");
    printf("rmdir <path>            : Remove an empty directory at the specified path.\n");
    printf("rename <source> <target>: Rename a file or directory from source to target.\n");
    printf("ls [-al] <path>         : List the contents of the current directory.\n");
    printf("ln <original> <link>    : Create a hard link from original to link.\n");
    printf("rm <path>              	: Remove a file at the specified path.\n");
    printf("cat <filename>         	: Display the contents of the specified file.\n");
    printf("cp <source> <destination>: Copy a file from source to destination.\n");
    printf("ps [-aux]               : Display a list of currently running processes.\n");
    printf("kill <pid>              : Terminate a process by its process ID (PID).\n");
    printf("help                    : Show this help message.\n");
    printf("---------------------------------------------------------------\n");
}

