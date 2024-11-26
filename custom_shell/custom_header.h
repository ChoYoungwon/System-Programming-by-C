#ifndef CUSTOM_HEADER_H
#define CUSTOM_HEADER_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <regex.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <sys/sysinfo.h>


#define MAX_CMD_SIZE    (128)
#define MAX_ARG		(4)
#define MAX_PATH 	(256)

// 매크로 함수 선언을 자동화
#define DECLARE_CMDFUNC(str)	void cmd_##str(int argc, char **argv);

// 함수 선언 
DECLARE_CMDFUNC(help);
DECLARE_CMDFUNC(mkdir);
DECLARE_CMDFUNC(rmdir);
DECLARE_CMDFUNC(rename);
DECLARE_CMDFUNC(cd);
DECLARE_CMDFUNC(ls);
DECLARE_CMDFUNC(ln);
DECLARE_CMDFUNC(rm);
DECLARE_CMDFUNC(chmod);
DECLARE_CMDFUNC(cat);
DECLARE_CMDFUNC(cp);
DECLARE_CMDFUNC(ps);
DECLARE_CMDFUNC(kill);
DECLARE_CMDFUNC(run);

// 작업 기본 경로
extern const char *BASE_PATH;

// getopt 관련 전역 변수 선언
extern char *optarg;
extern int optind, opterr, optopt;

// 상대경로를 절대경로로 변환 함수 선언
void convert_to_absolute(const char *relative_path, char *absolute_path);

// 상위 폴더 접근 제한 함수선언
int is_valid_path(const char *path);

#endif
