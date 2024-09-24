#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <list>
#include <vector>
#include <utility> // For std::pair
#include <queue>
#include <algorithm>

using namespace std;

class Graph {
public:
    /// @brief Constructor to initialize the graph with n nodes and edges.
    /// @param n The number of nodes in the graph.
    /// @param edges The edges of the graph (pair of nodes with weights).
    Graph(int n, const vector<pair<pair<int, int>, double>>& edges);

    /// @brief Adds an edge with weight to the graph.
    void addEdge(int u, int v, double weight);

    /// @brief Removes an edge from the graph.
    void removeEdge(int u, int v);

    /// @brief Prints the current state of the graph.
    void printGraph() const;

    /// @brief Returns the number of nodes in the graph.
    int getNumNodes() const { return n; }

    /// @brief Returns all the edges in the graph.
    vector<pair<pair<int, int>, double>> getEdges() const { return edges; }

    /// @brief Returns the adjacency list for Prim's algorithm.
    const vector<list<pair<int, double>>>& getAdjacencyList() const { return graph; }

private:
    int n;  ///< Number of nodes in the graph.
    vector<list<pair<int, double>>> graph;  ///< Adjacency list of the graph (with weights).
    vector<pair<pair<int, int>, double>> edges;  ///< Stores edges for Kruskal's algorithm.
};

#endif
