#define _XOPEN_SOURCE 500
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>

struct mymsgbuf {
    long mtype;
    char mtext[256];  // 경로 길이에 맞춰 조정 가능
};

// 재귀적으로 디렉토리 삭제를 위한 함수
int remove_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    int ret = remove(fpath);
    if (ret) {
        perror(fpath);
    }
    return ret;
}

// 파일&디렉토리 삭제 함수
int remove_file_or_directory(const char *path) {
    struct stat path_stat;

    if (stat(path, &path_stat) != 0) {
        perror("stat failed");
        return -1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        // 디렉토리인 경우
        if (rmdir(path) == 0) {
            // 빈 디렉토리 삭제 성공
            return 0;
        } else {
            // 디렉토리가 비어있지 않은 경우, 재귀적으로 삭제
            return nftw(path, remove_callback, 64, FTW_DEPTH | FTW_PHYS);
        }
    } else {
        // 일반 파일인 경우
        return unlink(path);
    }
}

int rename_file_or_directory(char *path) {
    char *old_name, *new_name, *saveptr;
    old_name = strtok_r(path, "$", &saveptr);
    new_name = strtok_r(NULL, "$", &saveptr);
    if (!old_name || !new_name) return -1;
    if (rename(old_name, new_name) == -1) {
        perror("rename");
        return -1;
    }
    return 0;
}

void send_response(int msgid, long mtype, const char *message) {

    struct mymsgbuf response;

    response.mtype = mtype;
    strncpy(response.mtext, message, sizeof(response.mtext) - 1);
    response.mtext[sizeof(response.mtext) - 1] = '\0';

    if (msgsnd(msgid, &response, sizeof(response.mtext), 0) == -1) {
        perror("msgsnd (response)");
    } else {
        printf("Response sent: %s\n", message);
    }
}

int move_directory(const char *src_path, const char *dest_path) {
    struct stat st;
    
    // 소스 디렉토리 정보 가져오기
    if (stat(src_path, &st) != 0) {
        return -1;
    }

    // 디렉토리인 경우
    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(src_path);
        if (!dir) {
            return -1;
        }

        // 목적지 디렉토리 생성
        mkdir(dest_path, st.st_mode);

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            // . 과 .. 는 건너뛰기
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // 소스와 목적지의 전체 경로 생성
            char src_buf[PATH_MAX];
            char dest_buf[PATH_MAX];
            snprintf(src_buf, sizeof(src_buf), "%s/%s", src_path, entry->d_name);
            snprintf(dest_buf, sizeof(dest_buf), "%s/%s", dest_path, entry->d_name);

            // 재귀적으로 이동
            if (move_directory(src_buf, dest_buf) != 0) {
                closedir(dir);
                return -1;
            }
        }
        closedir(dir);

        // 원본 디렉토리와 그 내용을 삭제
        return remove_file_or_directory(src_path);  // rmdir 대신 remove_file_or_directory 사용
    } else {
        // 파일인 경우 rename으로 이동 시도
        if (rename(src_path, dest_path) == 0) {
            return 0;
        }
        
        // rename 실패 시 복사 후 삭제
        FILE *src = fopen(src_path, "rb");
        if (!src) return -1;

        FILE *dst = fopen(dest_path, "wb");
        if (!dst) {
            fclose(src);
            return -1;
        }

        char buf[8192];
        size_t size;
        while ((size = fread(buf, 1, sizeof(buf), src)) > 0) {
            if (fwrite(buf, 1, size, dst) != size) {
                fclose(src);
                fclose(dst);
                unlink(dest_path);
                return -1;
            }
        }

        fclose(src);
        fclose(dst);
        
        // 원본 파일 삭제
        return unlink(src_path);
    }
}

