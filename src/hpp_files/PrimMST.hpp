#ifndef PRIM_MST_H 
#define PRIM_MST_H

#include "Graph.hpp"
#include <vector>  // Include vector for storing edges

/**
 * Class that implements Prim's algorithm for finding the Minimum Spanning Tree (MST).
 * The MST edges are stored in the `mstEdges` vector.
 */
class PrimMST {
public:
    /**
     * Constructor that initializes the PrimMST with a reference to the graph.
     * @param g - reference to the graph object
     */
    PrimMST(Graph& g) : g(g) {}

    /**
     * Method to find the Minimum Spanning Tree (MST) using Prim's algorithm.
     * @return The total weight of the MST.
     */
    double findMST();

    /**
     * Getter method to retrieve the edges of the MST.
     * @return A vector of pairs containing the edges of the MST and their weights.
     */
    std::vector<std::pair<std::pair<int, int>, double>> getMSTEdges() const;

private:
    Graph& g;   // Reference to the graph
    std::vector<std::pair<std::pair<int, int>, double>> mstEdges;  // Vector to store MST edges
};

#endif // PRIM_MST_H
