#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

/// @brief Sends a command to the server.
/// @param socket The socket of the server.
/// @param command The command to send to the server.
void sendCommand(int socket, const string& command) {
    // Writes the command to the specified socket. The command is converted to a C-style string using c_str().
    if (write(socket, command.c_str(), command.size()) < 0) {
        cerr << "Error writing to socket" << endl;
    }
}

/// @brief Receives a response from the server and prints it to the console.
/// @param socket The socket of the server.
void receiveResponse(int socket) {
    char buffer[1024];
    bzero(buffer, 1024); // Clears the buffer.
    int n = read(socket, buffer, 1023); // Reads from the socket into the buffer.
    if (n < 0) {
        cerr << "Error reading from socket" << endl;
    } else {
        cout << buffer;  // Simply print whatever is received
    }
}

/// @brief Prints a help message with available commands and their descriptions.
void printHelp() {
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
         << "AverageDistance u v\n"
         << "  - Calculate average distance between all pairs\n"
         << "  - Example: AverageDistance 1 3\n"
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
    int sockfd, portno = 9034; // the port of the server
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use localhost for testing
    serv_addr.sin_port = htons(portno); // Convert the port into big-endian

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Error connecting" << endl;
        return 1;
    }

    while (true) {
        cout << "Enter command (NewGraph, NewEdge, RemoveEdge, Kruskal, Prim, MSTWeight, LongestDistance, AverageDistance, PrintGraph, help, exit): ";
        string command;
        getline(cin, command); // Reads the entire input line into the command string.

        if (command == "help") {
            printHelp();
            continue;
        }

        sendCommand(sockfd, command + "\n");
        receiveResponse(sockfd);
        if (command == "exit") break;

        if (command.find("NewGraph") == 0) {
            int n, m;
            sscanf(command.c_str(), "NewGraph %d %d", &n, &m);
            for (int i = 0; i < m; ++i) {
                cout << "Enter edge " << i + 1 << " (format: u v weight): ";
                getline(cin, command);
                sendCommand(sockfd, command + "\n");
                receiveResponse(sockfd);
            }
        }
    }

    close(sockfd);
    return 0;

}
