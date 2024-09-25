
# MST-Server

## Project Overview

This project addresses the **Minimal Spanning Tree (MST)** problem on a **weighted undirected graph**. It implements a server-client architecture using multithreaded design patterns and offers several MST-related calculations. The project supports the **Prim** and **Kruskal** algorithms for calculating the MST.

### Key Features:
- **Graph Data Structure**: Implements essential graph operations such as adding and removing edges.
- **MST Algorithms**: Supports Prim and Kruskal algorithms.
- **Measurements**:
  - Total weight of the MST
  - Longest distance between two vertices
  - Average distance between all edges in the graph
  - Shortest distance between two vertices where the edge belongs to the MST
- **Server Architecture**: 
  - Processes graph changes and MST-related requests
  - Utilizes the **Leader-Follower thread pool** and **Pipeline pattern** for efficient handling of tasks
  - Implements Active Object for asynchronous handling
- **Valgrind Analysis**: Provides memory and thread checks using Valgrind tools.

## Installation & Setup

### Clone the Repository
```bash
git clone https://github.com/menashe12346/MST-Server.git
cd MST-Server
```

### Compilation
Compile the project using the provided `Makefile`:
```bash
make
```

### Running the Server

There are two types of servers in the project:

1. **Leader-Follower Server** (`LFServer`): This server uses the Leader-Follower thread pool design pattern for handling client requests. To run this server:
   ```bash
   ./LFServer
   ```

2. **Pipeline Server** (`PipelineServer`): This server uses the Pipeline pattern for processing requests in stages. To run this server:
   ```bash
   ./PipelineServer
   ```

Choose one of the servers based on your requirements.

### Running the Client
To connect to the server and perform MST calculations:
```bash
./client
```

## Commands & Usage

The client can send the following commands to the server:

1. **NewGraph n**: Create a new graph with `n` vertices.
2. **NewEdge u v w**: Add an edge between vertices `u` and `v` with weight `w`.
3. **RemoveEdge u v**: Remove the edge between vertices `u` and `v`.
4. **Kruskal**: Execute Kruskal's MST algorithm.
5. **Prim**: Execute Prim's MST algorithm.
6. **MSTWeight**: Retrieve the total weight of the MST.
7. **LongestDistance**: Get the longest distance between two vertices in the MST.
8. **AverageDistance**: Calculate the average distance between all pairs of vertices.
9. **ShortestPath**: Find the shortest path between two vertices in the MST.
10. **PrintGraph**: Print the current state of the graph.
11. **help**: Display a list of available commands.
12. **exit**: Disconnect the client from the server.

## Design Patterns

This project leverages several design patterns:
- **Factory Pattern**: Allows switching between different MST algorithms dynamically.
- **Pipeline Pattern**: Breaks down the process into stages, where each stage handles one part of the job (like reading data, processing it, and responding). It allows multiple requests to be processed concurrently at different stages, increasing efficiency.
- **Leader-Follower Thread Pool**: Optimizes multithreading by having one leader thread handle an event while follower threads wait. Once the leader thread completes, another follower thread becomes the leader, ensuring efficient task distribution and minimizing contention between threads.

## Valgrind and Code Coverage

### Valgrind

To test the server for memory leaks using Valgrind, you can run the following commands provided in the `Makefile`:

For **pipelineServer**:
```bash
make valgrind_pipelineServer
```

For **LFServer**:
```bash
make valgrind_LFServer
```

This will run Valgrind with the `--leak-check=full` option and automatically handle both the server and client processes.

Example output of Valgrind for `LFServer`:
![צילום מסך 2024-09-25 163512](https://github.com/user-attachments/assets/7373a2a5-f378-4233-8469-1cd4436c8873)

### Code Coverage

After running tests, you can check the coverage by using `gcov`:

For the client:
```bash
gcov client.cpp
```

For the servers:
```bash
gcov LFServer.cpp
gcov pipelineServer.cpp
```

This generates `.gcov` files showing the coverage percentage for each file.
