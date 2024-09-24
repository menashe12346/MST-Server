#include "../hpp_files/MSTFactory.hpp"
#include "../hpp_files/KruskalMST.hpp"
#include "../hpp_files/PrimMST.hpp"

std::unique_ptr<KruskalMST> MSTFactory::createKruskalMST(Graph& g) {
    // Create and return a unique pointer to a KruskalMST instance using the provided graph
    return std::make_unique<KruskalMST>(g);
}

std::unique_ptr<PrimMST> MSTFactory::createPrimMST(Graph& g) {
    // Create and return a unique pointer to a PrimMST instance using the provided graph
    return std::make_unique<PrimMST>(g);
}
