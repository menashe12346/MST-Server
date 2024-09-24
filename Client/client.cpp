#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

/// @brief Sends a command to the server.
void sendCommand(int socket, const string& command) {
    // Send the command to the server using the provided socket
    if (write(socket, command.c_str(), command.size()) < 0) {
        cerr << "Error writing to socket" << endl; // Error handling
    }
}

/// @brief Receives a response from the server and prints it to the console.
void receiveResponse(int socket) {
    char buffer[1024];
    bzero(buffer, 1024); // Clear the buffer
    int n = read(socket, buffer, 1023); // Read response from the server
    if (n < 0) {
        cerr << "Error reading from socket" << endl; // Error handling
    } else {
        cout << buffer; // Print the server's response to the console
    }
}

/// @brief Prints a help message with available commands and their descriptions.
void printHelp() {
    // Help message listing all available commands with their usage
    cout << "\nAvailable commands:\n"
         << "NewGraph n m\n"
         << "  - Create a new graph with n vertices and m edges\n"
         << "  - Example: NewGraph 5 5\n"
         << "  - After this command, provide the edges one by one:\n"
         << "    1 2 1.0\n"
         << "    2 3 2.0\n"
         << "    3 4 3.0\n"
         << "    4 5 4.0\n"
         << "    5 1 5.0\n"
         << "NewEdge u v weight\n"
         << "  - Add a new edge from vertex u to vertex v with the specified weight\n"
         << "  - Example: NewEdge 3 4 1.5\n"
         << "RemoveEdge u v\n"
         << "  - Remove an edge from vertex u to vertex v\n"
         << "  - Example: RemoveEdge 3 4\n"
         << "Kruskal\n"
         << "  - Run Kruskal's algorithm to find the minimum spanning tree\n"
         << "  - Example: Kruskal\n"
         << "Prim\n"
         << "  - Run Prim's algorithm to find the minimum spanning tree\n"
         << "  - Example: Prim\n"
         << "MSTWeight\n"
         << "  - Get the total weight of the current MST\n"
         << "  - Example: MSTWeight\n"
         << "LongestDistance u v\n"
         << "  - Find the longest distance between two vertices u and v\n"
         << "  - Example: LongestDistance 1 3\n"
         << "AverageDistance\n"
         << "  - Calculate the average distance of all paths in the MST\n"
         << "  - Example: AverageDistance\n"
         << "ShortestPath u v\n"
         << "  - Find the shortest path between two vertices u and v\n"
         << "  - Example: ShortestPath 1 3\n"
         << "PrintGraph\n"
         << "  - Print the current graph\n"
         << "  - Example: PrintGraph\n"
         << "exit\n"
         << "  - Exit the client\n"
         << "  - Example: exit\n"
         << "help\n"
         << "  - Display this help message\n"
         << "  - Example: help\n"
         << endl;
}

int main() {
    int sockfd, portno = 9034; // Port number of the server
    struct sockaddr_in serv_addr; // Struct to hold server address information

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl; // Error handling if socket creation fails
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use localhost for testing
    serv_addr.sin_port = htons(portno); // Convert the port number to network byte order (big-endian)

    // Attempt to connect to the server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Error connecting" << endl; // Error handling if connection fails
        return 1;
    }

    // Main loop to continuously accept commands from the user
    while (true) {
        cout << "Enter command (NewGraph, NewEdge, RemoveEdge, Kruskal, Prim, MSTWeight, LongestDistance, AverageDistance, ShortestPath, PrintGraph, help, exit): ";
        string command;
        getline(cin, command); // Read the user's input

        // If the user asks for help, display the help message
        if (command == "help") {
            printHelp();
            continue;
        }

        sendCommand(sockfd, command + "\n"); // Send the user command to the server
        receiveResponse(sockfd); // Receive and print the server's response
        if (command == "exit") break; // Exit the loop if the user types "exit"

        // If the command is NewGraph, take additional input for the edges
        if (command.find("NewGraph") == 0) {
            int n, m;
            sscanf(command.c_str(), "NewGraph %d %d", &n, &m); // Extract n and m from the command
            for (int i = 0; i < m; ++i) {
                cout << "Enter edge " << i + 1 << " (format: u v weight): ";
                getline(cin, command); // Read each edge
                sendCommand(sockfd, command + "\n"); // Send each edge to the server
                receiveResponse(sockfd); // Receive and print the server's response for each edge
            }
        }
    }

    close(sockfd); // Close the socket when done
    return 0;
}
