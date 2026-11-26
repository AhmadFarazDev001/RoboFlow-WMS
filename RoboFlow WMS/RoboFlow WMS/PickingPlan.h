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
// LOCAL STUB STRUCTURES FOR STEP 1
// (Lightweight interfaces; align with Ahmed's & Hamza's headers during merge)
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

    // Standard BST/AVL lookup by ProductID
    ProductNode* searchProduct(const std::string& prodID) const {
        ProductNode* cur = root;
        while (cur != nullptr) {
            if (prodID == cur->prodID) return cur;
            cur = (prodID < cur->prodID) ? cur->left : cur->right;
        }
        return nullptr;
    }

    //inventory hooks for module 5 & 6
    virtual bool reserveStock(const std::string& prodID, const std::string& rackID, const std::string& orderID, int qty) { return false; }
    virtual void releaseOrderReservations(const std::string& orderID) {}
    virtual void releaseSingleStopReservation(const std::string& prodID, const std::string& rackID, const std::string& orderID, int qty) {}
    virtual bool commitPickedStock(const std::string& prodID, const std::string& rackID, const std::string& orderID, int qty) { return false; }
    virtual ~LocalInventoryTree() {}
};

struct LocalWarehouseGraph {
    // Maps rackID ("RK01") -> warehouse locationID ("L04")
    virtual std::string getRackLocation(const std::string& rackID) const = 0;

    // Returns shortest OPEN Dijkstra distance from startLoc to endLoc,
    // or -1 if unreachable / invalid location
    virtual int getShortestDistance(const std::string& startLoc, const std::string& endLoc) const = 0;

    //graph hooks for packing station & open pathway validation
    virtual std::string getNearestPackingStation(const std::string& fromLoc, int& outDist) const { outDist = -1; return ""; }
    virtual bool isRouteStillOpen(const std::string& startLoc, const std::string& endLoc) const { return true; }

    virtual ~LocalWarehouseGraph() {}
};

// ============================================================================
// STEP 1: DETERMINISTIC RACK ALLOCATION DATA STRUCTURES & DECLARATIONS
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

class CandidateList {
public:
    CandidateRack* head;
    CandidateRack* tail;

    CandidateList() : head(nullptr), tail(nullptr) {}
    ~CandidateList();
};

// Step 1 linked-list & memory cleanup helpers
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

    ~OrderedStopList();
};

// Represents one trip from the current/packing location across one or more racks
struct PickingTripNode {
    int tripNumber;               // 1-based trip index
    std::string startLocationID;
    std::string endLocationID;    // Packing/drop-off station ID
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

// Step 2 linked-list & memory cleanup helpers
void appendOrderedStop(OrderedStopList& list, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int dist);
void clearOrderedStopList(OrderedStopList& list);

void appendStopToTrip(PickingTripNode* trip, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int legDist);
void appendTripNode(PickingTripList& tripList, const std::string& startLoc, const std::string& endLoc);
void clearPickingTripList(PickingTripList& tripList);

bool removeAllocationByRack(AllocationList& list, const std::string& targetRackID);

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

// ============================================================================
// STEP 3: MODULE 5 - ROBOT SELECTION & PICKING PLAN GENERATION
// ============================================================================

struct OrderItemNode {
    std::string prodID;
    int requiredQty;
    int deliveredQty;
    OrderItemNode* next;

    OrderItemNode(const std::string& pID, int req, int del = 0)
        : prodID(pID), requiredQty(req), deliveredQty(del), next(nullptr) {}
};

struct OrderNode {
    std::string orderID;
    std::string customerID;
    std::string status;
    std::string assignedRobotID;
    OrderItemNode* itemsHead;
    OrderNode* next;

    OrderNode(const std::string& oID, const std::string& cID)
        : orderID(oID), customerID(cID), status("Pending"), assignedRobotID(""),
          itemsHead(nullptr), next(nullptr) {}
};

struct LocalOrderQueue {
    virtual OrderNode* peekFront() const = 0;
    virtual OrderNode* getNextPending(OrderNode* current) const = 0;
    virtual void removeOrder(const std::string& orderID) = 0;
    virtual void pushFront(OrderNode* order) = 0;
    virtual ~LocalOrderQueue() {}
};

struct LocalRobot {
    std::string robotID;
    std::string type;
    std::string currentLocationID;
    double batteryLevel;
    double maxCapacity;
    std::string status;
    std::string assignedTaskID;
    LocalRobot* next;

    virtual double getBatteryRate() const = 0;
    virtual bool isEligibleForOrder(double totalWeight, double maxUnitWeight) const = 0;
    virtual ~LocalRobot() {}
};

struct LocalRobotFleet {
    LocalRobot* head;
    LocalRobotFleet() : head(nullptr) {}
    virtual LocalRobot* searchRobot(const std::string& robotID) const = 0;
    virtual void assignRobot(const std::string& robotID, const std::string& orderID) = 0;
    virtual void releaseRobot(const std::string& robotID) = 0;
    virtual ~LocalRobotFleet() {}
};

//extra per-trip info for execution & battery checks
struct TripMetaNode {
    int tripNumber;
    double tripWeight;
    double tripBattery;
    bool isExecuted;
    TripMetaNode* next;

    TripMetaNode(int num, double w, double batt)
        : tripNumber(num), tripWeight(w), tripBattery(batt), isExecuted(false), next(nullptr) {}
};

class PickingPlan {
public:
    std::string orderID;
    std::string assignedRobotID;
    PickingTripList trips;
    TripMetaNode* metaHead;
    TripMetaNode* metaTail;
    int totalPlanDistance;
    double totalEstimatedBattery;
    bool isValid;

    PickingPlan()
        : orderID(""), assignedRobotID(""), metaHead(nullptr), metaTail(nullptr),
          totalPlanDistance(0), totalEstimatedBattery(0.0), isValid(false) {}

    ~PickingPlan();
    void clear();
};

double calcLegBattery(int distance, double rate, double carriedWeight, double maxCapacity);

bool buildOrderOrderedStops(
    const OrderNode* order,
    const std::string& startLocationID,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    OrderedStopList& orderedOut
);

bool buildRobotPickingPlan(
    const LocalRobot* robot,
    const OrderNode* order,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    PickingPlan& planOut
);

bool selectBestRobotAndGeneratePlan(
    LocalOrderQueue& orderQueue,
    LocalInventoryTree& inventory,
    LocalRobotFleet& fleet,
    const LocalWarehouseGraph& graph,
    PickingPlan& finalPlanOut,
    OrderNode*& selectedOrderOut
);

void displayPickingPlan(const PickingPlan& plan);

#endif // PICKINGPLAN_H
