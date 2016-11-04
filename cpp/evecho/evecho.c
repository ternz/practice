#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <event2/event.h>

void on_accept(evutil_socket_t fd, short what, void* arg) {
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
			//create event
		}
	} else {
		fprintf(stderr, "accept unknown evern: %d\n", what);
		exit(1);
	}
	
}