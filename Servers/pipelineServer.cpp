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
#include "../src/hpp_files/Graph.hpp"
#include "../src/hpp_files/KruskalMST.hpp"
#include "../src/hpp_files/PrimMST.hpp"
#include "../src/hpp_files/Tree.hpp"

using namespace std;

// Global Variables
mutex graphMutex; // Mutex to synchronize access to the graph
Graph* graph = nullptr; // Pointer to the current graph instance
Tree* mstTree = nullptr; // Pointer to the current MST Tree instance

// Command structure for client requests
struct Command {
    int clientSocket;
    string command;
};

// Queue to hold incoming commands and associated synchronization
queue<Command> commandQueue;
mutex queueMutex; // Mutex to synchronize access to the command queue
condition_variable queueCondition; // Condition variable to signal new commands

// Function to process commands sent by clients
void processCommand(const Command& cmd) {
    string response;
    static int edgesToReceive = 0; // Number of edges to be received
    static vector<pair<pair<int, int>, double>> edges; // Vector to store the edges of the graph

    const string& command = cmd.command; // Command received from client

    if (command.find("NewGraph") == 0) {
        // Command to create a new graph
        int n, m;
        sscanf(command.c_str(), "NewGraph %d %d", &n, &m);
        edgesToReceive = m;
        edges.clear(); // Clear the current list of edges
        edges.resize(m); // Resize the edges vector to hold m edges

        // Prepare the response message
        response = "Creating new graph...\n";
        response += "Number of vertices: " + to_string(n) + ", Number of edges: " + to_string(m) + "\n";
        response += "Please provide the edges one by one (format: u v weight):\n";

    } else if (edgesToReceive > 0) {
        // Expecting to receive the edges
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "%d %d %lf", &u, &v, &weight) == 3) {
            // Check that vertex numbers are valid
            if (u > 0 && v > 0 && (u <= static_cast<int>(edges.size() + 1)) && (v <= static_cast<int>(edges.size() + 1))) {
                edges[edges.size() - edgesToReceive] = {{u, v}, weight};
                edgesToReceive--; // Decrement the number of edges to receive

                response = "Edge " + to_string(edges.size() - edgesToReceive) + ": " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";

                // If all edges have been received, create the graph
                if (edgesToReceive == 0) {
                    graphMutex.lock();
                    delete graph; // Delete the current graph if it exists
                    graph = new Graph(edges.size(), edges); // Create a new graph with the provided edges
                    graphMutex.unlock();

                    response += "Graph created successfully with " + to_string(edges.size()) + " edges\n";

                    // Print the graph structure
                    stringstream ss;
                    streambuf* coutbuf = cout.rdbuf(); // Save old buffer
                    cout.rdbuf(ss.rdbuf()); // Redirect cout to stringstream
                    graph->printGraph(); // Print the graph
                    cout.rdbuf(coutbuf); // Restore cout to original buffer
                    response += ss.str();
                }
            } else {
                response = "Invalid edge input. Ensure vertices are within the correct range.\n";
            }
        } else {
            response = "Invalid edge format. Please use: u v weight\n";
        }
    } else if (command.find("NewEdge") == 0) {
        // Command to add a new edge
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "NewEdge %d %d %lf", &u, &v, &weight) == 3) {
            graphMutex.lock();
            if (graph) {
                graph->addEdge(u, v, weight); // Add the new edge to the graph
                response = "Edge added successfully: " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock();
        } else {
            response = "Invalid NewEdge command format. Use: NewEdge u v weight\n";
        }
    } else if (command.find("RemoveEdge") == 0) {
        // Command to remove an edge
        int u, v;
        if (sscanf(command.c_str(), "RemoveEdge %d %d", &u, &v) == 2) {
            graphMutex.lock();
            if (graph) {
                graph->removeEdge(u, v); // Remove the edge from the graph
                response = "Edge removed successfully: " + to_string(u) + " -> " + to_string(v) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock();
        } else {
            response = "Invalid RemoveEdge command format. Use: RemoveEdge u v\n";
        }
    } else if (command.find("Kruskal") == 0) {
        // Command to execute Kruskal's MST algorithm
        graphMutex.lock();
        if (graph) {
            KruskalMST kruskalMST(*graph); // Create an instance of Kruskal's algorithm
            double mstWeight = kruskalMST.findMST(); // Compute the MST weight
            response = "Kruskal's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST edges in a tree
            if (mstTree) delete mstTree;
            mstTree = new Tree(graph->getNumNodes(), kruskalMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : kruskalMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) + " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }
        graphMutex.unlock();
    } else if (command.find("Prim") == 0) {
        // Command to execute Prim's MST algorithm
        graphMutex.lock();
        if (graph) {
            PrimMST primMST(*graph); // Create an instance of Prim's algorithm
            double mstWeight = primMST.findMST(); // Compute the MST weight
            response = "Prim's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST edges in a tree
            if (mstTree) delete mstTree;
            mstTree = new Tree(graph->getNumNodes(), primMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : primMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) + " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }
        graphMutex.unlock();
    } else if (command.find("MSTWeight") == 0) {
        // Command to get the total weight of the MST
        if (mstTree) {
            double mstWeight = mstTree->getMSTWeight();
            response = "Total MST Weight: " + to_string(mstWeight) + "\n";
        } else {
            response = "MST not calculated. Run Prim or Kruskal first.\n";
        }
    } else if (command.find("LongestDistance") == 0) {
        // Command to find the longest distance in the MST between two vertices
        int u, v;
        if (sscanf(command.c_str(), "LongestDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                double distance = mstTree->longestDistance(u, v);
                if (distance >= 0) {
                    response = "Longest Distance between " + to_string(u) + " and " + to_string(v) + " is: " + to_string(distance) + "\n";
                    
                    // Get the longest path
                    vector<int> path = mstTree->getLongestPath(u, v);
                    stringstream ss;
                    ss << "Longest path: ";
                    for (size_t i = 0; i < path.size(); ++i) {
                        ss << path[i];
                        if (i < path.size() - 1) ss << " -> ";
                    }
                    ss << "\n";
                    response += ss.str();
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
        // Command to find the average distance between two vertices using Floyd-Warshall
        int u, v;
        if (sscanf(command.c_str(), "AverageDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                auto [dist, next] = mstTree->floydWarshall();
                double distance = dist[u][v];
                if (distance < numeric_limits<double>::infinity()) {
                    response = "AverageDistance between " + to_string(u) + " and " + to_string(v) + ": " + to_string(distance) + "\n";

                    vector<int> path;
                    mstTree->reconstructPath(u, v, next, path);

                    response += "Path from " + to_string(u) + " to " + to_string(v) + ": ";
                    for (size_t i = 0; i < path.size(); ++i) {
                        response += to_string(path[i]);
                        if (i < path.size() - 1) response += " -> ";
                    }
                    response += "\n";
                } else {
                    response = "No path exists between the vertices.\n";
                }
            } else {
                response = "MST not calculated. Run Prim or Kruskal first.\n";
            }
        } else {
            response = "Invalid AverageDistance command format. Use: AverageDistance u v\n";
        }
    } else if (command.find("ShortestPath") == 0) {
        // Command to find the shortest path between two vertices using Floyd-Warshall
        int u, v;
        if (sscanf(command.c_str(), "ShortestPath %d %d", &u, &v) == 2) {
            if (mstTree) {
                auto [dist, next] = mstTree->floydWarshall();
                double distance = dist[u][v];
                if (distance < numeric_limits<double>::infinity()) {
                    response = "Shortest Distance between " + to_string(u) + " and " + to_string(v) + ": " + to_string(distance) + "\n";

                    vector<int> path;
                    mstTree->reconstructPath(u, v, next, path);

                    response += "Path from " + to_string(u) + " to " + to_string(v) + ": ";
                    for (size_t i = 0; i < path.size(); ++i) {
                        response += to_string(path[i]);
                        if (i < path.size() - 1) response += " -> ";
                    }
                    response += "\n";
                } else {
                    response = "No path exists between the vertices.\n";
                }
            } else {
                response = "MST not calculated. Run Prim or Kruskal first.\n";
            }
        } else {
            response = "Invalid ShortestPath command format. Use: ShortestPath u v\n";
        }
    } else if (command.find("PrintGraph") == 0) {
        // Command to print the current graph structure
        graphMutex.lock();
        if (graph) {
            stringstream ss;
            streambuf* coutbuf = cout.rdbuf();
            cout.rdbuf(ss.rdbuf());
            graph->printGraph();
            cout.rdbuf(coutbuf);
            response = ss.str();
        }
        graphMutex.unlock();
    } else if (command.find("exit") == 0) {
        // Command to exit the server
        response = "Exiting...\n";
    } else {
        // Invalid command received
        response = "Invalid command\n";
    }

    // Send the response to the client
    write(cmd.clientSocket, response.c_str(), response.size());
}

// Worker thread to process incoming commands
void workerThread() {
    while (true) {
        Command cmd;
        {
            unique_lock<mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !commandQueue.empty(); }); // Wait until a command is available
            cmd = commandQueue.front(); // Get the next command
            commandQueue.pop(); // Remove the command from the queue
        }
        processCommand(cmd); // Process the command
    }
}

// Main function to start the server
int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set masterSet, readSet;
    int fdMax;

    // Create a socket for the server
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    // Set socket options to reuse the address
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Error setting socket options" << endl;
        return 1;
    }

    // Initialize the server address structure
    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections on all interfaces
    serverAddr.sin_port = htons(9034); // Set the port to 9034

    // Bind the server socket to the address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error on binding" << endl;
        return 1;
    }

    // Start listening for incoming connections
    listen(serverSocket, 1);
    cout << "Server started on port 9034" << endl;

    // Initialize file descriptor sets for select
    FD_ZERO(&masterSet);
    FD_ZERO(&readSet);

    FD_SET(serverSocket, &masterSet); // Add the server socket to the master set
    fdMax = serverSocket;

    // Start the worker thread to process commands
    thread worker(workerThread);
    worker.detach(); // Detach the thread to allow it to run independently

    // Main loop to accept and process incoming connections
    while (true) {
        readSet = masterSet;

        // Use select to monitor file descriptors for activity
        if (select(fdMax + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
            cerr << "Error on select" << endl;
            return 1;
        }

        // Check each file descriptor for activity
        for (int i = 0; i <= fdMax; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    // Accept a new connection
                    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
                    if (clientSocket == -1) {
                        cerr << "Error on accept" << endl;
                    } else {
                        FD_SET(clientSocket, &masterSet); // Add the new socket to the master set
                        if (clientSocket > fdMax) {
                            fdMax = clientSocket;
                        }
                        cout << "New connection on socket " << clientSocket << endl;
                    }
                } else {
                    // Handle data from an existing connection
                    char buffer[1024];
                    bzero(buffer, 1024); // Clear the buffer
                    int nbytes = read(i, buffer, 1023);
                    if (nbytes <= 0) {
                        // Connection closed or error
                        if (nbytes == 0) {
                            cout << "Socket " << i << " hung up" << endl;
                        } else {
                            cerr << "Error on read" << endl;
                        }
                        close(i); // Close the socket
                        FD_CLR(i, &masterSet); // Remove it from the master set
                    } else {
                        // Create a command from the received data
                        Command cmd{clientSocket, string(buffer)};
                        {
                            unique_lock<mutex> lock(queueMutex);
                            commandQueue.push(cmd); // Add the command to the queue
                        }
                        queueCondition.notify_one(); // Notify the worker thread
                    }
                }
            }
        }
    }

    // Close the server socket before exiting
    close(serverSocket);
    return 0;
}