#include <sys/mman.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void handler(int dummy) {
	;
}

int main() {
	key_t key;
	int shmid;
	void *shmaddr;
	char buf[1024];
	sigset_t mask;

	key = ftok("shmfile", 1);
	shmid = shmget(key, 1024, IPC_CREAT|0666);

	// SIGUSR1 시그널을 제외하고 모든 시그널을 블로킹
	sigfillset(&mask);										// 시그널 집합에 모든 시그널 설정
	sigdelset(&mask, SIGUSR1);								// 시그널 집합에서 SIGUSR1 제거
	signal(SIGUSR1, handler);								// SIGUSR1 처리

	printf("Listener wait for Talker\n");
	sigsuspend(&mask);										// 시그널 기다리기

	printf("Listener Start =====\n");
	shmaddr = shmat(shmid, NULL, 0);						// 공유 메모리 연결
	strcpy(buf, shmaddr);
	printf("Listener received : %s\n", buf);

	strcpy(shmaddr, "Have a nice day\n");
	sleep(3);
	shmdt(shmaddr);											//공유 메모리 연결 해제
}

