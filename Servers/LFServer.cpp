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
#include "../src/hpp files/Graph.hpp"
#include "../src/hpp files/KruskalMST.hpp"
#include "../src/hpp files/PrimMST.hpp"
#include "../src/hpp files/Tree.hpp"  // Include the Tree class
#include "../src/hpp files/ThreadPool.hpp"

using namespace std;

Graph* graph = nullptr;  // Pointer to the current graph
Tree* mstTree = nullptr; // Pointer to the current MST tree
mutex graphMutex;        // Mutex for graph operations

// Command structure to hold requests
struct Command {
    int clientSocket;
    string command;
};

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
        edges.clear();
        edges.resize(m);
        response = "Creating new graph...\n";
        response += "Number of vertices: " + to_string(n) + ", Number of edges: " + to_string(m) + "\n";
        response += "Please provide the edges one by one (format: u v weight):\n";
    } else if (edgesToReceive > 0) { // Expecting edges to complete the graph creation
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "%d %d %lf", &u, &v, &weight) == 3) {
            if (u > 0 && v > 0) {
                edges[edges.size() - edgesToReceive] = {{u, v}, weight};
                response = "Edge " + to_string(edges.size() - edgesToReceive + 1) + ": " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
                edgesToReceive--;

                if (edgesToReceive == 0) { // All edges received, create the graph
                    graphMutex.lock();
                    delete graph;
                    graph = new Graph(edges.size(), edges);
                    graphMutex.unlock();
                    response += "Graph created successfully with " + to_string(edges.size()) + " edges\n";

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
        int u, v;
        double weight;
        if (sscanf(command.c_str(), "NewEdge %d %d %lf", &u, &v, &weight) == 3) {
            graphMutex.lock();
            if (graph) {
                graph->addEdge(u, v, weight);
                response = "Edge added successfully: " + to_string(u) + " -> " + to_string(v) + " with weight " + to_string(weight) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock();
        } else {
            response = "Invalid NewEdge command format. Use: NewEdge u v weight\n";
        }
    } else if (command.find("RemoveEdge") == 0) {
        int u, v;
        if (sscanf(command.c_str(), "RemoveEdge %d %d", &u, &v) == 2) {
            graphMutex.lock();
            if (graph) {
                graph->removeEdge(u, v);
                response = "Edge removed successfully: " + to_string(u) + " -> " + to_string(v) + "\n";
            } else {
                response = "Graph is not initialized.\n";
            }
            graphMutex.unlock();
        } else {
            response = "Invalid RemoveEdge command format. Use: RemoveEdge u v\n";
        }
    } else if (command.find("Kruskal") == 0) {
        graphMutex.lock();
        if (graph) {
            KruskalMST kruskalMST(*graph);
            double mstWeight = kruskalMST.findMST();
            response = "Kruskal's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

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
        graphMutex.lock();
        if (graph) {
            PrimMST primMST(*graph);
            double mstWeight = primMST.findMST();
            response = "Prim's algorithm executed. MST Weight: " + to_string(mstWeight) + "\n";

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
                    response = "Longest Distance between " + to_string(u) + " and " + to_string(v) + " is: " + to_string(distance) + "\n";
                    
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
        if (mstTree) {
            double avgDist = mstTree->averageDistance();
            response = "Average distance between all pairs: " + to_string(avgDist) + "\n";
        } else {
            response = "MST not calculated. Run Prim or Kruskal first.\n";
        }
    } else if (command.find("ShortestPath") == 0) {
        int u, v;
        if (sscanf(command.c_str(), "ShortestPath %d %d", &u, &v) == 2) {
            if (mstTree) {
                auto [dist, next] = mstTree->floydWarshall();
                double distance = dist[u][v];

                if (distance < std::numeric_limits<double>::infinity()) {
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
        graphMutex.lock();
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
        graphMutex.unlock();
    } else if (command.find("exit") == 0) {
        response = "Exiting...\n";
    } else {
        response = "Invalid command\n";
    }

    // Send response to client
    write(cmd.clientSocket, response.c_str(), response.size());
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set masterSet;

    ThreadPool pool(4);  // Create thread pool with desired number of threads

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Error setting socket options" << endl;
        return 1;
    }

    memset((char*)&serverAddr, 0, sizeof(serverAddr)); // Replace bzero with memset
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9034);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error on binding" << endl;
        return 1;
    }

    listen(serverSocket, 1);
    cout << "Server started on port 9034" << endl;

    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);

    while (true) {
        fd_set readSet = masterSet;
        if (select(FD_SETSIZE, &readSet, nullptr, nullptr, nullptr) == -1) {
            cerr << "Error on select" << endl;
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
                    if (clientSocket != -1) {
                        cout << "New connection on socket " << clientSocket << endl;
                        pool.enqueue([clientSocket] {
                            char buffer[1024];
                            memset(buffer, 0, 1024); // Replace bzero with memset
                            while (true) {
                                int nbytes = read(clientSocket, buffer, 1023);
                                if (nbytes > 0) {
                                    Command cmd = {clientSocket, string(buffer)};
                                    processCommand(cmd);
                                } else {
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

    close(serverSocket);
    return 0;
}
