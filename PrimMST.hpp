#ifndef PRIM_MST_H
#define PRIM_MST_H

#include "Graph.hpp"
#include <vector>

class PrimMST {
public:
    PrimMST(Graph& g) : g(g) {}
    double findMST();
    std::vector<std::pair<std::pair<int, int>, double>> getMSTEdges() const; // Method to get MST edges

private:
    Graph& g;
    std::vector<std::pair<std::pair<int, int>, double>> mstEdges; // Store MST edges
};

#endif
