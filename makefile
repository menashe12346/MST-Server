CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -fprofile-arcs -ftest-coverage
LDFLAGS = -lpthread # for POSIX threads

# Define paths for directories
SRCDIR_CPP = src/cpp_files
SRCDIR_HPP = src/hpp_files
SERVERS_DIR = Servers
CLIENT_DIR = Client

# Targets
all: client pipelineServer LFServer

client: $(CLIENT_DIR)/client.o
	$(CXX) $(CXXFLAGS) -o $(CLIENT_DIR)/client $(CLIENT_DIR)/client.o $(LDFLAGS)

pipelineServer: $(SERVERS_DIR)/pipelineServer.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o Tree.o
	$(CXX) $(CXXFLAGS) -o $(SERVERS_DIR)/pipelineServer $(SERVERS_DIR)/pipelineServer.o Graph.o KruskalMST.o MSTFactory.o PrimMST.o Tree.o $(LDFLAGS)

LFServer: $(SERVERS_DIR)/LFServer.o Graph.o KruskalMST.o PrimMST.o ThreadPool.o Tree.o
	$(CXX) $(CXXFLAGS) -o $(SERVERS_DIR)/LFServer $(SERVERS_DIR)/LFServer.o Graph.o KruskalMST.o PrimMST.o ThreadPool.o Tree.o $(LDFLAGS)

# Object file rules
$(SERVERS_DIR)/LFServer.o: $(SERVERS_DIR)/LFServer.cpp $(SRCDIR_HPP)/Graph.hpp
	$(CXX) $(CXXFLAGS) -c $(SERVERS_DIR)/LFServer.cpp -o $(SERVERS_DIR)/LFServer.o

$(SERVERS_DIR)/pipelineServer.o: $(SERVERS_DIR)/pipelineServer.cpp $(SRCDIR_HPP)/Graph.hpp
	$(CXX) $(CXXFLAGS) -c $(SERVERS_DIR)/pipelineServer.cpp -o $(SERVERS_DIR)/pipelineServer.o

$(CLIENT_DIR)/client.o: $(CLIENT_DIR)/client.cpp $(SRCDIR_HPP)/Graph.hpp
	$(CXX) $(CXXFLAGS) -c $(CLIENT_DIR)/client.cpp -o $(CLIENT_DIR)/client.o

# Compile cpp files from src/cpp_files
Graph.o: $(SRCDIR_CPP)/Graph.cpp $(SRCDIR_HPP)/Graph.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/Graph.cpp -o Graph.o

KruskalMST.o: $(SRCDIR_CPP)/KruskalMST.cpp $(SRCDIR_HPP)/KruskalMST.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/KruskalMST.cpp -o KruskalMST.o

MSTFactory.o: $(SRCDIR_CPP)/MSTFactory.cpp $(SRCDIR_HPP)/MSTFactory.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/MSTFactory.cpp -o MSTFactory.o

PrimMST.o: $(SRCDIR_CPP)/PrimMST.cpp $(SRCDIR_HPP)/PrimMST.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/PrimMST.cpp -o PrimMST.o

ThreadPool.o: $(SRCDIR_CPP)/ThreadPool.cpp $(SRCDIR_HPP)/ThreadPool.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/ThreadPool.cpp -o ThreadPool.o

Tree.o: $(SRCDIR_CPP)/Tree.cpp $(SRCDIR_HPP)/Tree.hpp
	$(CXX) $(CXXFLAGS) -c $(SRCDIR_CPP)/Tree.cpp -o Tree.o

# Valgrind test for pipelineServer
valgrind_pipelineServer:
	-killall pipelineServer || true # Ensure no previous server is running
	valgrind --leak-check=full $(SERVERS_DIR)/pipelineServer -v 5 -e 6 -s 42 & # Run pipelineServer
	sleep 2 # Allow server to start
	valgrind --leak-check=full $(CLIENT_DIR)/client -v 5 -e 6 -s 42 # Run client to communicate with the server
	sleep 2 # Give time for client to complete communication
	-killall pipelineServer || true # Stop pipelineServer if still running

# Valgrind test for LFServer
valgrind_LFServer:
	-killall LFServer || true # Ensure no previous server is running
	valgrind --leak-check=full $(SERVERS_DIR)/LFServer -v 5 -e 6 -s 42 & # Run LFServer
	sleep 2 # Allow server to start
	valgrind --leak-check=full $(CLIENT_DIR)/client -v 5 -e 6 -s 42 # Run client to communicate with the server
	sleep 2 # Give time for client to complete communication
	-killall LFServer || true # Stop LFServer if still running

# Clean object files, executables, and coverage data
clean:
	rm -f $(CLIENT_DIR)/client $(SERVERS_DIR)/pipelineServer $(SERVERS_DIR)/LFServer *.o *.gcda *.gcno *.gcov
	rm -f $(CLIENT_DIR)/*.o $(CLIENT_DIR)/*.gcda $(CLIENT_DIR)/*.gcno $(CLIENT_DIR)/*.gcov
	rm -f $(SERVERS_DIR)/*.o $(SERVERS_DIR)/*.gcda $(SERVERS_DIR)/*.gcno $(SERVERS_DIR)/*.gcov
