#ifndef MSTFACTORY_H 
#define MSTFACTORY_H

#include "Graph.hpp"
#include <memory>  // For std::unique_ptr

// Forward declarations of KruskalMST and PrimMST classes
class KruskalMST;  
class PrimMST;     

/**
 * MSTFactory is responsible for creating objects of different MST (Minimum Spanning Tree)
 * algorithms such as Kruskal and Prim.
 */
class MSTFactory {
public:
    // Enum to specify the type of MST algorithm
    enum AlgorithmType { KRUSKAL, PRIM };

    /**
     * Creates and returns a unique pointer to a KruskalMST object.
     * @param g - reference to the graph object
     * @return A unique pointer to the KruskalMST object.
     */
    static std::unique_ptr<KruskalMST> createKruskalMST(Graph& g);

    /**
     * Creates and returns a unique pointer to a PrimMST object.
     * @param g - reference to the graph object
     * @return A unique pointer to the PrimMST object.
     */
    static std::unique_ptr<PrimMST> createPrimMST(Graph& g);

    // Virtual destructor for proper cleanup in case of inheritance
    virtual ~MSTFactory() {}
};

#endif // MSTFACTORY_H
