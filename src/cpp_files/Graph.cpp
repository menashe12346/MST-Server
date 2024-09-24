#include "../hpp_files/Graph.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

Graph::Graph(int n, const vector<pair<pair<int, int>, double>>& edges) : n(n) {
    // Initialize the graph with n+1 nodes to accommodate 1-based indexing
    graph.resize(n + 1);
    // Initialize the edges vector
    this->edges = edges; // Store edges for Kruskal's algorithm
    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double weight = edge.second;
        // Add edge with weight to the graph
        graph[u].push_back({v, weight});
        // Since it's undirected, add the reverse edge
        graph[v].push_back({u, weight});
    }
}

void Graph::printGraph() const {
    cout << "\nCurrent Graph (Adjacency List with Weights):\n";
    for (int i = 1; i <= n; ++i) {
        cout << i << " -> ";
        for (const auto& neighbor : graph[i]) {
            cout << "(" << neighbor.first << ", " << neighbor.second << ") "; // Print node and weight
        }
        cout << endl;
    }
}

void Graph::addEdge(int u, int v, double weight) {
    graph[u].push_back({v, weight}); // Add edge with weight to the graph
    graph[v].push_back({u, weight}); // Add reverse edge
}

void Graph::removeEdge(int u, int v) {
    graph[u].remove_if([v](const pair<int, double>& p) { return p.first == v; }); // Remove edge from the graph
    graph[v].remove_if([u](const pair<int, double>& p) { return p.first == u; }); // Remove reverse edge from the graph
}
