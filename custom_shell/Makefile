
TARGET = program

# 소스 파일 목록
SRC = main.c path_utils.c cmd_cat.c cmd_help.c cmd_mkdir.c \
	cmd_rmdir.c cmd_cd.c cmd_kill.c cmd_ps.c cmd_run.c cmd_chmod.c \
	cmd_ln.c cmd_rename.c cmd_ls.c cmd_rm.c cmd_cp.c config.c

# 객체 파일 목록
OBJS = $(SRC:.c=.o)

# 컴파일러, 플래그
CC = gcc
CFLAGS = -Wall -Wextra -O0

# 기본 빌드 규칙(실행파일 만들기)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# 개별 .O 파일 생성 규칙
%.o: %.c custom_header.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

#실행파일 삭제
clean:
	rm -f $(TARGET) $(OBJS)
