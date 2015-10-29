#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "work_queue.h"
#include "producter.h"
#include "consumer.h"
using namespace std;

void print_queue(WorkQueue *que)
{
	printf("queue size: %d\n", (int)que->size());
	/*queue<int>::iterator iter = que->begin();
	for(; iter!=que->end(); iter++)
	{
		printf("%d ", *iter);
	}
	printf("\n");*/
}

int main(int argc, char **argv)
{
	WorkQueue queue(1024*1024);
	
	int producter_num = 2;
	int consumer_num = 2;
	if(argc == 3) {
		producter_num = atoi(argv[1]);
		consumer_num = atoi(argv[2]);
	} 
	Producter *producters[producter_num];
	Consumer *consumers[consumer_num];
	
	for(int i=0; i<producter_num; i++) {
		producters[i] = new Producter("producter", &queue);
	}
	for(int i=0; i<consumer_num; i++) {
		consumers[i] = new Consumer("consumer", &queue);
	}
	for(int i=0; i<producter_num; i++) {
		producters[i]->Start();
	}
	for(int i=0; i<consumer_num; i++) {
		consumers[i]->Start();
	}
	
	queue.StartCounter();
	
	//char c = getchar();
	
	for(int i=0; i<producter_num; i++) {
		producters[i]->Stop();
	}
	for(int i=0; i<consumer_num; i++) {
		consumers[i]->Stop();
	}
	
	/*Producter producter1("producter1", &queue);
	Producter producter2("producter2", &queue);
	
	Consumer consumer1("consumer1", &queue);
	Consumer consumer2("consumer2", &queue);
	
	producter1.Start();
	producter2.Start();
	consumer1.Start();
	consumer2.Start();
	
	while(1) {
		sleep(2);
		print_queue(&queue);
	}*/
	
	//char = getchar();
	
	//producter1.Stop();
	//producter2.Stop();
	//consumer1.Stop();
	//consumer2.Stop();
	
	return 0;
}