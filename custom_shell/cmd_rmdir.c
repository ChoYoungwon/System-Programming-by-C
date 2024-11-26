#include "config.h"
#ifdef ENABLE_CMD_RMDIR
#include "custom_header.h"


// rmdir 명령 실행 함수
void cmd_rmdir(int argc, char **argv)
{
    char *absolute = malloc(BUFSIZ);
    if(absolute == NULL) {
        perror("malloc");
        return;
    }
    if (argc == 2){
        // 절대 경로로 변환
        convert_to_absolute(argv[1], absolute);

        // 유효성 검사
        if(is_valid_path(absolute))
        {
            if(rmdir(absolute) == -1) {
                    perror("rmdir");
            }
        }
        else
            printf("You can't access upper directory\n");
    }
    else {
            printf("usage : rmdir <directory>\n");
    }
    free(absolute);
}

#endif
