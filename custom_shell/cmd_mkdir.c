#include "config.h"
#ifdef ENABLE_CMD_MKDIR
#include "custom_header.h"

// mkdir 명령 실행 함수
void cmd_mkdir(int argc, char **argv)
{
	int n;
	mode_t mode = 0755;
	char *endptr;

	// getopt 상태 초기화
	optind = 0;

	//getopt로 옵션 확인
	while ((n = getopt(argc, argv, "m:")) != -1) {
		switch (n) {
		case 'm':
			errno = 0;
			mode = strtol(optarg, &endptr, 8);
			if (errno != 0 || *endptr != '\0' || endptr == optarg) {
				perror("Invalid mode");
				return;
			}
			break;
		}
	}


	if (optind >= argc) {
		printf("usage: mkdir [-m mode] <directory>\n");
		return;
	}

	char *absolute = malloc(BUFSIZ);
	if(absolute == NULL) {
        perror("malloc");
        return;
	} 

   convert_to_absolute(argv[optind], absolute); // 절대 경로 변환

	// 유효성 검사
    if(is_valid_path(absolute)) {
		mode_t old_umask = umask(0);

		if(mkdir(absolute, mode) == -1) {
	        perror("mkdir");
        } else {
			if (chmod(absolute, mode) == -1) {
				perror("chmod");
			}
		}

		umask(old_umask);
    }
    else {
        printf("You can't access upper directory\n");
    }

	// printf("Debug: optind = %d, optarg = %s\n", optind, optarg);
	free(absolute);
}

#endif
