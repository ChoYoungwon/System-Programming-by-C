#include "config.h"
#ifdef ENABLE_CMD_CP
#include "custom_header.h"

// 원본 파일을 타겟 파일에 복사시키는 함수
static void copy_file(const char *source, const char *target)
{
	FILE *src = fopen(source, "rb");
	if (src == NULL) {
		perror("Error opening source file");
		return;
	}
	FILE *tar = fopen(target, "wb");
	if (tar == NULL) {
		perror("Error opening source file");
		fclose(src);
		return;
	}
	char buffer[BUFSIZ];
	size_t byte;

	while((byte = fread(buffer, 1, sizeof(buffer), src)) > 0) {
		fwrite(buffer, 1, byte, tar);
	}
	fclose(src);
	fclose(tar);
}

// cp 명령을 처리하는 함수 
void cmd_cp (int argc, char **argv)
{
	char *absolute_origin = malloc(BUFSIZ);
	char *absolute_new = malloc(BUFSIZ);

	if (argc == 3) {
		convert_to_absolute(argv[1], absolute_origin);
		convert_to_absolute(argv[2], absolute_new);
		if (is_valid_path(absolute_origin) && is_valid_path(absolute_new)) {
			copy_file(absolute_origin, absolute_new);
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
	else {
		printf("usage: cp <original file> <new file>\n");
	}
}

#endif
