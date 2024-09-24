#ifndef KRUSKAL_MST_H
#define KRUSKAL_MST_H

#include "Graph.hpp"   // Include the Graph class header
#include <vector>      // Include vector for dynamic array support

/**
 * Class that implements Kruskal's algorithm for finding the Minimum Spanning Tree (MST).
 * The MST is stored in the `mstEdges` vector.
 */
class KruskalMST {
public:
    /**
     * Constructor that initializes the KruskalMST with a reference to the graph.
     * @param g - reference to the graph object
     */
    KruskalMST(Graph& g) : g(g) {}

    /**
     * Method to find the Minimum Spanning Tree (MST) using Kruskal's algorithm.
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
    std::vector<std::pair<std::pair<int, int>, double>> mstEdges;  // Vector to store the edges of the MST

    /**
     * Helper function to find the root of a node `u` in the disjoint set (used in union-find).
     * @param u - the node to find the root for
     * @param parent - the parent array representing the disjoint set
     * @return The root of the node `u`.
     */
    int find(int u, std::vector<int>& parent);

    /**
     * Helper function to unite two sets containing `u` and `v` in the disjoint set (union by rank).
     * @param u - the first node
     * @param v - the second node
     * @param parent - the parent array representing the disjoint set
     * @param rank - the rank array to optimize union operations
     */
    void unite(int u, int v, std::vector<int>& parent, std::vector<int>& rank);
};

#endif // KRUSKAL_MST_H
