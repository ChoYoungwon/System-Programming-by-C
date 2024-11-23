#include "custom_header.h"

// 상대 경로를 절대 경로로 변환하는 함수
void convert_to_absolute(const char *relative_path, char *absolute_path)
{
	char * current_path;
	current_path = getcwd(NULL, BUFSIZ);
	char *entire_path = malloc((strlen(relative_path)+strlen(current_path)+3));
	if(entire_path == NULL) {
	        perror("malloc");
	        return;
	}

	// absolute_path를 빈 문자열로 초기화한다.
	strcpy(absolute_path, "");

	// 상대 경로로 접근한 경우
	if (*relative_path != '/')
	{
		strcpy(entire_path, current_path);
		strcat(entire_path, "/");
		strcat(entire_path, relative_path);
	}
	// 절대 경로로 접근한 경우
	else {
		strcpy(entire_path, BASE_PATH);
		strcat(entire_path, relative_path);
	}
	// 함친 문자열을 절대 경로로 변환한다.
	char *token = strtok(entire_path, "/");
	while (token != NULL) {
	        if (strcmp(token, "..") == 0) {
	            // absolute_path 문자열의 마지막 / 이후의 경로를 지운다
	            char *slash = strrchr(absolute_path, '/');
	            if (slash != NULL && slash != absolute_path) {
	                *slash = '\0';
	            }
	        } else if (strcmp(token, "") != 0 && strcmp(token, ".") != 0) {
		    // absolute_path에 /'token'을 추가한다
	            strcat(absolute_path, "/");
	            strcat(absolute_path, token);
	        }
	        token = strtok(NULL, "/");
	}
	free(entire_path);
	// printf("implement_absolute_path : %s\n", absolute_path);
}

// 유효성 검사 함수 (경로가 상위 디렉토리에 접근하는지 확인)
int is_valid_path(const char *path)
{
	return (strncmp(path, BASE_PATH, strlen(BASE_PATH)) == 0);
}
