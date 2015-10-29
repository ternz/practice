#ifndef _WORK_QUEUE_H_
#define _WORK_QUEUE_H_

#include <pthread.h>
#include <queue>

class WorkQueue {
public:
	WorkQueue(int size);
	~WorkQueue();
	int Pop();
	void Push(int);
	int size();
	void StartCounter();
private:
	pthread_mutex_t mutex_;
	pthread_cond_t cond_read_;
	pthread_cond_t cond_write_;
	std::queue<int> queue_;
	int max_size_;
	unsigned long long in_counter;
	unsigned long long out_counter;
};

#endif