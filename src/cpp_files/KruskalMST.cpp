#include "../hpp_files/KruskalMST.hpp"
#include <algorithm>
#include <numeric>

int KruskalMST::find(int u, std::vector<int>& parent) {
    if (u != parent[u]) {
        parent[u] = find(parent[u], parent); // Path compression
    }
    return parent[u];
}

void KruskalMST::unite(int u, int v, std::vector<int>& parent, std::vector<int>& rank) {
    int rootU = find(u, parent);
    int rootV = find(v, parent);
    if (rootU != rootV) {
        if (rank[rootU] > rank[rootV])
            parent[rootV] = rootU;
        else if (rank[rootU] < rank[rootV])
            parent[rootU] = rootV;
        else {
            parent[rootV] = rootU;
            rank[rootU]++;
        }
    }
}

double KruskalMST::findMST() {
    int n = g.getNumNodes();
    auto edges = g.getEdges();
    
    // Sort edges based on weight
    std::sort(edges.begin(), edges.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second;
    });

    std::vector<int> parent(n + 1), rank(n + 1, 0);
    std::iota(parent.begin(), parent.end(), 0); // Initialize parent array

    double mstWeight = 0;
    mstEdges.clear(); // Clear previous MST edges

    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double weight = edge.second;

        if (find(u, parent) != find(v, parent)) {
            mstWeight += weight;
            unite(u, v, parent, rank);
            mstEdges.push_back(edge); // Store the edge in the MST
        }
    }
    return mstWeight;
}

std::vector<std::pair<std::pair<int, int>, double>> KruskalMST::getMSTEdges() const {
    return mstEdges; // Return the stored MST edges
}
