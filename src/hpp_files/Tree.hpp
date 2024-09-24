#ifndef TREE_H 
#define TREE_H

#include "Graph.hpp"
#include <vector>

class Tree : public Graph {
public:
    Tree(int n, const std::vector<std::pair<std::pair<int, int>, double>>& edges);

    double getMSTWeight() const;
    double longestDistance(int u, int v);
    double shortestDistance(int u, int v);

    std::vector<int> getLongestPath(int u, int v);

    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<int>>> floydWarshall();
    void reconstructPath(int u, int v, const std::vector<std::vector<int>>& next, std::vector<int>& path);

private:
    std::vector<int> longestPath;

    void dfs(int current, int parent, double currentWeight, int target, double &maxWeight, bool &found, std::vector<int>& currentPath, std::vector<int>& bestPath);
};

#endif