// 복사 기능
int copy_file(const char *src_path, const char *dest_path) {
    struct stat st;
    if (stat(src_path, &st) != 0) {
        return -1;
    }

    // 파일인 경우
    if (S_ISREG(st.st_mode)) {
        FILE *src = fopen(src_path, "rb");
        if (!src) return -1;

        FILE *dst = fopen(dest_path, "wb");
        if (!dst) {
            fclose(src);
            return -1;
        }

        char buf[8192];
        size_t size;
        while ((size = fread(buf, 1, sizeof(buf), src)) > 0) {
            if (fwrite(buf, 1, size, dst) != size) {
                fclose(src);
                fclose(dst);
                unlink(dest_path);
                return -1;
            }
        }

        fclose(src);
        fclose(dst);

        // 원본 파일의 권한을 복사본에도 적용
        chmod(dest_path, st.st_mode);
        return 0;
    }
    // 디렉토리인 경우
    else if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(src_path);
        if (!dir) {
            return -1;
        }

        // 목적지 디렉토리 생성
        mkdir(dest_path, st.st_mode);

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char src_buf[PATH_MAX];
            char dest_buf[PATH_MAX];
            snprintf(src_buf, sizeof(src_buf), "%s/%s", src_path, entry->d_name);
            snprintf(dest_buf, sizeof(dest_buf), "%s/%s", dest_path, entry->d_name);

            if (copy_file(src_buf, dest_buf) != 0) {
                closedir(dir);
                return -1;
            }
        }
        closedir(dir);
        return 0;
    }

    return -1;
}

