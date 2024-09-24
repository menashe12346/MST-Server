#include "MSTFactory.hpp"
#include "KruskalMST.hpp"
#include "PrimMST.hpp"

std::unique_ptr<KruskalMST> MSTFactory::createKruskalMST(Graph& g) {
    return std::make_unique<KruskalMST>(g);
}

std::unique_ptr<PrimMST> MSTFactory::createPrimMST(Graph& g) {
    return std::make_unique<PrimMST>(g);
}
