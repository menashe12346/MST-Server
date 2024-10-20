#include <iostream>
#include <vector>
#include <string>
#include <functional>  // For std::function
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
#include <cstring>  // For bzero
#include "../src/hpp_files/Graph.hpp"
#include "../src/hpp_files/KruskalMST.hpp"
#include "../src/hpp_files/PrimMST.hpp"
#include "../src/hpp_files/Tree.hpp"

using namespace std;

// Structure to represent a client command
struct Command {
    int clientSocket;
    string command;
};

// ActiveObject class manages a thread to process tasks asynchronously.
class ActiveObject {
public:
    using Task = std::function<void()>;

    ActiveObject() : stopFlag(false), workerThread(&ActiveObject::run, this) {}

    ~ActiveObject() {
        stop();
        workerThread.join();
    }

    // Submit a new task to the active object
    void submit(Task task) {
        unique_lock<mutex> lock(queueMutex);
        taskQueue.push(move(task));
        condition.notify_one();
    }

private:
    queue<Task> taskQueue;
    mutex queueMutex;
    condition_variable condition;
    bool stopFlag;
    thread workerThread;

    // The run function continuously processes tasks from the queue
    void run() {
        while (true) {
            Task task;
            {
                unique_lock<mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stopFlag || !taskQueue.empty(); });
                if (stopFlag && taskQueue.empty()) break;
                task = move(taskQueue.front());
                taskQueue.pop();
            }
            task();  // Execute the task
        }
    }

    // Stop the active object by setting the stop flag
    void stop() {
        unique_lock<mutex> lock(queueMutex);
        stopFlag = true;
        condition.notify_all();
    }
};

// Global variables for thread synchronization and task management
mutex graphMutex;  
mutex queueMutex;
condition_variable queueCondition;
queue<Command> commandQueue;

// Pointers to the graph and MST tree
Graph* graph = nullptr;
Tree* mstTree = nullptr;

// ActiveObjects for different pipeline stages
ActiveObject commandParser, graphOperator, responseHandler;

void sendResponse(int clientSocket, const std::string& response) {
    write(clientSocket, response.c_str(), response.size());
}

// Function to parse a command and pass it to the appropriate pipeline stage
void handleCommand(const Command& cmd) {
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

    responseHandler.submit([cmd, response] {
        sendResponse(cmd.clientSocket, response);  // העברת התשובה לשלב הבא
    });
}

void parseCommand(const Command& cmd) {
    graphOperator.submit([cmd] {
        handleCommand(cmd);  // העברת הפקודה לשלב הבא
    });
}

// Worker thread function to process commands from the queue
void workerThread() {
    while (true) {
        Command cmd;
        {
            unique_lock<mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !commandQueue.empty(); });
            cmd = commandQueue.front();
            commandQueue.pop();
        }
        commandParser.submit([cmd] { parseCommand(cmd); });
    }
}

// Main function to set up the server and handle incoming connections
int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set masterSet, readSet;
    int fdMax;

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9034);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error on binding" << endl;
        return 1;
    }

    // Start listening for incoming connections
    listen(serverSocket, 1);
    cout << "Server started on port 9034" << endl;

    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);
    fdMax = serverSocket;

    // Start the worker thread
    thread worker(workerThread);
    worker.detach();

    // Main server loop to handle connections and commands
    while (true) {
        readSet = masterSet;
        if (select(fdMax + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
            cerr << "Error on select" << endl;
            return 1;
        }

        for (int i = 0; i <= fdMax; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
                    if (clientSocket == -1) {
                        cerr << "Error on accept" << endl;
                    } else {
                        FD_SET(clientSocket, &masterSet);
                        if (clientSocket > fdMax) {
                            fdMax = clientSocket;
                        }
                        cout << "New connection on socket " << clientSocket << endl;
                    }
                } else {
                    char buffer[1024];
                    bzero(buffer, 1024);
                    int nbytes = read(i, buffer, 1023);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            cout << "Socket " << i << " hung up" << endl;
                        } else {
                            cerr << "Error on read" << endl;
                        }
                        close(i);
                        FD_CLR(i, &masterSet);
                    } else {
                        Command cmd{i, string(buffer)};
                        {
                            unique_lock<mutex> lock(queueMutex);
                            commandQueue.push(cmd);
                        }
                        queueCondition.notify_one();
                    }
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}

/**
 * Pipeline Design Pattern Implementation:
 * 
 * - Client sends a command – The server receives the command and pushes it to the command queue.
 * - workerThread retrieves a command from the queue and passes it to commandParser.
 * - commandParser interprets the command and forwards it to graphOperator for processing.
 * - graphOperator performs the requested operation and sends the result to responseHandler.
 * - responseHandler sends the response back to the client.
 */
