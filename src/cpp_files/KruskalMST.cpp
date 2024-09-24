#include "../hpp_files/KruskalMST.hpp"
#include <algorithm>
#include <numeric>

int KruskalMST::find(int u, std::vector<int>& parent) {
    if (u != parent[u]) {
        parent[u] = find(parent[u], parent); // Path compression to flatten the tree
    }
    return parent[u]; // Return the representative (root) of the set containing u
}

void KruskalMST::unite(int u, int v, std::vector<int>& parent, std::vector<int>& rank) {
    int rootU = find(u, parent); // Find root of u
    int rootV = find(v, parent); // Find root of v
    if (rootU != rootV) {
        // Union by rank to keep the tree shallow
        if (rank[rootU] > rank[rootV])
            parent[rootV] = rootU; // rootU becomes the parent of rootV
        else if (rank[rootU] < rank[rootV])
            parent[rootU] = rootV; // rootV becomes the parent of rootU
        else {
            parent[rootV] = rootU; // Arbitrarily make rootU the parent of rootV
            rank[rootU]++;         // Increase rank of rootU
        }
    }
}

double KruskalMST::findMST() {
    int n = g.getNumNodes(); // Get the number of nodes in the graph
    auto edges = g.getEdges(); // Retrieve all edges from the graph
    
    // Sort edges in ascending order based on their weight
    std::sort(edges.begin(), edges.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second; // Compare weights
    });

    std::vector<int> parent(n + 1), rank(n + 1, 0); // Initialize parent and rank arrays
    std::iota(parent.begin(), parent.end(), 0); // Initialize parent array with each node being its own parent

    double mstWeight = 0; // Variable to store the total weight of the MST
    mstEdges.clear(); // Clear previous MST edges

    // Iterate through the sorted edges
    for (const auto& edge : edges) {
        int u = edge.first.first; // Start vertex of the edge
        int v = edge.first.second; // End vertex of the edge
        double weight = edge.second; // Weight of the edge

        // Check if u and v belong to different sets (to avoid cycles)
        if (find(u, parent) != find(v, parent)) {
            mstWeight += weight; // Add the weight of the edge to the MST
            unite(u, v, parent, rank); // Union the sets of u and v
            mstEdges.push_back(edge); // Store the edge in the MST
        }
    }
    return mstWeight; // Return the total weight of the MST
}

std::vector<std::pair<std::pair<int, int>, double>> KruskalMST::getMSTEdges() const {
    return mstEdges; // Return the stored MST edges
}
