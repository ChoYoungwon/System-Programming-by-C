#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main()
{
	char func[1024];
	int ps;
	char *addr;
	int ret;

	ps = sysconf(_SC_PAGE_SIZE);
	addr = ((long)func)&~(ps-1);
	printf("%p, %p\n", func, addr);
	getc(stdin);

	ret = mprotect(addr, ps, PROT_READ|PROT_EXEC|PROT_WRITE);
	printf("mprotect() = %d\n", ret);

	getc(stdin);

	return 0;
}
