//
//  PickingPlan.h
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//

#ifndef PICKINGPLAN_H
#define PICKINGPLAN_H

#include "Library.h"

// ============================================================================
// LOCAL STUB STRUCTURES FOR STEP 1 (Your own simple structures for now;
// swap/align with Ahmed's & Hamza's implementations during final merge)
// ============================================================================

struct RackEntry {
    std::string rackID;
    int totalQty;
    int reservedQty;
    RackEntry* next;

    RackEntry(const std::string& id, int total, int reserved, RackEntry* nxt = nullptr)
        : rackID(id), totalQty(total), reservedQty(reserved), next(nxt) {}
};

struct ProductNode {
    std::string prodID;
    std::string name;
    double unitWeight;
    int reorderLevel;
    RackEntry* rackHead;
    ProductNode* left;
    ProductNode* right;

    ProductNode(const std::string& id, const std::string& prodName, double weight, int reorder)
        : prodID(id), name(prodName), unitWeight(weight), reorderLevel(reorder),
          rackHead(nullptr), left(nullptr), right(nullptr) {}
};

struct LocalInventoryTree {
    ProductNode* root;

    LocalInventoryTree() : root(nullptr) {}

    ProductNode* searchProduct(const std::string& prodID) const {
        ProductNode* cur = root;
        while (cur != nullptr) {
            if (prodID == cur->prodID) return cur;
            cur = (prodID < cur->prodID) ? cur->left : cur->right;
        }
        return nullptr;
    }
};

struct LocalWarehouseGraph {
    // Maps rackID ("RK01") -> warehouse locationID ("L04")
    virtual std::string getRackLocation(const std::string& rackID) const = 0;

    // Returns shortest OPEN Dijkstra distance from startLoc to endLoc,
    // or -1 if unreachable / invalid location
    virtual int getShortestDistance(const std::string& startLoc, const std::string& endLoc) const = 0;

    virtual ~LocalWarehouseGraph() {}
};

// ============================================================================
// STEP 1 DATA STRUCTURES & FUNCTION DECLARATIONS
// ============================================================================

struct AllocationNode {
    std::string rackID;
    std::string locationID;
    int allocatedQty;
    int distanceFromRef;
    AllocationNode* next;

    AllocationNode(const std::string& rID, const std::string& lID, int qty, int dist)
        : rackID(rID), locationID(lID), allocatedQty(qty), distanceFromRef(dist), next(nullptr) {}
};

class AllocationList {
public:
    AllocationNode* head;
    AllocationNode* tail;
    int totalAllocated;
    bool isFullyAllocated;

    AllocationList()
        : head(nullptr), tail(nullptr), totalAllocated(0), isFullyAllocated(false) {}

    ~AllocationList();
};

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
class CandidateList{
public:
    CandidateRack * head;
    CandidateRack * tail;
    CandidateList(){
        head = nullptr;
        tail = nullptr;
    }
};
// Memory cleanup helpers
void appendAllocationNode(AllocationList& list, const std::string& rID, const std::string& lID, int qty, int dist);
void clearAllocationList(AllocationList& list);
void clearCandidateList(CandidateRack*& head);

// Module 5 - Step 1: Deterministic Rack Allocation Rule
bool allocateRacksForProduct(
    const std::string& prodID,
    int reqQty,
    const std::string& refLocationID,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    AllocationList& outList
);

// ============================================================================
// STEP 2: NEAREST-REACHABLE-RACK VISIT ORDERING & MULTI-TRIP SPLITTING
// ============================================================================

// Represents a single ordered stop along a picking route or trip
struct OrderedPickStop {
    std::string prodID;
    std::string rackID;
    std::string locationID;
    int pickQty;
    int legDistance;          // Shortest distance from previous location to this stop
    bool visited;
    OrderedPickStop* next;

    OrderedPickStop(const std::string& pID, const std::string& rID, const std::string& lID, int qty, int dist)
        : prodID(pID), rackID(rID), locationID(lID), pickQty(qty), legDistance(dist), visited(false), next(nullptr) {}
};

// Custom linked list of ordered picking stops
struct OrderedStopList {
    OrderedPickStop* head;
    OrderedPickStop* tail;
    int stopCount;
    int totalUnits;

    OrderedStopList()
        : head(nullptr), tail(nullptr), stopCount(0), totalUnits(0) {}
};

// Represents one trip from the current/packing location across one or more racks
struct PickingTripNode {
    int tripNumber;           // 1-based trip index
    std::string startLocationID;
    std::string endLocationID;    // e.g., packing/drop-off station ID
    OrderedPickStop* stopsHead;
    OrderedPickStop* stopsTail;
    int tripUnits;
    int totalTripDistance;
    PickingTripNode* next;

    PickingTripNode(int num, const std::string& startLoc, const std::string& endLoc)
        : tripNumber(num), startLocationID(startLoc), endLocationID(endLoc),
          stopsHead(nullptr), stopsTail(nullptr), tripUnits(0), totalTripDistance(0), next(nullptr) {}
};

// Custom linked list of split trips for a picking plan
class PickingTripList {
public:
    PickingTripNode* head;
    PickingTripNode* tail;
    int tripCount;
    int totalPlannedUnits;

    PickingTripList()
        : head(nullptr), tail(nullptr), tripCount(0), totalPlannedUnits(0) {}

    ~PickingTripList();
};

// Step 2 memory-cleanup helpers
void appendOrderedStop(OrderedStopList& list, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int dist);
void clearOrderedStopList(OrderedStopList& list);
void appendStopToTrip(PickingTripNode* trip, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int legDist);
void appendTripNode(PickingTripList& tripList, const std::string& startLoc, const std::string& endLoc);
void clearPickingTripList(PickingTripList& tripList);

// Orders allocated rack stops by nearest reachable rack from startLocationID
bool orderStopsByNearestReachable(
    const AllocationList& allocations,
    const std::string& prodID,
    const std::string& startLocationID,
    const LocalWarehouseGraph& graph,
    OrderedStopList& orderedOut
);

// Splits ordered stops into multi-trip routes based on robot carry capacity
bool splitStopsIntoTrips(
    const OrderedStopList& orderedStops,
    int robotCapacity,
    const std::string& packingStationID,
    const LocalWarehouseGraph& graph,
    PickingTripList& tripsOut
);
#endif // PICKINGPLAN_H
