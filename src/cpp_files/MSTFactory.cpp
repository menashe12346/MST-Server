#include "../hpp_files/MSTFactory.hpp"
#include "../hpp_files/KruskalMST.hpp"
#include "../hpp_files/PrimMST.hpp"

std::unique_ptr<KruskalMST> MSTFactory::createKruskalMST(Graph& g) {
    return std::make_unique<KruskalMST>(g);
}

std::unique_ptr<PrimMST> MSTFactory::createPrimMST(Graph& g) {
    return std::make_unique<PrimMST>(g);
}
