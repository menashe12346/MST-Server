#include "PrimMST.hpp"
#include <limits>
#include <queue>

double PrimMST::findMST() {
    int n = g.getNumNodes();
    std::vector<bool> inMST(n + 1, false);
    std::vector<double> key(n + 1, std::numeric_limits<double>::max());
    std::vector<int> parent(n + 1, -1);
    key[1] = 0; // Start from node 1

    using Pii = std::pair<double, int>; // Pair of (weight, vertex)
    std::priority_queue<Pii, std::vector<Pii>, std::greater<Pii>> pq;
    pq.push({0, 1});

    double mstWeight = 0;

    while (!pq.empty()) {
        int u = pq.top().second;
        double weight = pq.top().first;
        pq.pop();

        if (inMST[u]) continue;
        inMST[u] = true;
        mstWeight += weight;

        for (const auto& [v, w] : g.getAdjacencyList()[u]) {
            if (!inMST[v] && w < key[v]) {
                key[v] = w;
                parent[v] = u;
                pq.push({w, v});
            }
        }
    }

    // Collect the edges of the MST
    for (int i = 2; i <= n; ++i) {
        if (parent[i] != -1) {
            mstEdges.emplace_back(std::make_pair(parent[i], i), key[i]);
        }
    }

    return mstWeight;
}

std::vector<std::pair<std::pair<int, int>, double>> PrimMST::getMSTEdges() const {
    return mstEdges; // Return the edges of the MST
}
