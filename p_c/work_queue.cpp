#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "work_queue.h"

WorkQueue::WorkQueue(int size)
	:max_size_(size) {
	pthread_mutex_init(&mutex_, NULL);
	pthread_cond_init(&cond_read_, NULL);
	pthread_cond_init(&cond_write_, NULL);
	in_counter = 0;
	out_counter = 0;
	state_ = STATE_NONE;
	//mutex_ = PTHREAD_MUTEX_INITIALIZER;
	//cond_read_ = PTHREAD_COND_INITIALIZER;
	//cond_write_ = PTHREAD_COND_INITIALIZER;
}

WorkQueue::~WorkQueue(){
	pthread_mutex_destroy(&mutex_);
	pthread_cond_destroy(&cond_read_);
	pthread_cond_destroy(&cond_write_);
}

int WorkQueue::Pop() {
	pthread_mutex_lock(&mutex_);
	while(queue_.empty()) {
		state_ = STATE_EMPTY;
		pthread_cond_wait(&cond_read_, &mutex_);
	}
	int data = queue_.front();
	queue_.pop();
	++out_counter; 
	pthread_mutex_unlock(&mutex_);
	if(state_ == STATE_FULL && queue_.size() < max_size_/2) {
		pthread_cond_broadcast(&cond_write_);
		state_ = STATE_NONE;
	}
	//pthread_cond_signal(&cond_write_);
	return data;
}

void WorkQueue::Push(int data) {
	pthread_mutex_lock(&mutex_);
	while(queue_.size() >= max_size_) {
		state_ = STATE_FULL;
		pthread_cond_wait(&cond_write_, &mutex_);
	}
	queue_.push(data);
	++in_counter;
	pthread_mutex_unlock(&mutex_);
	if(state_ == STATE_EMPTY && queue_.size() > max_size_/2) {
		pthread_cond_broadcast(&cond_read_);
		state_ = STATE_NONE;
	}
	//pthread_cond_signal(&cond_read_);
}

int WorkQueue::size() {
	return queue_.size();
}

void WorkQueue::StartCounter() {
	while(1) {
		printf("write speed: %llu\n",in_counter);
		printf("read speed: %llu\n", out_counter);
		in_counter = 0;
		out_counter = 0;
		sleep(1);
	}
}
