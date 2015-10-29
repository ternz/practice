#ifndef _PRODUCTER_H_
#define _PRODUCTER_H_

#include <pthread.h>
#include "work_queue.h"

class AtomCounter {
private:
	static int counter_;
	static pthread_mutex_t mutex_;
public:
	int operator++(int);
};

class Producter {
public:
	Producter(const char *name, WorkQueue *queue);
	~Producter();
	void Start();
	void Stop();
	static void* Run(void *arg);
private:
	AtomCounter counter_;
	pthread_t tid_;
	//bool stop_;
	WorkQueue *p_queue_;
	char name_[64];
};

#endif