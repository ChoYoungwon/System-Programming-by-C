#include "config.h"
#ifdef ENABLE_CMD_RM
#include "custom_header.h"

// 파일 삭제 명령(rm) 
void cmd_rm(int argc, char **argv)
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
            if(unlink(absolute) == -1) {
                    perror("rm");
            }
        }
        else
            printf("You can't access upper directory\n");
    }
    else {
        printf("usage : rm <directory>\n");
    }
    free(absolute);
}

#endif
