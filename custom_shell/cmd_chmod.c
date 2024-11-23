#include "custom_header.h"

/*
	8진수 형식: 그대로 반환한다.
	문자열 형식 : 기존 권한에서 원하는 권한을 추가하거나 삭제한다.
*/
mode_t chmod_format(const char *mode_str, mode_t current_mode)
{
	regex_t regex;			// 정규표현식 사용을 위한 변수 할당
	int result, i = 0;
	mode_t permission = 0;
	char op;
	mode_t mode = current_mode;
	//  정규표현식 컴파일
	if (regcomp(&regex, "^[0-7]+$", REG_EXTENDED) != 0) {
		return 0;
	}

	result = regexec(&regex, mode_str, 0, NULL, 0);
	regfree(&regex);

	// 8진수 형태인 경우(매칭 성공)
	if (result == 0) {
		return (mode_t) strtol(mode_str, NULL, 8);
	}

	// 일반 문자열인 경우
	while(mode_str[i] != '\0' && (mode_str[i] == 'u' || mode_str[i] == 'g' || mode_str[i] == 'o')) {
		switch(mode_str[i]) {
			case 'u': permission |= S_IRUSR | S_IWUSR | S_IXUSR; break;
			case 'g': permission |= S_IRGRP | S_IWGRP | S_IXGRP; break;
			case 'o': permission |= S_IROTH | S_IWOTH | S_IXOTH; break;
			default: return (mode_t)-1;
		}
		i++;
	}
	if (permission == 0) {
		permission = 0777;
	}
	op = mode_str[i++];
	if (op != '+' && op != '-') {
		return (mode_t)-1;
	}
	while (mode_str[i] != '\0') {
		if(mode_str[i] == 'r' || mode_str[i] == 'w' || mode_str[i] == 'x') {
			switch(mode_str[i]) {
				case 'r':
					if (op == '+') mode |= permission & (S_IRUSR | S_IRGRP | S_IROTH);
					if (op == '-') mode &= ~(permission & (S_IRUSR | S_IRGRP | S_IROTH));
					break;
				case 'w':
                                        if (op == '+') mode |= permission & (S_IWUSR | S_IWGRP | S_IWOTH);
                                        if (op == '-') mode &= ~(permission & (S_IWUSR | S_IWGRP | S_IWOTH));
                                        break;
   				case 'x':
                                        if (op == '+') mode |= permission & (S_IXUSR | S_IXGRP | S_IXOTH);
                                        if (op == '-') mode &= ~(permission & (S_IXUSR | S_IXGRP | S_IXOTH));
                                        break;
				default:
					return -1;
			}
			i++;
		} else {
			return (mode_t)-1;
		}
	}
	return mode;
}

/*chmod 명령 함수*/
void cmd_chmod(int argc, char **argv)
{
	struct stat file_stat;
	char *absolute = malloc(BUFSIZ);
	if (absolute == NULL) {
		perror("malloc");
		return;
	}

	if (argc == 3) {
		convert_to_absolute(argv[2], absolute);
		if (stat(absolute, &file_stat) == -1) {
			perror("stat");
			free(absolute);
			return;
		}

		mode_t mode = chmod_format(argv[1], file_stat.st_mode);	// 문자열을 8진수 접근권환으로
		if (mode == (mode_t)-1) {
			printf("Invalid mode format\n");
			free(absolute);
			return;
		}
		if(is_valid_path(absolute)) {
			if(chmod(absolute, mode) == -1) {
				perror(absolute);
			}
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
	else {
		printf("usage : chmod <접근권한: 8진수><파일명>");
	}
	free(absolute);
}
