//
//  PickingPlan.h
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//

#ifndef PICKINGPLAN_H
#define PICKINGPLAN_H

#include "Libraries.h"

// Represents a single rack allocation entry for one product
struct AllocationNode {
    std::string rackID;       // e.g., "RK01"
    std::string locationID;   // e.g., "L04" (location where RK01 is installed)
    int allocatedQty;         // Units planned to be picked from this rack
    int distanceFromRef;      // Dijkstra shortest distance from reference location
    AllocationNode* next;     // Custom singly linked list pointer (no STL)

    AllocationNode(const std::string& rID, const std::string& lID, int qty, int dist)
        : rackID(rID), locationID(lID), allocatedQty(qty), distanceFromRef(dist), next(nullptr) {}
};

// Linked-list container holding all rack allocations for a requested product
struct AllocationList {
    AllocationNode* head;
    AllocationNode* tail;
    int totalAllocated;       // Sum of allocatedQty across all nodes
    bool isFullyAllocated;    // True if totalAllocated == requestedQty

    AllocationList()
        : head(nullptr), tail(nullptr), totalAllocated(0), isFullyAllocated(false) {}
};

// Lightweight candidate node used internally while evaluating racks for one product
struct CandidateRack {
    std::string rackID;
    std::string locationID;
    int availableQty;
    int distance;
    bool used;
    CandidateRack* next;

    CandidateRack(const std::string& rID, const std::string& lID, int avail, int dist)
        : rackID(rID), locationID(lID), availableQty(avail), distance(dist), used(false), next(nullptr) {}
};

// Forward declarations for Hamza's (Module 1) and Ahmed's (Module 2) classes
// (Replace with #include "WarehouseGraph.h" and #include "ProductAVL.h" during integration)
class WarehouseGraph;
class ProductAVL;

// Helper functions to manage dynamic memory cleanly (no memory leaks)
void appendAllocationNode(AllocationList& list, const std::string& rID, const std::string& lID, int qty, int dist);
void clearAllocationList(AllocationList& list);
void clearCandidateList(CandidateRack*& head);

// Module 5 - Function 1: Deterministic Rack Allocation Rule
// Returns true if full requested quantity was allocated, false if stock shortage or unreachable
bool allocateRacksForProduct(
    const std::string& prodID,
    int reqQty,
    const std::string& refLocationID,
    ProductAVL& inventory,
    WarehouseGraph& graph,
    AllocationList& outList
                             ){
    
};

#endif // PICKINGPLAN_H
