# Makefile for building the server and client applications

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
LDFLAGS = -lpthread # for POSIX threads

all: server client pipelineServer LFServer

server: server.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o
	$(CXX) $(CXXFLAGS) -o server server.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o $(LDFLAGS)

client: client.o
	$(CXX) $(CXXFLAGS) -o client client.o $(LDFLAGS)

pipelineServer: pipelineServer.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o Tree.o
	$(CXX) $(CXXFLAGS) -o pipelineServer pipelineServer.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o Tree.o $(LDFLAGS)

LFServer: LFServer.o Graph.o KruskalMST.o PrimMST.o ThreadPool.o Tree.o
	$(CXX) $(CXXFLAGS) -o LFServer LFServer.o Graph.o KruskalMST.o PrimMST.o ThreadPool.o Tree.o $(LDFLAGS)

LFServer.o: LFServer.cpp
	$(CXX) $(CXXFLAGS) -c LFServer.cpp

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp

pipelineServer.o: pipelineServer.cpp
	$(CXX) $(CXXFLAGS) -c pipelineServer.cpp

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c client.cpp

Graph.o: Graph.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c Graph.cpp

KruskalMST.o: KruskalMST.cpp KruskalMST.hpp
	$(CXX) $(CXXFLAGS) -c KruskalMST.cpp

MSTFactory.o: MSTFactory.cpp MSTFactory.hpp
	$(CXX) $(CXXFLAGS) -c MSTFactory.cpp

PrimMST.o: PrimMST.cpp PrimMST.hpp
	$(CXX) $(CXXFLAGS) -c PrimMST.cpp

ThreadPool.o: ThreadPool.cpp ThreadPool.hpp
	$(CXX) $(CXXFLAGS) -c ThreadPool.cpp

Tree.o: Tree.cpp Tree.hpp
	$(CXX) $(CXXFLAGS) -c Tree.cpp

clean:
	rm -f server client pipelineServer LFServer *.o
