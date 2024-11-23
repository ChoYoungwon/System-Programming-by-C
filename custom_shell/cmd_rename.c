#include "custom_header.h"

// rename 명령 실행 함수
void cmd_rename(int argc, char **argv)
{
    char *old_absolute = malloc(BUFSIZ);
    char *new_absolute = malloc(BUFSIZ);

    if(old_absolute == NULL || new_absolute == NULL) {
        perror("malloc");
        return;
    }
    if (argc == 3) {
        // 절대 경로로 변환 
        convert_to_absolute(argv[1], old_absolute);
        convert_to_absolute(argv[2], new_absolute);

        // 유효성 검사
        if(is_valid_path(old_absolute) && is_valid_path(new_absolute))
        {
            if(rename(old_absolute, new_absolute) == -1) {
                perror("rename");
            }
        }
        else
            printf("You can't access upper directory\n");
        }
    else {
        printf("usage : rmdir <old_name> <new_name>\n");
    }
    free(old_absolute);
    free(new_absolute);
}
