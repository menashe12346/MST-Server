#include "Tree.hpp"
#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>

Tree::Tree(int n, const std::vector<std::pair<std::pair<int, int>, double>>& edges)
    : Graph(n, edges) {
    for (const auto& edge : edges) {
        addEdge(edge.first.first, edge.first.second, edge.second);
        addEdge(edge.first.second, edge.first.first, edge.second); // Undirected graph, add reverse edge
    }
}

double Tree::getMSTWeight() const {
    double totalWeight = 0;
    for (const auto& edge : getEdges()) {
        totalWeight += edge.second; // Sum up the weights of all edges
    }
    return totalWeight;
}

void Tree::dfs(int current, int parent, double currentWeight, int target, double &maxWeight, bool &found, std::vector<int>& currentPath, std::vector<int>& bestPath) {
    currentPath.push_back(current);
    if (current == target) {
        if (currentWeight > maxWeight) {
            maxWeight = currentWeight;
            bestPath = currentPath; // Store the path
        }
        found = true;
    }

    for (const auto& neighbor : getAdjacencyList()[current]) {
        if (neighbor.first != parent) { // Avoid going back to parent
            dfs(neighbor.first, current, currentWeight + neighbor.second, target, maxWeight, found, currentPath, bestPath);
            if (found) return; // Stop if target is found
        }
    }

    currentPath.pop_back(); // Backtrack
}

double Tree::longestDistance(int u, int v) {
    double maxWeight = 0;
    bool found = false;
    std::vector<int> currentPath, bestPath;
    dfs(u, -1, 0, v, maxWeight, found, currentPath, bestPath); // Start DFS from u to find v

    longestPath = bestPath; // Store the longest path found
    return found ? maxWeight : -1; // Return -1 if v is not reachable from u
}

std::vector<int> Tree::getLongestPath(int u, int v) {
    longestDistance(u, v); // First, calculate the longest distance
    return longestPath; // Return the stored longest path
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<int>>> Tree::floydWarshall() {
    int n = getNumNodes();
    std::vector<std::vector<double>> dist(n + 1, std::vector<double>(n + 1, std::numeric_limits<double>::infinity()));
    std::vector<std::vector<int>> next(n + 1, std::vector<int>(n + 1, -1));

    for (int i = 1; i <= n; ++i) {
        dist[i][i] = 0;
        for (const auto& neighbor : getAdjacencyList()[i]) {
            dist[i][neighbor.first] = neighbor.second;
            next[i][neighbor.first] = neighbor.first;
        }
    }

    for (int k = 1; k <= n; ++k) {
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= n; ++j) {
                if (dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }

    return {dist, next};
}

void Tree::reconstructPath(int u, int v, const std::vector<std::vector<int>>& next, std::vector<int>& path) {
    if (next[u][v] == -1) return;

    path.push_back(u);
    while (u != v) {
        u = next[u][v];
        path.push_back(u);
    }
}

double Tree::averageDistance() {
    int n = getNumNodes();
    auto [dist, next] = floydWarshall();

    double totalDistance = 0;
    int count = 0;

    std::cout << "Paths between all pairs of vertices:\n";

    for (int i = 1; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            if (dist[i][j] < std::numeric_limits<double>::infinity()) {
                totalDistance += dist[i][j];
                ++count;

                std::vector<int> path;
                reconstructPath(i, j, next, path);

                std::cout << "Path from " << i << " to " << j << ": ";
                for (size_t k = 0; k < path.size(); ++k) {
                    std::cout << path[k];
                    if (k < path.size() - 1) std::cout << " -> ";
                }
                std::cout << " (Distance: " << dist[i][j] << ")\n";
            }
        }
    }

    return (count > 0) ? totalDistance / count : -1;
}
