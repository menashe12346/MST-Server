#include "../hpp_files/Tree.hpp"
#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>

Tree::Tree(int n, const std::vector<std::pair<std::pair<int, int>, double>>& edges)
    : Graph(n, edges) {
    // Add edges to the tree, for undirected graph, add both directions
    for (const auto& edge : edges) {
        addEdge(edge.first.first, edge.first.second, edge.second);
        addEdge(edge.first.second, edge.first.first, edge.second); // Undirected graph
    }
}

double Tree::getMSTWeight() const {
    double totalWeight = 0;
    // Sum up the weight of all edges
    for (const auto& edge : getEdges()) {
        totalWeight += edge.second;
    }
    return totalWeight; // Return the total weight of the MST
}

double Tree::shortestDistance(int u, int v) {
    // Calculate all-pairs shortest distances using Floyd-Warshall
    auto [dist, next] = floydWarshall();
    return dist[u][v]; // Return the shortest distance between u and v
}

void Tree::dfs(int current, int parent, double currentWeight, int target, double &maxWeight, bool &found, std::vector<int>& currentPath, std::vector<int>& bestPath) {
    currentPath.push_back(current); // Add current node to the path
    if (current == target) { // If target node is found
        if (currentWeight > maxWeight) { // If the current path is the longest
            maxWeight = currentWeight; // Update the maximum weight
            bestPath = currentPath; // Update the best path
        }
        found = true; // Mark as found
    }

    // Explore neighbors of the current node
    for (const auto& neighbor : getAdjacencyList()[current]) {
        if (neighbor.first != parent) { // Avoid revisiting the parent node
            dfs(neighbor.first, current, currentWeight + neighbor.second, target, maxWeight, found, currentPath, bestPath);
            if (found) return; // If target is found, stop further exploration
        }
    }

    currentPath.pop_back(); // Backtrack
}

double Tree::longestDistance(int u, int v) {
    double maxWeight = 0;
    bool found = false;
    std::vector<int> currentPath, bestPath;
    // Use DFS to find the longest path between u and v
    dfs(u, -1, 0, v, maxWeight, found, currentPath, bestPath);

    longestPath = bestPath; // Store the longest path found
    return found ? maxWeight : -1; // Return the maximum weight or -1 if not found
}

std::vector<int> Tree::getLongestPath(int u, int v) {
    longestDistance(u, v); // Compute the longest path
    return longestPath; // Return the longest path
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<int>>> Tree::floydWarshall() {
    int n = getNumNodes(); // Get the number of nodes
    std::vector<std::vector<double>> dist(n + 1, std::vector<double>(n + 1, std::numeric_limits<double>::infinity()));
    std::vector<std::vector<int>> next(n + 1, std::vector<int>(n + 1, -1));

    // Initialize distances and paths for direct neighbors
    for (int i = 1; i <= n; ++i) {
        dist[i][i] = 0; // Distance to itself is 0
        for (const auto& neighbor : getAdjacencyList()[i]) {
            dist[i][neighbor.first] = neighbor.second; // Set the edge weight
            next[i][neighbor.first] = neighbor.first; // Set the next node in the path
        }
    }

    // Floyd-Warshall algorithm to find all pairs shortest paths
    for (int k = 1; k <= n; ++k) {
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= n; ++j) {
                if (dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j]; // Update the shortest distance
                    next[i][j] = next[i][k]; // Update the next node in the path
                }
            }
        }
    }

    return {dist, next}; // Return the distance matrix and the path matrix
}

void Tree::reconstructPath(int u, int v, const std::vector<std::vector<int>>& next, std::vector<int>& path) {
    if (next[u][v] == -1) return; // If there is no path between u and v, return

    path.push_back(u); // Add the starting node to the path
    while (u != v) { // Traverse through the path
        u = next[u][v]; // Move to the next node in the path
        path.push_back(u); // Add the node to the path
    }
}
