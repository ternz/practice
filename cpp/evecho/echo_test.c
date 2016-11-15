#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define R_BLOCK_SIZE 1024
#define W_BLOCK_SIZE 1024

void * process(void *arg) {
	pthread_t tid = pthread_self();
	
	struct sockaddr_in *addr = arg;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		printf("thread:%d create socket failed: %s\n", tid, strerror(errno));
		return;
	}
	int ret;
	ret = connect(sockfd, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		printf("thread:%d socket connect failed: %s\n", tid, strerror(errno));
		return;
	}
	int rsize = R_BLOCK_SIZE;
	int wsize = W_BLOCK_SIZE;
	char *rbuf = malloc(rsize);
	char *wbuf = malloc(wsize);
	memset(wbuf, 0, wsize);
	
	int rn, wn=0;
	while(1) {
		while(wsize > wn) {
			int n = write(sockfd, wbuf+wn, wsize-rn);
			if(n < 0) {
				printf("thread:%d write socket error\n", tid);
				return;
			} 
			wn += n;
		}
		
		while(rsize > rn) {
			int n = read(sockfd, rbuf+rn, rsize-rn);
			if(n < 0) {
				printf("thread:%d read socket error: %s\n", tid, strerror(errno));
				return;
			} else if(n = 0) {
				printf("thread:%d peer socket closed\n", tid);
				return;
			} else {
				rn += n;
			}
		}
	}
	free(rbuf);
	free(wbuf);
}

void usage() {
	printf("usage: %s <ip> <port> <thn> <size>\n");
	exit(0);
}

void handle_signal(int sig) {
	printf("exit\n");
	exit(0);
}

int main(int argc, char **argv) {
	if(argc != 5) 
		usage();
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));
	
	int thn = atoi(argv[3]);
	pthread_t *tids = malloc(sizeof(pthread_t) * thn);
	
	int i;
	for(i=0; i < thn; i++) {
		if(pthread_create(&tids[i], NULL, process, &addr) < 0) {
			printf("create thread error: %s", strerror(errno));
			exit(1);
		}
	}
	
	//signal(SIGTERM, handle_signal);
	
	for(i=0; i<thn; i++) {
		pthread_join(tids[i], NULL);
	}
	free(tids);
}