int main() {
    struct mymsgbuf buf;
    key_t key, response_key;
    int msgid, response_msgid;

    // 명령 수신용 메시지 큐
    if (access("command_keyfile", F_OK) == -1) {
        FILE *fp = fopen("command_keyfile", "w");
        if (!fp) {
            perror("Failed to create keyfile");
            exit(1);
        }
        fclose(fp);
    }

    key = ftok("command_keyfile", 1);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msgid = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

	// 응답 전숑용 메시지 큐
	if (access("response_keyfile", F_OK) == -1) {
        FILE *fp = fopen("response_keyfile", "w");
        if (!fp) {
            perror("Failed to create response_keyfile");
            exit(1);
        }
        fclose(fp);
    }

    response_key = ftok("response_keyfile", 1);
    if (response_key == -1) {
        perror("ftok (response)");
        exit(1);
    }

    response_msgid = msgget(response_key, IPC_CREAT | 0666);
    if (response_msgid == -1) {
        perror("msgget (response)");
        exit(1);
    }

    printf("Backend process started. Waiting for delete requests...\n");

    while (1) {
        if (msgrcv(msgid, &buf, sizeof(buf.mtext), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        // 종료 신호 확인
        if (buf.mtype == 1 && strcmp(buf.mtext, "QUIT") == 0) {
            printf("Received quit signal. Exiting...\n");
            break;
        }

        printf("Received delete request for: %s\n", buf.mtext);

		// 삭제 메시지 수신
		if (buf.mtype == 2) {
	        if (remove_file_or_directory(buf.mtext) == 0) {
	            printf("Successfully deleted: %s\n", buf.mtext);
				send_response(response_msgid, 10, "success");
	        } else {
	            printf("Failed to delete: %s\n", buf.mtext);
	        }
		}
		// 디렉토리 생성 메시지 수신
		else if (buf.mtype == 3) {
			size_t len = strlen(buf.mtext);
			char *file = malloc(len + 2);
			if (!file) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			strncpy(file, buf.mtext, len);
			file[len] = (char)('0');
			file[len +1] = '\0';
			int count = 0;
			while (count < 11) {
	        	if (mkdir(file, 0755) == 0) {
		            free(file);
					send_response(response_msgid, 10, "success");
		            break; // 성공
		        } else {
		            if (errno == EEXIST) {
		                file[len] = '0' + (count % 10);
		                count++;
		            } else {
		                perror("mkdir");
		                free(file);
						break;
		            }
		        }
		    }
			printf("Failed to mkdir: %s\n", buf.mtext);
		}
		else if (buf.mtype == 4) {
			if (rename_file_or_directory(buf.mtext) == 0) {
                printf("Successfully renamed: %s\n", buf.mtext);
				send_response(response_msgid, 10, "success");
            } else {
                printf("Failed to rename: %s\n", buf.mtext);
            }
		}

		else if (buf.mtype == 5) {  // 이동 작업
		    char *src_path = strtok(buf.mtext, "$");
		    char *dest_path = strtok(NULL, "$");
		    if (src_path && dest_path) {
		        // 대상 디렉토리 존재 확인
		        char *dest_dir = dirname(strdup(dest_path));
		        struct stat st;

		        if (stat(dest_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
		            // rename을 먼저 시도하고 실패할 경우 move_directory 사용
		            if (rename(src_path, dest_path) == 0) {
		                printf("Successfully moved: %s -> %s\n", src_path, dest_path);
		                send_response(response_msgid, 10, "success");
		            } else {
		                // rename 실패 시 move_directory 함수 사용
		                if (move_directory(src_path, dest_path) == 0) {
		                    printf("Successfully moved using copy: %s -> %s\n", src_path, dest_path);
		                    send_response(response_msgid, 10, "success");
		                } else {
		                    perror("Failed to move file/directory");
		                    send_response(response_msgid, 10, "failed");
		                }
		            }
		        } else {
		            printf("Destination directory does not exist: %s\n", dest_dir);
		            send_response(response_msgid, 10, "failed");
		        }
		        free(dest_dir);
		    }
		}

		// main 함수의 while 루프에 추가
		else if (buf.mtype == 6) {  // 복사 작업
		    char *src_path = strtok(buf.mtext, "$");
		    char *dest_path = strtok(NULL, "$");

		    if (src_path && dest_path) {
		        char *dest_dir = dirname(strdup(dest_path));
		        struct stat st;

		        if (stat(dest_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
		            if (copy_file(src_path, dest_path) == 0) {
		                printf("Successfully copied: %s -> %s\n", src_path, dest_path);
		                send_response(response_msgid, 10, "success");
		            } else {
		                perror("Failed to copy");
		                send_response(response_msgid, 10, "failed");
		            }
		        } else {
		            printf("Destination directory does not exist: %s\n", dest_dir);
		            send_response(response_msgid, 10, "failed");
		        }
		        free(dest_dir);
		    }
		}
		else if (buf.mtype == 7) {  // chmod 작업
		    char *path = strtok(buf.mtext, "$");
		    char *mode_str = strtok(NULL, "$");

		    if (path && mode_str) {
		        // 8진수 문자열을 mode_t로 변환
		        char *endptr;
		        mode_t mode = (mode_t)strtol(mode_str, &endptr, 8);
		        if (*endptr == '\0' && chmod(path, mode) == 0) {
		            printf("Successfully changed permissions for %s to %s\n", path, mode_str);
		            send_response(response_msgid, 10, "success");
		        } else {
		            perror("Failed to change permissions");
		            send_response(response_msgid, 10, "failed");
		        }
		    }
		}
		else if (buf.mtype == 8) {  // 심볼릭 링크 생성
		    char *src_path = strtok(buf.mtext, "$");
		    char *dest_path = strtok(NULL, "$");
		    
		    if (src_path && dest_path) {
		        // /tmp/link 디렉토리가 없으면 생성
		        struct stat st = {0};
		        if (stat("/tmp/link", &st) == -1) {
		            mkdir("/tmp/link", 0755);
		        }
		        if (symlink(src_path, dest_path) == 0) {
		            printf("Successfully created symlink: %s -> %s\n", dest_path, src_path);
		            send_response(response_msgid, 10, "success");
		        } else {
		            if (errno == EEXIST) {
		                unlink(dest_path);  // 기존 링크 삭제
		                if (symlink(src_path, dest_path) == 0) {
		                    printf("Successfully updated symlink: %s -> %s\n", dest_path, src_path);
		                    send_response(response_msgid, 10, "success");
		                } else {
		                    perror("Failed to create symlink");
		                    send_response(response_msgid, 10, "failed");
		                }
		            } else {
		                perror("Failed to create symlink");
		                send_response(response_msgid, 10, "failed");
		            }
		        }
		    }
		}
    }
    return 0;
}
