#ifndef TREE_H
#define TREE_H

#include "Graph.hpp"
#include <vector>

class Tree : public Graph {
public:
    Tree(int n, const std::vector<std::pair<std::pair<int, int>, double>>& edges);

    double getMSTWeight() const; // Get the total weight of the MST
    double longestDistance(int u, int v); // Find the longest distance between two vertices
    double averageDistance(); // Find the average distance between all pairs of vertices

    std::vector<int> getLongestPath(int u, int v); // Return the longest path between two vertices

    // Floyd-Warshall implementation to find the shortest distances and paths between all pairs
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<int>>> floydWarshall();

    // Helper function to reconstruct the path between two nodes
    void reconstructPath(int u, int v, const std::vector<std::vector<int>>& next, std::vector<int>& path);
    
private:
    std::vector<int> longestPath; // Store the longest path

    // Helper function for longest distance calculation
    void dfs(int current, int parent, double currentWeight, int target, double &maxWeight, bool &found, std::vector<int>& currentPath, std::vector<int>& bestPath);
};

#endif
