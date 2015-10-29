#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "consumer.h"

Consumer::Consumer(const char *name, WorkQueue *queue)
	:p_queue_(queue) {
	snprintf(name_, sizeof(name_), "%s", name);
}

Consumer::~Consumer() {
	
}

void Consumer::Start() {
	int ret = pthread_create(&tid_, NULL, Run, (void*)this);
	if(ret != 0) {
		perror("Can't create thread \n");
	}
}

void Consumer::Stop() {
	pthread_cancel(tid_);
}

void* Consumer::Run(void *arg) {
	Consumer *pthis = (Consumer *)arg;
	while(1) {
		int data = pthis->p_queue_->Pop();
		//printf("%s consum data %d\n", pthis->name_, data);
		//sleep(1);
	}
}