#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define R_BLOCK_SIZE 102400
#define W_BLOCK_SIZE 102400

int g_run = 1;

typedef struct {
	struct sockaddr_in *addr;
	int packsize;
} Args;

void * process(void *arg) {
	pthread_t tid = pthread_self();
	
	Args *args = arg;
	struct sockaddr_in *addr = args->addr;
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
	int rsize = args->packsize;
	int wsize = args->packsize;
	char *rbuf = (char *)malloc(rsize);
	char *wbuf = (char *)malloc(wsize);
	memset(wbuf, 'A', wsize);
	
	while(g_run) {
		int wn = 0;
		while(wsize > wn) {
			int n = write(sockfd, wbuf+wn, wsize-wn);
			if(n < 0) {
				printf("thread:%d write socket error: %s\n", tid, strerror(errno));
				return;
			} 
			wn += n;
		}
		
		//sleep(1);
		int rn = 0;
		while(rsize > rn) {
			int n = read(sockfd, rbuf+rn, rsize-rn);
			if(n < 0) {
				printf("thread:%d read socket error: %s\n", tid, strerror(errno));
				return;
			} else if(n == 0) {
				printf("thread:%d peer socket closed\n", tid);
				return;
			} else {
				rn += n;
			}
		}
		
		//sleep(1);
	}
	close(sockfd);
	free(rbuf);
	free(wbuf);
}

void usage() {
	printf("usage: %s <ip> <port> <thn> <size>\n");
	exit(0);
}

void handle_signal(int sig) {
	printf("wait for program exit\n");
	g_run = 0;
	sleep(2);
	printf("done\n");
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
	
	Args args;
	args.addr = &addr;
	args.packsize = atoi(argv[4]);
	
	int i;
	for(i=0; i < thn; i++) {
		if(pthread_create(&tids[i], NULL, process, &args) < 0) {
			printf("create thread error: %s", strerror(errno));
			exit(1);
		}
	}
	
	signal(SIGTERM, handle_signal);
	signal(SIGINT, handle_signal);
	
	for(i=0; i<thn; i++) {
		pthread_join(tids[i], NULL);
	}
	free(tids);
}