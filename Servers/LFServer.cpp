#include <iostream>
#include <vector>
#include <string>
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
#include <cstring> // For memset
#include "../src/hpp_files/Graph.hpp"
#include "../src/hpp_files/KruskalMST.hpp"
#include "../src/hpp_files/PrimMST.hpp"
#include "../src/hpp_files/Tree.hpp"  // Include the Tree class
#include "../src/hpp_files/ThreadPool.hpp"  // Include the ThreadPool class

using namespace std;

// Global graph and MST tree pointers
Graph* graph = nullptr;  // Pointer to the current graph
Tree* mstTree = nullptr; // Pointer to the current MST tree
mutex graphMutex;        // Mutex for thread-safe graph operations

// Command structure to hold client requests
struct Command {
    int clientSocket;   // Socket for client communication
    string command;     // Command string sent by the client
};

// Function to process client commands
void processCommand(const Command& cmd) {
    string response;  // Response to send back to the client
    static int edgesToReceive = 0;  // Number of edges to receive for graph creation
    static vector<pair<pair<int, int>, double>> edges;  // Vector to store edges with weights

    const string& command = cmd.command;  // Command extracted from the client request

    if (command.find("NewGraph") == 0) {
        // Command to create a new graph
        int n, m;
        sscanf(command.c_str(), "NewGraph %d %d", &n, &m);
        edgesToReceive = m;  // Set the number of edges expected
        edges.clear();
        edges.resize(m);  // Resize edges vector to match the number of edges
        response = "Creating new graph...\n";
        response += "Number of vertices: " + to_string(n) + ", Number of edges: " + to_string(m) + "\n";
        response += "Please provide the edges one by one (format: u v weight):\n";

    } else if (edgesToReceive > 0) {
        // Expecting edges to complete the graph creation
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "%d %d %lf", &u, &v, &weight) == 3) {
            // Ensure valid vertex numbers
            if (u > 0 && v > 0) {
                edges[edges.size() - edgesToReceive] = {{u, v}, weight};
                response = "Edge " + to_string(edges.size() - edgesToReceive + 1) +
                           ": " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
                edgesToReceive--;

                if (edgesToReceive == 0) {
                    // All edges received, create the graph
                    lock_guard<mutex> lock(graphMutex);
                    delete graph;  // Delete any existing graph
                    graph = new Graph(edges.size(), edges);  // Create a new graph
                    response += "Graph created successfully with " + to_string(edges.size()) + " edges\n";

                    // Capture and send the graph structure
                    stringstream ss;
                    streambuf* coutbuf = cout.rdbuf();
                    cout.rdbuf(ss.rdbuf());
                    graph->printGraph();
                    cout.rdbuf(coutbuf);
                    response += ss.str();
                }
            } else {
                response = "Invalid edge input. Ensure vertices are in range.\n";
            }
        } else {
            response = "Invalid edge format. Use: u v weight\n";
        }

    } else if (command.find("NewEdge") == 0) {
        // Command to add a new edge to the graph
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "NewEdge %d %d %lf", &u, &v, &weight) == 3) {
            lock_guard<mutex> lock(graphMutex);
            if (graph) {
                graph->addEdge(u, v, weight);  // Add the edge to the graph
                response = "Edge added successfully: " + to_string(u) + " -> " + to_string(v) +
                           " with weight " + to_string(weight) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
        } else {
            response = "Invalid NewEdge command format. Use: NewEdge u v weight\n";
        }

    } else if (command.find("RemoveEdge") == 0) {
        // Command to remove an edge from the graph
        int u, v;
        if (sscanf(command.c_str(), "RemoveEdge %d %d", &u, &v) == 2) {
            lock_guard<mutex> lock(graphMutex);
            if (graph) {
                graph->removeEdge(u, v);  // Remove the edge
                response = "Edge removed successfully: " + to_string(u) + " -> " + to_string(v) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
        } else {
            response = "Invalid RemoveEdge command format. Use: RemoveEdge u v\n";
        }

    } else if (command.find("Kruskal") == 0) {
        // Command to run Kruskal's algorithm
        lock_guard<mutex> lock(graphMutex);
        if (graph) {
            KruskalMST kruskalMST(*graph);  // Initialize KruskalMST
            double mstWeight = kruskalMST.findMST();  // Calculate MST weight
            response = "Kruskal's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST in the tree
            if (mstTree) delete mstTree;
            mstTree = new Tree(graph->getNumNodes(), kruskalMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : kruskalMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) +
                           " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }

    } else if (command.find("Prim") == 0) {
        // Command to run Prim's algorithm
        lock_guard<mutex> lock(graphMutex);
        if (graph) {
            PrimMST primMST(*graph);  // Initialize PrimMST
            double mstWeight = primMST.findMST();  // Calculate MST weight
            response = "Prim's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

            // Store the MST in the tree
            if (mstTree) delete mstTree;
            mstTree = new Tree(graph->getNumNodes(), primMST.getMSTEdges());

            response += "Edges of the MST:\n";
            for (const auto& edge : primMST.getMSTEdges()) {
                response += to_string(edge.first.first) + " -> " + to_string(edge.first.second) +
                           " (Weight: " + to_string(edge.second) + ")\n";
            }
        } else {
            response = "Graph is not initialized.\n";
        }

    } else if (command.find("MSTWeight") == 0) {
        // Command to return the total weight of the MST
        if (mstTree) {
            double mstWeight = mstTree->getMSTWeight();
            response = "Total MST Weight: " + to_string(mstWeight) + "\n";
        } else {
            response = "MST not calculated. Run Prim or Kruskal first.\n";
        }

    } else if (command.find("LongestDistance") == 0) {
        // Command to calculate the longest distance between two vertices
        int u, v;
        if (sscanf(command.c_str(), "LongestDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                double distance = mstTree->longestDistance(u, v);
                if (distance >= 0) {
                    response = "Longest Distance between " + to_string(u) + " and " + to_string(v) +
                               " is: " + to_string(distance) + "\n";

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
        // Command to calculate the average distance between two vertices using Floyd-Warshall
        int u, v;
        if (sscanf(command.c_str(), "AverageDistance %d %d", &u, &v) == 2) {
            if (mstTree) {
                auto [dist, next] = mstTree->floydWarshall();  // Run Floyd-Warshall algorithm
                double distance = dist[u][v];

                if (distance < numeric_limits<double>::infinity()) {
                    response = "AverageDistance between " + to_string(u) + " and " + to_string(v) + ": " + to_string(distance) + "\n";

                    vector<int> path;
                    mstTree->reconstructPath(u, v, next, path);  // Reconstruct the path

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
        // Command to calculate the shortest path between two vertices
        int u, v;
        if (sscanf(command.c_str(), "ShortestPath %d %d", &u, &v) == 2) {
            if (mstTree) {
                auto [dist, next] = mstTree->floydWarshall();  // Run Floyd-Warshall algorithm
                double distance = dist[u][v];

                if (distance < numeric_limits<double>::infinity()) {
                    response = "Shortest Distance between " + to_string(u) + " and " + to_string(v) + ": " + to_string(distance) + "\n";

                    // Reconstruct the shortest path
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
        lock_guard<mutex> lock(graphMutex);
        if (graph) {
            stringstream ss;
            streambuf* coutbuf = cout.rdbuf();
            cout.rdbuf(ss.rdbuf());
            graph->printGraph();
            cout.rdbuf(coutbuf);
            response = ss.str();
        } else {
            response = "Graph is not initialized.\n";
        }

    } else if (command.find("exit") == 0) {
        // Command to exit the server
        response = "Exiting...\n";

    } else {
        response = "Invalid command\n";  // Invalid command received
    }

    // Send response back to client
    write(cmd.clientSocket, response.c_str(), response.size());
}

int main() {
    // Server setup
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set masterSet;  // Set of file descriptors for select()

    ThreadPool pool(4);  // Create thread pool with 4 threads

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // Create server socket
    if (serverSocket < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Error setting socket options" << endl;
        return 1;
    }

    memset((char*)&serverAddr, 0, sizeof(serverAddr));  // Clear server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9034);  // Set port to 9034

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error on binding" << endl;
        return 1;
    }

    listen(serverSocket, 1);  // Start listening for connections
    cout << "Server started on port 9034" << endl;

    FD_ZERO(&masterSet);  // Initialize the master set
    FD_SET(serverSocket, &masterSet);  // Add the server socket to the set

    // Main loop for accepting client connections
    while (true) {
        fd_set readSet = masterSet;  // Copy the master set
        if (select(FD_SETSIZE, &readSet, nullptr, nullptr, nullptr) == -1) {
            cerr << "Error on select" << endl;
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);  // Accept new connection
                    if (clientSocket != -1) {
                        cout << "New connection on socket " << clientSocket << endl;
                        pool.enqueue(clientSocket, [clientSocket] {
                        // Handling the client in a separate thread from the thread pool
                        char buffer[1024];
                        memset(buffer, 0, sizeof(buffer));  // Clear buffer

                        // Process all commands from the client in the same thread
                        while (true) {
                            int nbytes = read(clientSocket, buffer, sizeof(buffer) - 1);  // Read data from the client
                            if (nbytes > 0) {
                                Command cmd = {clientSocket, string(buffer)};

                                // Process the client command within the same thread
                                processCommand(cmd);

                                // Clear the buffer after each command to avoid residual data issues
                                memset(buffer, 0, sizeof(buffer));
                            } else {
                                // Close the client socket on error or disconnect
                                close(clientSocket);
                                break;
                            }
                        }
                    });
                    }
                }
            }
        }
    }

    close(serverSocket);  // Close server socket
    return 0;
}

/*
Leader-Follower Pattern Implementation:

1. **Adding a Task to the Queue**  
   - Function: `enqueue(int graphId, function<void()> task)`  
   - Adds a task to the queue, associating it with a graph identifier (`graphId`).
   - The queue ensures that tasks related to the same graph are handled by only one thread at a time.

2. **Thread Becomes the Leader and Picks a Task**  
   - Function: `worker()`  
   - One of the threads takes the first task from the queue and starts processing it as the Leader.

3. **Locking the Graph by graphId**  
   - Function: `worker()`  
   - The thread checks if the graph with the given `graphId` is already locked.
     If it is locked, the task is returned to the queue.  
     If not, the thread locks the graph to prevent other threads from accessing it simultaneously.

4. **Performing All Operations on the Same Graph**  
   - Function: `processCommand(const Command& cmd)`  
   - The thread performs all commands related to the graph in sequence without interruptions.

5. **Releasing the Lock After the Task is Complete**  
   - Function: `worker()`  
   - Once the thread finishes processing the task, it releases the lock on the graph, allowing other threads to work on it if needed.

6. **Waiting for New Tasks**  
   - Function: `worker()`  
   - After releasing the lock, the thread returns to wait for new tasks from the queue. If a new task arrives, the thread becomes the Leader again and handles it.

Purpose of the Implementation:
- **Prevents Conflicts**: Only one thread works on a specific graph at any given time.
- **Improves Performance**: All operations on a graph are handled by the same thread, improving cache locality and minimizing context switching.
- **Simplifies Task Management**: Tasks are organized in a queue, and the leading thread completes the entire task before releasing the graph for others.

*/
