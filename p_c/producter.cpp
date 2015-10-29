#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "producter.h"

int AtomCounter::counter_ = 0;
pthread_mutex_t AtomCounter::mutex_ = PTHREAD_MUTEX_INITIALIZER;

int AtomCounter::operator++(int) {
	int val = counter_;
	pthread_mutex_lock(&mutex_);
	++ counter_;
	pthread_mutex_unlock(&mutex_);
	return val;
}

Producter::Producter(const char *name, WorkQueue *queue)
	:p_queue_(queue) {
	snprintf(name_, sizeof(name_), "%s", name);
}

Producter::~Producter() {
	
}

void Producter::Start() {
	int rat = pthread_create(&tid_, NULL, Run, (void *)this);
	if(rat != 0) {
		perror("Can't create thread \n");
	}
	//stop_ = false;
}

void Producter::Stop() {
	//stop_ = true;
	pthread_cancel(tid_);
}

void* Producter::Run(void *arg) {
	Producter *pthis = (Producter *)arg;
	while(1) {
		int data = pthis->counter_++;
		pthis->p_queue_->Push(data);
		//printf("%s product data %d\n", pthis->name_, data);
		//sleep(1);
	}
}