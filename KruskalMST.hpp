#ifndef KRUSKAL_MST_H
#define KRUSKAL_MST_H

#include "Graph.hpp"
#include <vector>

class KruskalMST {
public:
    KruskalMST(Graph& g) : g(g) {}
    double findMST();
    std::vector<std::pair<std::pair<int, int>, double>> getMSTEdges() const; // Method to get MST edges

private:
    Graph& g;
    int find(int u, std::vector<int>& parent);
    void unite(int u, int v, std::vector<int>& parent, std::vector<int>& rank);
    std::vector<std::pair<std::pair<int, int>, double>> mstEdges; // Store MST edges
};

#endif
