#include "custom_header.h"

// cat 명령을 실행하는 함수
void cmd_cat (int argc, char **argv)
{
    FILE *file;
    char ch;
    char *absolute = malloc(BUFSIZ);
	if (argc == 2) {
		convert_to_absolute(argv[1], absolute);		// 절대 경로로 변환
	    if (is_valid_path(absolute)) {
	        file = fopen(argv[1], "r");
	        if (file == NULL) {
                perror("file");
                free(absolute);
                return;
	        }
	        while((ch = fgetc(file)) != EOF) {
                putchar(ch);
	        }
	        fclose(file);
	    }
	    else {
	        printf("You can't access upper directory\n");
	    }
	}
	else {
	    printf("usage: cat <파일명>\n");
	}
	free(absolute);
}

