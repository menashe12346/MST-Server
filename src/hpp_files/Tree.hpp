#ifndef TREE_H 
#define TREE_H

#include "Graph.hpp"
#include <vector>

/**
 * Tree class inheriting from the Graph class. 
 * Represents a tree structure with additional functionalities for calculating distances and paths.
 */
class Tree : public Graph {
public:
    /**
     * Constructor that initializes a tree with a specified number of nodes and edges.
     * @param n - Number of nodes in the tree.
     * @param edges - A vector of pairs representing edges and their weights.
     */
    Tree(int n, const std::vector<std::pair<std::pair<int, int>, double>>& edges);

    /**
     * Returns the total weight of the Minimum Spanning Tree (MST).
     * @return Total MST weight.
     */
    double getMSTWeight() const;

    /**
     * Returns the longest distance between two nodes in the tree.
     * @param u - First node.
     * @param v - Second node.
     * @return Longest distance between nodes u and v.
     */
    double longestDistance(int u, int v);

    /**
     * Returns the shortest distance between two nodes in the tree.
     * @param u - First node.
     * @param v - Second node.
     * @return Shortest distance between nodes u and v.
     */
    double shortestDistance(int u, int v);

    /**
     * Returns the longest path between two nodes in the tree.
     * @param u - First node.
     * @param v - Second node.
     * @return A vector of integers representing the nodes in the longest path from u to v.
     */
    std::vector<int> getLongestPath(int u, int v);

    /**
     * Executes the Floyd-Warshall algorithm to find all pairs shortest paths.
     * @return A pair of matrices:
     *         - First matrix is the distances between each pair of nodes.
     *         - Second matrix holds the next node in the path for reconstructing the path.
     */
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<int>>> floydWarshall();

    /**
     * Reconstructs the path between two nodes based on the Floyd-Warshall results.
     * @param u - Start node.
     * @param v - End node.
     * @param next - Matrix from Floyd-Warshall that holds the next node in the path.
     * @param path - Vector to store the reconstructed path from u to v.
     */
    void reconstructPath(int u, int v, const std::vector<std::vector<int>>& next, std::vector<int>& path);

private:
    std::vector<int> longestPath;  // Stores the longest path between two nodes.

    /**
     * Depth-first search (DFS) helper function to find the longest path between two nodes.
     * @param current - Current node in the DFS.
     * @param parent - Parent node of the current node.
     * @param currentWeight - Weight of the current path.
     * @param target - Target node for the search.
     * @param maxWeight - Maximum weight found so far in the search.
     * @param found - Boolean flag to indicate if the target node has been found.
     * @param currentPath - The current path being explored.
     * @param bestPath - The best (longest) path found so far.
     */
    void dfs(int current, int parent, double currentWeight, int target, double &maxWeight, bool &found, 
             std::vector<int>& currentPath, std::vector<int>& bestPath);
};

#endif // TREE_H
