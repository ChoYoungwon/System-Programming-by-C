#include <stdio.h>

static int my_static(void)
{
	printf("static function\n");
}
int my_function(int arg)
{
	return (arg+1);
}

int main(int argc, char **argv)
{
	int i;
	for (i = 0; i < 10; i++);
	printf("hello, this is a message: %d\n", my_function(20));
	my_static();
	return 0;
}
