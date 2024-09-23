#ifndef MSTFACTORY_H
#define MSTFACTORY_H

#include "Graph.hpp"
#include <memory>

// Forward declarations
class KruskalMST; // Forward declaration
class PrimMST;    // Forward declaration

class MSTFactory {
public:
    enum AlgorithmType { KRUSKAL, PRIM };

    static std::unique_ptr<KruskalMST> createKruskalMST(Graph& g);
    static std::unique_ptr<PrimMST> createPrimMST(Graph& g);
    virtual ~MSTFactory() {}
};

#endif
