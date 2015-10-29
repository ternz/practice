CC=g++
FLAG=-std=c++11 
OBJ=work_queue.o producter.o consumer.o
LIBS=-lpthread
main: main.cpp $(OBJ)
	$(CC) $(FLAG) -o $@ $< $(OBJ) $(LIBS)

work_queue.o: work_queue.cpp work_queue.h $(LIBS)
	$(CC) $(FLAG) -c $<
	
producter.o: producter.cpp producter.h $(LIBS)
	$(CC) $(FLAG) -c $<
	
consumer.o: consumer.cpp consumer.h $(LIBS)
	$(CC) $(FLAG) -c $<
	
clean:
	rm -f main $(OBJ)
