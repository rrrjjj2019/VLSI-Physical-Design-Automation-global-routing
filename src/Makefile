CC=g++
CFLAGS=-Wall -std=c++11 -Ofast
# HEADERS=cell_in_partition.h partition.h
# SOURCES=UnbalanceException.cpp PartitionSizeZeroException.cpp CellNotFoundException.cpp cell_in_partition.cpp main.cpp
# OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=../bin/hw5

$(EXECUTABLE): main.o
	$(CC) $(CFLAGS) -o $(EXECUTABLE) main.o

main.o: main.cpp globalEdge.h net.h
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o $(EXECUTABLE)
