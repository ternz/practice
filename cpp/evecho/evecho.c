#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <event2/event.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

void on_accept(evutil_socket_t fd, short what, void* arg);
void on_read(evutil_socket_t fd, short what, void* arg);
void on_write(evutil_socket_t fd, short what, void* arg);
void on_signal(evutil_socket_t fd, short what, void* arg);

int g_run = 1;

void setnonblock(int fd) {
	int flags;
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags) == -1 ) {
		fprintf(stderr, "set fd nonblock error: %s\n", strerror(errno));
		exit(1);
	}
}

struct event_pair {
	struct event *rev;
	struct event *wev;
	
	struct iobuffer *write_buf;
};

#define FREE_PAIR(pair) do{\
	event_free((pair)->rev);\
	event_free((pair)->wev);\
	free(pair);\
}while(0)

struct iobuffer {
	char *base;
	char *curr;
	int size;
	int length;
};

#define BUFPTR(buf) (buf)->curr
#define BUFLEN(buf) (buf)->length
#define BUF_FORWARD(buf, len)  (buf)->curr += len;(buf)->length -= len

struct iobuffer * new_iobuffer(int len) {
	struct iobuffer *buf = malloc(sizeof(struct iobuffer));
	if(buf == NULL) return buf;
	buf->base = malloc(len);
	if(buf->base == NULL) {
		free(buf);
		return NULL;
	}
	buf->curr = buf->base;
	buf->size = len;
	buf->length = 0;
	return buf;
}

#define del_iobuffer(buf) free((buf)->base);free(buf)

//arg is a event_base
void on_accept(evutil_socket_t fd, short what, void* arg) {
	struct event_base *evbase = arg;
	if(what & EV_READ) {
		int connfd = accept(fd, NULL, NULL);
		if(connfd == -1) {
			if(errno == EAGAIN) return;
			else {
				fprintf(stderr, "accept error: %s\n", strerror(errno));
				exit(1);
			}
		}
		else {
			setnonblock(connfd);
			
			struct event_pair *pair = malloc(sizeof(struct event_pair));
			pair->rev = event_new(evbase, connfd, EV_READ | EV_PERSIST, on_read, pair);
			if(pair->rev == NULL) {
				fprintf(stderr, "create read event failed\n");
				exit(1);
			}
			pair->wev = event_new(evbase, connfd, EV_WRITE, on_write, pair);
			if(pair->wev == NULL) {
				fprintf(stderr, "create write event failed\n");
				exit(1);
			}
			event_add(pair->rev, NULL);
		}
	} else {
		fprintf(stderr, "accept unknown event: %d\n", what);
		exit(1);
	}
}

void active_write(struct event_pair *pair, char *buffer, int size, int length, int offset) {
	struct iobuffer *wbuf = malloc(sizeof(struct iobuffer));
	if(wbuf == NULL) {
		fprintf(stderr, "allocat iobuffer memory error\n");
		exit(1);
	}
	wbuf->base = buffer;
	wbuf->curr = buffer+offset;
	wbuf->size = size;
	wbuf->length = length;
	pair->write_buf = wbuf;
	int ret;
	ret = event_del(pair->rev);
	if(ret < 0) {
		fprintf(stderr, "delete read event failed\n");
		exit(1);
	}
	ret = event_add(pair->wev, NULL);
	if(ret < 0) {
		fprintf(stderr, "add write event failed\n");
		exit(1);
	}
}

//arg is a event_pair
void on_read(evutil_socket_t fd, short what, void* arg) {
	struct event_pair *pair = arg;
	if(what & EV_READ) {
		size_t buf_size = 128;
		char *buf = malloc(buf_size);
		int rn,wn;
		while(1) {
			rn = read(fd, buf, buf_size);
			if(rn < 0) {
				if(errno == EAGAIN) {
					free(buf);
					return;
				} else {
					fprintf(stderr, "read socket error. fd:%d, msg:%s\n", fd, strerror(errno));
					free(buf);
					//event_free(pair->rev);
					FREE_PAIR(pair);
					close(fd);
					return;
				}
			} else if(rn == 0) {
				printf("peer has closed socket %d\n", fd);
				free(buf);
				//event_free(pair->rev);
				FREE_PAIR(pair);
				close(fd);
				return;
			} else {
				wn = write(fd, buf, rn);
				if(wn < 0) {
					if(errno == EAGAIN) {
						active_write(pair, buf, buf_size, rn, 0);
						return;
					} else {
						fprintf(stderr, "read socket error. fd:%d, msg:%s\n", fd, strerror(errno));
						free(buf);
						//event_free(pair->rev);
						FREE_PAIR(pair);
						close(fd);
						return;
					}
				} else if(wn < rn) {
					active_write(pair, buf, buf_size, rn-wn, wn);
					return;
				} else {
					//do nothing
				}
			}
		}
	} else {
		fprintf(stderr, "read unknown event: %d\n", what);
		//close fd
		//event_free(pair->rev);
		FREE_PAIR(pair);
		close(fd);
	}
}

void on_write(evutil_socket_t fd, short what, void* arg) {
	struct event_pair *pair = arg;
	int wn = write(fd, pair->write_buf->curr, pair->write_buf->length);
	if(wn < 0) {
		if(errno == EAGAIN) return;
		else {
			fprintf(stderr, "read socket error. fd:%d, msg:%s\n", fd, strerror(errno));
			del_iobuffer(pair->write_buf);
			FREE_PAIR(pair);
			close(fd);
			return;
		}
	} else if(wn < pair->write_buf->length) {
		BUF_FORWARD(pair->write_buf, wn);
		event_add(pair->wev, NULL);
	} else {
		del_iobuffer(pair->write_buf);
		event_add(pair->rev, NULL);
	}
}

void on_signal(evutil_socket_t fd, short what, void* arg) {
	struct event_base *base = arg;
	switch(fd) {
		case SIGTERM: 
			event_base_loopbreak(base);
			break;
		//default:
	}
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		return;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(atoi(argv[1]));
	
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	setnonblock(listen_fd);
	
	if(listen_fd < 0) {
		fprintf(stderr, "crate lisent socket error: %s", strerror(errno));
		exit(1);
	}
	if(bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "bind addr error: %s\n", strerror(errno));
		exit(1);
	}
	if(listen(listen_fd, 10) < 0) {
		fprintf(stderr, "socket listen error: %s\n", strerror(errno));
		exit(1);
	}
	
	struct event_base *evbase = event_base_new();
	struct event *evlisten = event_new(evbase, listen_fd, EV_READ | EV_PERSIST, on_accept, evbase);
	if(event_add(evlisten, NULL) < 0) {
		fprintf(stderr, "add listen event error\n");
		exit(1);
	}
	
	struct event *evsighup = event_new(evbase, SIGHUP, EV_SIGNAL | EV_PERSIST, on_signal, evbase);
	struct event *evsigterm = event_new(evbase, SIGTERM, EV_SIGNAL | EV_PERSIST, on_signal, evbase);
	if(event_add(evsighup, NULL) < 0 || event_add(evsigterm, NULL) < 0) {
		fprintf(stderr, "add signal event error\n");
		exit(1);
	}
	
	event_base_dispatch(evbase);
}