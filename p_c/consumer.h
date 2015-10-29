#ifndef _CONSUMER_H_
#define _CONSUMER_H_

#include <pthread.h>
#include "work_queue.h"

class Consumer {
	public:
	Consumer(const char *name, WorkQueue *queue);
	~Consumer();
	void Start();
	void Stop();
	static void* Run(void *arg);
private:
	pthread_t tid_;
	//bool stop_;
	WorkQueue *p_queue_;
	char name_[64];
};

#endif