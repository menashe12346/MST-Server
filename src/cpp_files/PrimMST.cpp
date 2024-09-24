#include "../hpp_files/PrimMST.hpp"
#include <limits>
#include <queue>

double PrimMST::findMST() {
    int n = g.getNumNodes(); // Get the number of nodes
    std::vector<bool> inMST(n + 1, false); // Track nodes included in MST
    std::vector<double> key(n + 1, std::numeric_limits<double>::max()); // Track the minimum weights for edges
    std::vector<int> parent(n + 1, -1); // Store the parent nodes
    key[1] = 0; // Start from node 1, initialize its key to 0

    using Pii = std::pair<double, int>; // Pair representing (weight, vertex)
    std::priority_queue<Pii, std::vector<Pii>, std::greater<Pii>> pq; // Min-heap to select edges by minimum weight
    pq.push({0, 1}); // Push the starting node (1) with weight 0

    double mstWeight = 0; // Variable to store the total weight of the MST

    while (!pq.empty()) {
        int u = pq.top().second; // Get the vertex with the smallest weight
        double weight = pq.top().first; // Get the corresponding weight
        pq.pop(); // Remove the element from the priority queue

        if (inMST[u]) continue; // If the vertex is already in the MST, skip it
        inMST[u] = true; // Mark the vertex as included in the MST
        mstWeight += weight; // Add the edge's weight to the total MST weight

        // Iterate over all adjacent nodes of the current vertex
        for (const auto& [v, w] : g.getAdjacencyList()[u]) {
            // If the vertex is not in the MST and the current edge weight is less than the stored key
            if (!inMST[v] && w < key[v]) {
                key[v] = w; // Update the minimum weight to reach vertex v
                parent[v] = u; // Set the parent of vertex v
                pq.push({w, v}); // Push the updated vertex and weight into the priority queue
            }
        }
    }

    // Collect the edges that form the MST
    for (int i = 2; i <= n; ++i) {
        if (parent[i] != -1) {
            // Store each edge in the MST (parent[i], i) with its weight key[i]
            mstEdges.emplace_back(std::make_pair(parent[i], i), key[i]);
        }
    }

    return mstWeight; // Return the total weight of the MST
}

std::vector<std::pair<std::pair<int, int>, double>> PrimMST::getMSTEdges() const {
    return mstEdges; // Return the edges of the MST
}
