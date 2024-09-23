#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstring>
#include <algorithm>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sstream>
#include "Graph.hpp"
#include "KruskalMST.hpp"
#include "PrimMST.hpp"
#include "Tree.hpp"

using namespace std;

mutex graphMutex;
Graph* graph = nullptr; // Pointer to the current graph
Tree* mstTree = nullptr; // Pointer to the current MST Tree

// Command structure to hold requests
struct Command {
    int clientSocket;
    string command;
};

// Command queue and synchronization primitives
queue<Command> commandQueue;
mutex queueMutex;
condition_variable queueCondition;

// Function to process commands
void processCommand(const Command& cmd) {
    string response;
    static int edgesToReceive = 0; // Number of edges to receive
    static vector<pair<pair<int, int>, double>> edges; // Vector to store edges with weights

    const string& command = cmd.command;

    if (command.find("NewGraph") == 0) {
        int n, m;
        sscanf(command.c_str(), "NewGraph %d %d", &n, &m);
        edgesToReceive = m; // Set the number of edges to receive
        edges.clear(); // Clear the edges vector
        edges.resize(m); // Resize the edges vector
        response = "Creating new graph...\n";
        response += "Number of vertices: " + to_string(n) + ", Number of edges: " + to_string(m) + "\n";
        response += "Please provide the edges one by one (format: u v weight):\n";
    } else if (edgesToReceive > 0) { // Expecting edges to complete the graph creation
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "%d %d %lf", &u, &v, &weight) == 3) { // Ensure parsing was successful

            // Ensure valid vertex numbers
            if (u > 0 && v > 0 && (u <= static_cast<int>(edges.size() + 1)) && (v <= static_cast<int>(edges.size() + 1))) {
                edges[edges.size() - edgesToReceive] = {{u, v}, weight}; // Store the edge with weight
                response = "Edge " + to_string(edges.size() - edgesToReceive + 1) + ": " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
                edgesToReceive--; // Decrease the number of edges to receive
                
                if (edgesToReceive == 0) { // All edges received, create the graph
                    graphMutex.lock(); // Lock the graph mutex
                    delete graph; // Delete the existing graph
                    graph = new Graph(edges.size(), edges); // Create a new graph with weights
                    graphMutex.unlock(); // Unlock the graph mutex
                    response += "Graph created successfully with " + to_string(edges.size()) + " edges\n";

                    stringstream ss;
                    streambuf* coutbuf = cout.rdbuf(); // Save old buffer
                    cout.rdbuf(ss.rdbuf()); // Redirect cout to stringstream
                    graph->printGraph(); // Print the graph
                    cout.rdbuf(coutbuf); // Reset cout to its old buffer
                    response += ss.str(); // Extract the graph structure from the stringstream
                }
            } else {
                response = "Invalid edge input. Please ensure vertices are within the correct range.\n";
            }
        } else {
            response = "Invalid edge format. Please use: u v weight\n";
        }
    } else if (command.find("NewEdge") == 0) {
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "NewEdge %d %d %lf", &u, &v, &weight) == 3) {
            graphMutex.lock(); // Lock the graph mutex
            if (graph) {
                graph->addEdge(u, v, weight); // Add the edge with weight
                response = "Edge added successfully: " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock(); // Unlock the graph mutex
        } else {
            response = "Invalid NewEdge command format. Use: NewEdge u v weight\n";
        }
    } else if (command.find("RemoveEdge") == 0) {
        int u, v;
        if (sscanf(command.c_str(), "RemoveEdge %d %d", &u, &v) == 2) {
            graphMutex.lock(); // Lock the graph mutex
            if (graph) {
                graph->removeEdge(u, v); // Remove the edge
                response = "Edge removed successfully: " + to_string(u) + " -> " + to_string(v) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock(); // Unlock the graph mutex
        } else {
            response = "Invalid RemoveEdge command format. Use: RemoveEdge u v\n";
        }
    } else if (command.find("Kruskal") == 0) {
        graphMutex.lock(); // Lock the graph mutex
        if (graph) {
            KruskalMST kruskalMST(*graph); // Create an instance of KruskalMST
            double mstWeight = kruskalMST.findMST(); // Calculate MST
            response = "Kruskal's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST edges in a tree
            if (mstTree) delete mstTree; // Delete any existing MST tree
            mstTree = new Tree(graph->getNumNodes(), kruskalMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : kruskalMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) + " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }
        graphMutex.unlock(); // Unlock the graph mutex
    } else if (command.find("Prim") == 0) {
        graphMutex.lock(); // Lock the graph mutex
        if (graph) {
            PrimMST primMST(*graph); // Create an instance of PrimMST
            double mstWeight = primMST.findMST(); // Calculate MST
            response = "Prim's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST edges in a tree
            if (mstTree) delete mstTree; // Delete any existing MST tree
            mstTree = new Tree(graph->getNumNodes(), primMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : primMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) + " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }
        graphMutex.unlock(); // Unlock the graph mutex
    } else if (command.find("MSTWeight") == 0) {
        if (mstTree) {
            double mstWeight = mstTree->getMSTWeight();
            response = "Total MST Weight: " + to_string(mstWeight) + "\n";
        } else {
            response = "MST not calculated. Run Prim or Kruskal first.\n";
        }
    } else if (command.find("LongestDistance") == 0) {
        int u, v;
        if (sscanf(command.c_str(), "LongestDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                double distance = mstTree->longestDistance(u, v);
                if (distance >= 0) {
                    std::vector<int> path = mstTree->getLongestPath(u, v);
                    stringstream ss;
                    ss << "Longest Distance between " + to_string(u) + " and " + to_string(v) + " is: " + to_string(distance) + "\n";
                    ss << "Longest path: ";
                    for (size_t i = 0; i < path.size(); ++i) {
                        ss << path[i];
                        if (i < path.size() - 1) ss << " -> ";
                    }
                    ss << "\n";
                    response = ss.str();
                } else {
                    response = "No path exists between the vertices.\n";
                }
            } else {
                response = "MST not calculated. Run Prim or Kruskal first.\n";
            }
        } else {
            response = "Invalid LongestDistance command format. Use: LongestDistance u v\n";
        }
    } else if (command.find("AverageDistance") == 0) {
        int u, v;
        if (sscanf(command.c_str(), "AverageDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                double avgDistance = mstTree->averageDistance();
                stringstream ss;
                ss << "Average Distance between all pairs of vertices: " << avgDistance << "\n";

                // Get the paths between the requested vertices
                auto [dist, next] = mstTree->floydWarshall();

                if (u >= 1 && v >= 1 && u <= graph->getNumNodes() && v <= graph->getNumNodes()) {
                    if (dist[u][v] < std::numeric_limits<double>::infinity()) {
                        std::vector<int> path;
                        mstTree->reconstructPath(u, v, next, path);
                        ss << " (Distance: " << dist[u][v] << ")\n";
                    } else {
                        ss << "No path exists between " + to_string(u) + " and " + to_string(v) + ".\n";
                    }
                } else {
                    ss << "Invalid vertex range.\n";
                }
                response = ss.str();
            } else {
                response = "MST not calculated. Run Prim or Kruskal first.\n";
            }
        } else {
            response = "Invalid AverageDistance command format. Use: AverageDistance u v\n";
        }
    } else if (command.find("PrintGraph") == 0) {
        graphMutex.lock(); // Lock the graph mutex
        if (graph) {
            stringstream ss;
            streambuf* coutbuf = cout.rdbuf(); // Save old buffer
            cout.rdbuf(ss.rdbuf()); // Redirect cout to stringstream
            graph->printGraph(); // Print the graph
            cout.rdbuf(coutbuf); // Reset cout to its old buffer
            response = ss.str();
        }
        graphMutex.unlock(); // Unlock the graph mutex
    } else if (command.find("exit") == 0) {
        response = "Exiting...\n";
    } else {
        response = "Invalid command\n";
    }

    // Send response to client
    write(cmd.clientSocket, response.c_str(), response.size());
}

void workerThread() {
    while (true) {
        Command cmd;
        {
            unique_lock<mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !commandQueue.empty(); }); // Wait until there's a command
            cmd = commandQueue.front(); // Get the command
            commandQueue.pop(); // Remove the command from the queue
        }
        processCommand(cmd); // Process the command
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr; // hold the server and client address information
    socklen_t addrLen = sizeof(clientAddr);
    fd_set masterSet, readSet; // File descriptor sets for select.
    int fdMax;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if (serverSocket < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { // Set socket options
        cerr << "Error setting socket options" << endl;
        return 1;
    }

    bzero((char*)&serverAddr, sizeof(serverAddr)); // Clears the server address structure.
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections on all available networks.
    serverAddr.sin_port = htons(9034); // Convert the port into big-endian (network byte order).

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { // Bind the socket
        cerr << "Error on binding" << endl;
        return 1;
    }

    listen(serverSocket, 1); // Listen for connections
    cout << "Server started on port 9034" << endl;

    FD_ZERO(&masterSet);
    FD_ZERO(&readSet);

    FD_SET(serverSocket, &masterSet); // Adds the server socket to the master set.
    fdMax = serverSocket;

    // Start the worker thread
    thread worker(workerThread);
    worker.detach(); // Detach the thread to run independently

    while (true) {
        readSet = masterSet;

        if (select(fdMax + 1, &readSet, nullptr, nullptr, nullptr) == -1) { // Monitor sockets for activity
            cerr << "Error on select" << endl;
            return 1;
        }

        for (int i = 0; i <= fdMax; ++i) { // Iterates through the file descriptors.
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen); // Accept new connection
                    if (clientSocket == -1) {
                        cerr << "Error on accept" << endl;
                    } else {
                        FD_SET(clientSocket, &masterSet); // Add new socket to master set
                        if (clientSocket > fdMax) {
                            fdMax = clientSocket;
                        }
                        cout << "New connection on socket " << clientSocket << endl;
                    }
                } else {
                    char buffer[1024];
                    bzero(buffer, 1024); // Sets all bytes in the buffer to zero
                    int nbytes = read(i, buffer, 1023); // Read data from client
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            cout << "Socket " << i << " hung up" << endl;
                        } else {
                            cerr << "Error on read" << endl;
                        }
                        close(i); // Close the socket
                        FD_CLR(i, &masterSet); // Remove socket from master set
                    } else {
                        Command cmd{clientSocket, string(buffer)}; // Create command struct
                        {
                            unique_lock<mutex> lock(queueMutex);
                            commandQueue.push(cmd); // Add command to queue
                        }
                        queueCondition.notify_one(); // Notify the worker thread
                    }
                }
            }
        }
    }

    close(serverSocket); // Close the server socket
    return 0;
}
