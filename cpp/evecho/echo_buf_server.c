#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

//#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>

void buffer_read_cb(struct bufferevent *bev, void *ctx) {
	struct evbuffer *evb_in = bufferevent_get_input(bev);
	/*struct evbuffer *evb_out = bufferevent_get_output(bev);
	if(evbuffer_add_buffer(evb_out, evb_in) < 0) {
		fprintf(stderr, "evbuffer_add_buffer failed\n");
		//stop bufferevent
		bufferevent_free(bev);
	}*/
	if(bufferevent_write_buffer(bev, evb_in) < 0) {
		fprintf(stderr, "bufferevent_write_buffer failed\n");
		//stop bufferevent
		bufferevent_free(bev);
	}
}

void buffer_event_cb(struct bufferevent *bev, short events, void *ctx) {
	if(events & BEV_EVENT_ERROR) {
		int err = EVUTIL_SOCKET_ERROR();
		fprintf(stderr, "bufferevent error: %s\n", evutil_socket_error_to_string(err));
		bufferevent_free(bev);
	} else if(events & BEV_EVENT_EOF) {
		printf("peer socket closed: %d\n", bufferevent_getfd(bev));
		bufferevent_free(bev);
	} else {
		printf("unknow event %d\n", events);
	}
}

void listen_accept_cb(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr) {
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, buffer_read_cb, NULL, buffer_event_cb, NULL);
	
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void listen_error_cb(struct evconnlistener *listener, void *ptr) {
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Got an error %d (%s) on the listener.\n"
		"Shutting down.\n", err, evutil_socket_error_to_string(err));
	event_base_loopexit(base, NULL);
}

void on_signal(evutil_socket_t fd, short what, void* arg) {
	struct event_base *base = arg;
	switch(fd) {
		case SIGTERM: 
		case SIGINT:
			event_base_loopbreak(base);
			break;
		//default:
	}
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		return 0;
	}
	
	struct event_base *base = event_base_new();
	if(base == 0) {
		printf("new event_base failed\n");
		return 1;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(atoi(argv[1]));
	
	struct evconnlistener *listener = evconnlistener_new_bind(base, listen_accept_cb, NULL, 
		LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, 10,
		(struct sockaddr *)&addr, sizeof(addr));
	
	if(listener == NULL) {
		fprintf(stderr, "new ev listener error\n");
		return 1;
	}
	
	evconnlistener_set_error_cb(listener, listen_error_cb);
	
	struct event *evsighup = event_new(base, SIGHUP, EV_SIGNAL | EV_PERSIST, on_signal, base);
	struct event *evsigterm = event_new(base, SIGTERM, EV_SIGNAL | EV_PERSIST, on_signal, base);
	struct event *evsigint = event_new(base, SIGINT, EV_SIGNAL | EV_PERSIST, on_signal, base);
	struct event *evsigpipe = event_new(base, SIGPIPE, EV_SIGNAL | EV_PERSIST, on_signal, base);
	if(event_add(evsighup, NULL) < 0 || event_add(evsigterm, NULL) < 0 
		|| event_add(evsigint, NULL) < 0 || event_add(evsigpipe, NULL) < 0) {
		fprintf(stderr, "add signal event error\n");
		return 1;
	}
	
	event_base_dispatch(base);
	
	return 0;
}