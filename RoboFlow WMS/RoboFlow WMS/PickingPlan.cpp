//
//  PickingPlan.cpp
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//
#include "Library.h"
#include "PickingPlan.h"

using std::cout;
using  std::cin;
using std::string;

AllocationList::~AllocationList() {
    clearAllocationList(*this);
}
void clearAllocationList(AllocationList& list) {
    AllocationNode* current = list.head;
    while (current != nullptr) {
        AllocationNode* nodeToDelete = current;
        current = current->next;
        delete nodeToDelete;
    }

    list.head = nullptr;
    list.tail = nullptr;
    list.totalAllocated = 0;
    list.isFullyAllocated = false;
}

CandidateList::~CandidateList() {
    clearCandidateList(head);
    tail = nullptr;
}

void clearCandidateList(CandidateRack*& head) {
    while (head != nullptr) {
        CandidateRack* nodeToDelete = head;
        head = head->next;
        delete nodeToDelete;
    }
    head = nullptr;
}
CandidateRack * bestcandidate(CandidateRack * r1, CandidateRack * r2){
    if (r1 == nullptr) return r2;
    if (r2 == nullptr) return r1;
    if(r1->distance < r2->distance){
        return r1;
    }
    if(r2->distance < r1->distance){
        return r2;
    }
    if(r1->availableQty > r2->availableQty){
        return r1;
    }
    if(r2->availableQty > r1->availableQty){
        return r2;
    }
    if(r1->rackID < r2->rackID){
        return r1;
    }
    return r2;
}

bool allocateRacksForProduct(
    const std::string& prodID,
    int reqQty,
    const std::string& refLocationID,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    AllocationList& outList
                             ){
    clearAllocationList(outList);
    if(reqQty <= 0 || refLocationID.empty()){
        return false;
    }
    ProductNode* temp = inventory.searchProduct(prodID);
    if (temp == nullptr) {
        return false;
    }
    RackEntry* temprack = temp->rackHead;
    
    CandidateList candidatelist;
   
    while(temprack){
        int availableQty = temprack->totalQty - temprack->reservedQty;
            
            string rackLocationID = graph.getRackLocation(temprack->rackID);
            int shortestdistance = graph.getShortestDistance(refLocationID , rackLocationID);
        
            //if the rack is unreachable
            if(shortestdistance == -1){
                temprack = temprack->next;
                continue;
            }
            
            //check the available qty
        if(availableQty <= 0){
            temprack = temprack->next;
            continue;
        }
        CandidateRack * candidaterack = nullptr;
        //check tha available qty is qualified
        candidaterack = new CandidateRack(temprack->rackID, rackLocationID, availableQty, shortestdistance);
        if(candidatelist.head == nullptr){
            candidatelist.head = candidaterack;
            candidatelist.tail = candidaterack;
            candidatelist.tail->next = nullptr;
        }else{
            candidatelist.tail->next = candidaterack;
            candidatelist.tail = candidaterack;
            candidatelist.tail->next = nullptr;
        }
        temprack = temprack->next;
    }
    //now candidatelist has full
    CandidateRack * tempcandidaterack = candidatelist.head;
    if (tempcandidaterack == nullptr) {
        outList.isFullyAllocated = false;
        return false;
    }
    AllocationList allocatedlist;
    AllocationNode * tempallocatednode = nullptr;
    int remainingQty = reqQty;
    int totalQty = reqQty;
    
    while(true){
        if(remainingQty == 0){
            break;
        }
        if(tempcandidaterack->used){
            if (tempcandidaterack->next != nullptr) {
                tempcandidaterack = tempcandidaterack->next;
                continue;
            }
            break;
        }
        
        //if a single rack can fulfill the reqQty
        CandidateRack* bestSingleRack = nullptr;
        for (CandidateRack* it = candidatelist.head ; it != nullptr; it = it->next) {
            if(!it->used && it->availableQty >= remainingQty){
                bestSingleRack = bestcandidate(bestSingleRack, it);
            }
        }
        if (bestSingleRack != nullptr) {
            tempallocatednode = new AllocationNode(bestSingleRack->rackID, bestSingleRack->locationID, remainingQty, bestSingleRack->distance);
            if(allocatedlist.head == nullptr){
                allocatedlist.head = tempallocatednode;
                allocatedlist.tail = tempallocatednode;
                allocatedlist.tail->next = nullptr;
            }else{
                allocatedlist.tail->next = tempallocatednode;
                allocatedlist.tail = tempallocatednode;
                allocatedlist.tail->next = nullptr;
            }
            bestSingleRack->used = true;
            remainingQty = 0;
            break;
        }
        
        //check best rack
        CandidateRack * bestrack = nullptr;
        CandidateRack * checkrack = candidatelist.head;
        while(checkrack){
            if(!checkrack->used){
                bestrack = bestcandidate(bestrack, checkrack);
            }
            checkrack = checkrack->next;
        }
        if (bestrack == nullptr) {
            break;
        }
        int takeQty = (bestrack->availableQty < remainingQty) ? bestrack->availableQty : remainingQty;
        tempallocatednode = new AllocationNode(bestrack->rackID, bestrack->locationID, takeQty, bestrack->distance);
        remainingQty -= takeQty;
        bestrack->used = true;
        if(allocatedlist.head == nullptr){
            allocatedlist.head = tempallocatednode;
            allocatedlist.tail = tempallocatednode;
            allocatedlist.tail->next = nullptr;
        }else{
            allocatedlist.tail->next = tempallocatednode;
            allocatedlist.tail = tempallocatednode;
            allocatedlist.tail->next = nullptr;
        }
    }
    outList.head = allocatedlist.head;
    outList.tail = allocatedlist.tail;
    outList.totalAllocated = totalQty - remainingQty;
    outList.isFullyAllocated = (remainingQty == 0);
    allocatedlist.head = nullptr;
    allocatedlist.tail = nullptr;
    clearCandidateList(candidatelist.head);
    return outList.isFullyAllocated;
}


//step 2 record order picking stops

OrderedStopList::~OrderedStopList() {
    clearOrderedStopList(*this);
}

void clearOrderedStopList(OrderedStopList& list){
    OrderedPickStop * temp = list.head;
    while(temp){
        OrderedPickStop * tempdlr = temp;
        temp = temp->next;
        delete tempdlr;
    }
    list.head = nullptr;
    list.tail = nullptr;
    list.stopCount = 0;
    list.totalUnits = 0;
}

void appendOrderedStop(OrderedStopList& list, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int dist){
    OrderedPickStop * neworderpickstop = new OrderedPickStop(prodID , rID , lID , qty , dist);
    if(list.head == nullptr){
        list.head = neworderpickstop;
        list.tail = neworderpickstop;
        list.tail->next = nullptr;
    }else{
        list.tail->next = neworderpickstop;
        list.tail = neworderpickstop;
        list.tail->next = nullptr;
    }
}

bool removeAllocationByRack(AllocationList& list, const std::string& targetRackID) {
    if (list.head == nullptr) {
        return false;
    }

    AllocationNode* prev = nullptr;
    AllocationNode* curr = list.head;

    while (curr != nullptr) {
        if (curr->rackID == targetRackID) {
            if (prev == nullptr) {
                list.head = curr->next;
            } else {
                prev->next = curr->next;
            }

            if (curr == list.tail) {
                list.tail = prev;
            }

            list.totalAllocated -= curr->allocatedQty;
            delete curr;
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

AllocationNode * bestrack(const std::string& currentloc , AllocationNode * n1 , AllocationNode * n2 , LocalWarehouseGraph & graph){
    if (n1 == nullptr && n2 == nullptr) return nullptr;
    if (n1 == nullptr) return n2;
    if (n2 == nullptr) return n1;

    int d1 = graph.getShortestDistance(currentloc, n1->locationID);
    int d2 = graph.getShortestDistance(currentloc, n2->locationID);

    if (d1 == -1 && d2 == -1) return nullptr;
    if (d1 == -1) return n2;
    if (d2 == -1) return n1;

    if (d1 < d2) return n1;
    if (d2 < d1) return n2;

    // Tie-breaker by rackID for determinism
    if (n1->rackID < n2->rackID) return n1;
    if (n2->rackID < n1->rackID) return n2;
    return n1;
}

bool orderStopsByNearestReachable(
    const AllocationList& allocations,
    const std::string& prodID,
    const std::string& startLocationID,
    const LocalWarehouseGraph& graph,
    OrderedStopList& orderedOut
                                  ){
    clearOrderedStopList(orderedOut);
    if(allocations.head == nullptr){
        return false;
    }
    if(startLocationID.empty()){
        return false;
    }
    std::string currentLocation = startLocationID;

    AllocationList working;
    working.head = nullptr;
    working.tail = nullptr;
    working.totalAllocated = allocations.totalAllocated;
    working.isFullyAllocated = allocations.isFullyAllocated;

    for (AllocationNode* it = allocations.head; it != nullptr; it = it->next) {
        AllocationNode* copy = new AllocationNode(it->rackID, it->locationID, it->allocatedQty, it->distanceFromRef);
        if (working.head == nullptr) {
            working.head = copy;
            working.tail = copy;
            working.tail->next = nullptr;
        } else {
            working.tail->next = copy;
            working.tail = copy;
            working.tail->next = nullptr;
        }
    }

    AllocationNode * bestStop = nullptr;
    while(true){
        bestStop = nullptr;
        AllocationNode * rackstop = working.head;
        while (rackstop) {
            bestStop = bestrack(currentLocation, bestStop, rackstop, const_cast<LocalWarehouseGraph&>(graph));
            rackstop = rackstop->next;
        }
        if (bestStop == nullptr) {
            break;
        }

        int dist = graph.getShortestDistance(currentLocation, bestStop->locationID);
        appendOrderedStop(orderedOut, prodID, bestStop->rackID, bestStop->locationID, bestStop->allocatedQty, dist);

        orderedOut.stopCount += 1;
        orderedOut.totalUnits += bestStop->allocatedQty;
        currentLocation = bestStop->locationID;

        removeAllocationByRack(working, bestStop->rackID);

        if(working.head == nullptr){
            break;
        }else{
            continue;
        }
    }

    return orderedOut.stopCount > 0;
}

//step 3 split stops into trips

PickingTripList::~PickingTripList(){
    clearPickingTripList(*this);
}

void clearPickingTripList(PickingTripList& tripList){
    PickingTripNode * temp = tripList.head;
    while(temp){
        OrderedPickStop * currstop = temp->stopsHead;
        while(currstop){
            OrderedPickStop * stopdlr = currstop;
            currstop = currstop->next;
            delete stopdlr;
        }
        PickingTripNode * tempdlr = temp;
        temp = temp->next;
        delete tempdlr;
    }
    tripList.head = nullptr;
    tripList.tail = nullptr;
    tripList.totalPlannedUnits = 0;
    tripList.tripCount = 0;
}

void appendStopToTrip(PickingTripNode* trip, const std::string& prodID, const std::string& rID, const std::string& lID, int qty, int legDist){
    if(trip == nullptr){
        return;
    }
    OrderedPickStop * newStop = new OrderedPickStop(prodID, rID, lID, qty, legDist);
    if(trip->stopsHead == nullptr){
        trip->stopsHead = newStop;
        trip->stopsTail = newStop;
        trip->stopsTail->next = nullptr;
    }else{
        trip->stopsTail->next = newStop;
        trip->stopsTail = newStop;
        trip->stopsTail->next = nullptr;
    }
    trip->tripUnits += qty;
    trip->totalTripDistance += legDist;
}

void appendTripNode(PickingTripList& tripList, const std::string& startLoc, const std::string& endLoc){
    int nextTripNum = tripList.tripCount + 1;
    PickingTripNode * newTrip = new PickingTripNode(nextTripNum, startLoc, endLoc);
    if(tripList.head == nullptr){
        tripList.head = newTrip;
        tripList.tail = newTrip;
        tripList.tail->next = nullptr;
    }else{
        tripList.tail->next = newTrip;
        tripList.tail = newTrip;
        tripList.tail->next = nullptr;
    }
    tripList.tripCount = nextTripNum;
}

bool splitStopsIntoTrips(
    const OrderedStopList& orderedStops,
    int robotCapacity,
    const std::string& packingStationID,
    const LocalWarehouseGraph& graph,
    PickingTripList& tripsOut
                         ){
    clearPickingTripList(tripsOut);
    if(orderedStops.head == nullptr || robotCapacity <= 0 || packingStationID.empty()){
        return false;
    }
    appendTripNode(tripsOut, packingStationID, packingStationID);
    PickingTripNode * currentTrip = tripsOut.tail;
    string currentLoc = packingStationID;
    int remainingCap = robotCapacity;

    OrderedPickStop * currStop = orderedStops.head;
    while(currStop){
        if(currStop->pickQty <= 0){
            currStop = currStop->next;
            continue;
        }
        int unitsLeft = currStop->pickQty;
        while(unitsLeft > 0){
            //trip is full so return to packing station
            if(remainingCap == 0){
                int returnDist = graph.getShortestDistance(currentLoc, packingStationID);
                if(returnDist == -1){
                    clearPickingTripList(tripsOut);
                    return false;
                }
                currentTrip->totalTripDistance += returnDist;
                appendTripNode(tripsOut, packingStationID, packingStationID);
                currentTrip = tripsOut.tail;
                currentLoc = packingStationID;
                remainingCap = robotCapacity;
            }
            int takeQty = (unitsLeft < remainingCap) ? unitsLeft : remainingCap;
            int legDist = graph.getShortestDistance(currentLoc, currStop->locationID);
            if(legDist == -1){
                clearPickingTripList(tripsOut);
                return false;
            }
            appendStopToTrip(currentTrip, currStop->prodID, currStop->rackID, currStop->locationID, takeQty, legDist);
            tripsOut.totalPlannedUnits += takeQty;
            remainingCap -= takeQty;
            unitsLeft -= takeQty;
            currentLoc = currStop->locationID;
        }
        currStop = currStop->next;
    }
    //last trip return to packing station
    int finalReturnDist = graph.getShortestDistance(currentLoc, packingStationID);
    if(finalReturnDist == -1){
        clearPickingTripList(tripsOut);
        return false;
    }
    currentTrip->totalTripDistance += finalReturnDist;
    return tripsOut.tripCount > 0;
}

//step 4 full order plan & robot selection

PickingPlan::~PickingPlan(){
    clear();
}

void PickingPlan::clear(){
    clearPickingTripList(trips);
    TripMetaNode * temp = metaHead;
    while(temp){
        TripMetaNode * dlr = temp;
        temp = temp->next;
        delete dlr;
    }
    metaHead = nullptr;
    metaTail = nullptr;
    orderID = "";
    assignedRobotID = "";
    totalPlanDistance = 0;
    totalEstimatedBattery = 0.0;
    isValid = false;
}

//formula: distance * rate * (1 + 0.5 * carriedWeight / maxCapacity)
double calcLegBattery(int distance, double rate, double carriedWeight, double maxCapacity){
    if(distance <= 0 || maxCapacity <= 0.0){
        return 0.0;
    }
    return distance * rate * (1.0 + 0.5 * (carriedWeight / maxCapacity));
}

//collect all product allocations for an order and sort by nearest rack
bool buildOrderOrderedStops(
    const OrderNode* order,
    const std::string& startLocationID,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    OrderedStopList& orderedOut
){
    clearOrderedStopList(orderedOut);
    if(order == nullptr || order->itemsHead == nullptr || startLocationID.empty()){
        return false;
    }

    //pool of unsorted stops across all products in the order
    OrderedStopList pool;
    OrderItemNode * item = order->itemsHead;
    while(item){
        int needed = item->requiredQty - item->deliveredQty;
        if(needed > 0){
            AllocationList itemAlloc;
            if(!allocateRacksForProduct(item->prodID, needed, startLocationID, inventory, graph, itemAlloc)){
                clearOrderedStopList(pool);
                return false;
            }
            AllocationNode * aNode = itemAlloc.head;
            while(aNode){
                appendOrderedStop(pool, item->prodID, aNode->rackID, aNode->locationID, aNode->allocatedQty, aNode->distanceFromRef);
                pool.stopCount += 1;
                pool.totalUnits += aNode->allocatedQty;
                aNode = aNode->next;
            }
        }
        item = item->next;
    }

    if(pool.head == nullptr){
        return false;
    }

    //greedy nearest rack ordering across the whole order
    string currentLoc = startLocationID;
    while(pool.head != nullptr){
        OrderedPickStop * prevBest = nullptr;
        OrderedPickStop * bestStop = nullptr;
        int bestDist = -1;

        OrderedPickStop * prev = nullptr;
        OrderedPickStop * curr = pool.head;
        while(curr){
            int d = graph.getShortestDistance(currentLoc, curr->locationID);
            if(d >= 0){
                bool better = false;
                if(bestStop == nullptr || d < bestDist){
                    better = true;
                }else if(d == bestDist && curr->rackID < bestStop->rackID){
                    better = true;
                }
                if(better){
                    bestStop = curr;
                    prevBest = prev;
                    bestDist = d;
                }
            }
            prev = curr;
            curr = curr->next;
        }

        if(bestStop == nullptr){
            clearOrderedStopList(pool);
            clearOrderedStopList(orderedOut);
            return false;
        }

        appendOrderedStop(orderedOut, bestStop->prodID, bestStop->rackID, bestStop->locationID, bestStop->pickQty, bestDist);
        orderedOut.stopCount += 1;
        orderedOut.totalUnits += bestStop->pickQty;
        currentLoc = bestStop->locationID;

        //unlink bestStop from pool
        if(prevBest == nullptr){
            pool.head = bestStop->next;
        }else{
            prevBest->next = bestStop->next;
        }
        if(bestStop == pool.tail){
            pool.tail = prevBest;
        }
        delete bestStop;
    }

    return orderedOut.stopCount > 0;
}

//build multi-trip plan with kg capacity and battery calculation for one robot
bool buildRobotPickingPlan(
    const LocalRobot* robot,
    const OrderNode* order,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    PickingPlan& planOut
){
    planOut.clear();
    if(robot == nullptr || order == nullptr){
        return false;
    }

    OrderedStopList orderedStops;
    if(!buildOrderOrderedStops(order, robot->currentLocationID, inventory, graph, orderedStops)){
        return false;
    }

    planOut.orderID = order->orderID;
    planOut.assignedRobotID = robot->robotID;

    //trip 1 starts at robot current location
    appendTripNode(planOut.trips, robot->currentLocationID, "");
    PickingTripNode * currentTrip = planOut.trips.tail;
    string currentLoc = robot->currentLocationID;
    double carriedWeight = 0.0;
    double tripBattery = 0.0;
    double rate = robot->getBatteryRate();

    OrderedPickStop * stop = orderedStops.head;
    while(stop){
        ProductNode * prod = inventory.searchProduct(stop->prodID);
        if(prod == nullptr || prod->unitWeight <= 0.0 || prod->unitWeight > robot->maxCapacity){
            planOut.clear();
            return false;
        }

        int unitsLeft = stop->pickQty;
        while(unitsLeft > 0){
            double freeCap = robot->maxCapacity - carriedWeight;
            int canFit = (int)(freeCap / prod->unitWeight);

            //if even 1 unit cant fit go unload at nearest packing station
            if(canFit <= 0){
                int packDist = -1;
                string packLoc = graph.getNearestPackingStation(currentLoc, packDist);
                if(packLoc.empty() || packDist < 0){
                    planOut.clear();
                    return false;
                }
                currentTrip->endLocationID = packLoc;
                currentTrip->totalTripDistance += packDist;
                tripBattery += calcLegBattery(packDist, rate, carriedWeight, robot->maxCapacity);

                //record trip meta
                TripMetaNode * meta = new TripMetaNode(currentTrip->tripNumber, carriedWeight, tripBattery);
                if(planOut.metaHead == nullptr){
                    planOut.metaHead = meta;
                    planOut.metaTail = meta;
                }else{
                    planOut.metaTail->next = meta;
                    planOut.metaTail = meta;
                }

                //start next trip from packing station
                appendTripNode(planOut.trips, packLoc, "");
                currentTrip = planOut.trips.tail;
                currentLoc = packLoc;
                carriedWeight = 0.0;
                tripBattery = 0.0;
                continue;
            }

            int takeQty = (unitsLeft < canFit) ? unitsLeft : canFit;
            int legDist = graph.getShortestDistance(currentLoc, stop->locationID);
            if(legDist < 0){
                planOut.clear();
                return false;
            }

            //battery before loading new units at this stop
            tripBattery += calcLegBattery(legDist, rate, carriedWeight, robot->maxCapacity);
            appendStopToTrip(currentTrip, stop->prodID, stop->rackID, stop->locationID, takeQty, legDist);

            planOut.trips.totalPlannedUnits += takeQty;
            carriedWeight += takeQty * prod->unitWeight;
            unitsLeft -= takeQty;
            currentLoc = stop->locationID;
        }
        stop = stop->next;
    }

    //route final trip to nearest packing station
    int finalPackDist = -1;
    string finalPackLoc = graph.getNearestPackingStation(currentLoc, finalPackDist);
    if(finalPackLoc.empty() || finalPackDist < 0){
        planOut.clear();
        return false;
    }
    currentTrip->endLocationID = finalPackLoc;
    currentTrip->totalTripDistance += finalPackDist;
    tripBattery += calcLegBattery(finalPackDist, rate, carriedWeight, robot->maxCapacity);

    TripMetaNode * lastMeta = new TripMetaNode(currentTrip->tripNumber, carriedWeight, tripBattery);
    if(planOut.metaHead == nullptr){
        planOut.metaHead = lastMeta;
        planOut.metaTail = lastMeta;
    }else{
        planOut.metaTail->next = lastMeta;
        planOut.metaTail = lastMeta;
    }

    //sum totals across all trips
    PickingTripNode * t = planOut.trips.head;
    TripMetaNode * m = planOut.metaHead;
    while(t && m){
        planOut.totalPlanDistance += t->totalTripDistance;
        planOut.totalEstimatedBattery += m->tripBattery;
        t = t->next;
        m = m->next;
    }

    planOut.isValid = true;
    return true;
}

void displayPickingPlan(const PickingPlan& plan){
    cout << "\n========================================\n";
    cout << "PICKING PLAN FOR ORDER: " << plan.orderID << "\n";
    cout << "Assigned Robot: " << plan.assignedRobotID << "\n";
    cout << "Total Trips: " << plan.trips.tripCount << "\n";
    cout << "Total Distance: " << plan.totalPlanDistance << " units\n";
    cout << "Estimated Battery: " << plan.totalEstimatedBattery << "%\n";
    cout << "========================================\n";

    PickingTripNode * t = plan.trips.head;
    TripMetaNode * m = plan.metaHead;
    while(t && m){
        cout << "Trip " << t->tripNumber << " (" << t->startLocationID << " -> " << t->endLocationID << "):\n";
        OrderedPickStop * s = t->stopsHead;
        while(s){
            cout << "  - Rack " << s->rackID << " (" << s->locationID << "): Collect "
                 << s->prodID << " x " << s->pickQty << " (leg dist: " << s->legDistance << ")\n";
            s = s->next;
        }
        cout << "  Trip Weight: " << m->tripWeight << " kg | Trip Dist: " << t->totalTripDistance
             << " | Est Battery: " << m->tripBattery << "%\n";
        t = t->next;
        m = m->next;
    }
}

//robot selection rule + all-or-nothing reservation
bool selectBestRobotAndGeneratePlan(
    LocalOrderQueue& orderQueue,
    LocalInventoryTree& inventory,
    LocalRobotFleet& fleet,
    const LocalWarehouseGraph& graph,
    PickingPlan& finalPlanOut,
    OrderNode*& selectedOrderOut
){
    finalPlanOut.clear();
    selectedOrderOut = nullptr;

    OrderNode * currOrder = orderQueue.peekFront();
    if(currOrder == nullptr){
        cout << "No pending orders in queue.\n";
        return false;
    }

    while(currOrder != nullptr){
        //check total order weight and max single item weight
        double totalWeight = 0.0;
        double maxUnitWeight = 0.0;
        bool productMissing = false;

        OrderItemNode * item = currOrder->itemsHead;
        while(item){
            ProductNode * prod = inventory.searchProduct(item->prodID);
            if(prod == nullptr){
                productMissing = true;
                break;
            }
            int needed = item->requiredQty - item->deliveredQty;
            totalWeight += needed * prod->unitWeight;
            if(prod->unitWeight > maxUnitWeight){
                maxUnitWeight = prod->unitWeight;
            }
            item = item->next;
        }

        if(productMissing){
            cout << "Order " << currOrder->orderID << " skipped: product not found in inventory.\n";
            currOrder = orderQueue.getNextPending(currOrder);
            continue;
        }

        //evaluate all available & eligible robots
        LocalRobot * bestRobot = nullptr;
        PickingPlan bestPlan;
        bool hadBatteryIssue = false;
        bool hadEligibleRobot = false;

        LocalRobot * r = fleet.head;
        while(r != nullptr){
            if(r->status == "Available" && r->isEligibleForOrder(totalWeight, maxUnitWeight) && maxUnitWeight <= r->maxCapacity){
                hadEligibleRobot = true;
                PickingPlan tempPlan;
                if(buildRobotPickingPlan(r, currOrder, inventory, graph, tempPlan)){
                    //10% safety reserve check
                    if(r->batteryLevel >= tempPlan.totalEstimatedBattery + 10.0){
                        bool isBetter = false;
                        if(bestRobot == nullptr){
                            isBetter = true;
                        }else if(tempPlan.trips.tripCount < bestPlan.trips.tripCount){
                            isBetter = true;
                        }else if(tempPlan.trips.tripCount == bestPlan.trips.tripCount && tempPlan.totalPlanDistance < bestPlan.totalPlanDistance){
                            isBetter = true;
                        }else if(tempPlan.trips.tripCount == bestPlan.trips.tripCount && tempPlan.totalPlanDistance == bestPlan.totalPlanDistance && r->robotID < bestRobot->robotID){
                            isBetter = true;
                        }

                        if(isBetter){
                            bestRobot = r;
                            //move plan contents into bestPlan
                            bestPlan.clear();
                            bestPlan.orderID = tempPlan.orderID;
                            bestPlan.assignedRobotID = tempPlan.assignedRobotID;
                            bestPlan.trips.head = tempPlan.trips.head;
                            bestPlan.trips.tail = tempPlan.trips.tail;
                            bestPlan.trips.tripCount = tempPlan.trips.tripCount;
                            bestPlan.trips.totalPlannedUnits = tempPlan.trips.totalPlannedUnits;
                            bestPlan.metaHead = tempPlan.metaHead;
                            bestPlan.metaTail = tempPlan.metaTail;
                            bestPlan.totalPlanDistance = tempPlan.totalPlanDistance;
                            bestPlan.totalEstimatedBattery = tempPlan.totalEstimatedBattery;
                            bestPlan.isValid = true;

                            tempPlan.trips.head = nullptr;
                            tempPlan.trips.tail = nullptr;
                            tempPlan.metaHead = nullptr;
                            tempPlan.metaTail = nullptr;
                        }
                    }else{
                        hadBatteryIssue = true;
                    }
                }
            }
            r = r->next;
        }

        if(bestRobot == nullptr){
            cout << "Order " << currOrder->orderID << " cannot be planned right now: ";
            if(!hadEligibleRobot){
                cout << "stock shortage, capacity limit, or no eligible available robot.\n";
            }else if(hadBatteryIssue){
                cout << "eligible robots have insufficient battery (send robots to charging station).\n";
            }else{
                cout << "stock shortage or unreachable racks.\n";
            }
            //skipped order keeps its position in queue
            currOrder = orderQueue.getNextPending(currOrder);
            continue;
        }

        displayPickingPlan(bestPlan);
        cout << "Confirm and reserve inventory for Order " << currOrder->orderID << "? (y/n): ";
        char confirm;
        cin >> confirm;
        if(confirm != 'y' && confirm != 'Y'){
            cout << "Plan cancelled by user.\n";
            return false;
        }

        //all-or-nothing reservation
        bool allReserved = true;
        PickingTripNode * t = bestPlan.trips.head;
        while(t && allReserved){
            OrderedPickStop * s = t->stopsHead;
            while(s){
                if(!inventory.reserveStock(s->prodID, s->rackID, currOrder->orderID, s->pickQty)){
                    allReserved = false;
                    break;
                }
                s = s->next;
            }
            t = t->next;
        }

        if(!allReserved){
            inventory.releaseOrderReservations(currOrder->orderID);
            cout << "Reservation failed (all-or-nothing rollback). Order left Pending.\n";
            return false;
        }

        //update statuses & remove from pending queue
        currOrder->status = "Assigned";
        currOrder->assignedRobotID = bestRobot->robotID;
        orderQueue.removeOrder(currOrder->orderID);
        fleet.assignRobot(bestRobot->robotID, currOrder->orderID);

        //transfer bestPlan to finalPlanOut
        finalPlanOut.clear();
        finalPlanOut.orderID = bestPlan.orderID;
        finalPlanOut.assignedRobotID = bestPlan.assignedRobotID;
        finalPlanOut.trips.head = bestPlan.trips.head;
        finalPlanOut.trips.tail = bestPlan.trips.tail;
        finalPlanOut.trips.tripCount = bestPlan.trips.tripCount;
        finalPlanOut.trips.totalPlannedUnits = bestPlan.trips.totalPlannedUnits;
        finalPlanOut.metaHead = bestPlan.metaHead;
        finalPlanOut.metaTail = bestPlan.metaTail;
        finalPlanOut.totalPlanDistance = bestPlan.totalPlanDistance;
        finalPlanOut.totalEstimatedBattery = bestPlan.totalEstimatedBattery;
        finalPlanOut.isValid = true;

        bestPlan.trips.head = nullptr;
        bestPlan.trips.tail = nullptr;
        bestPlan.metaHead = nullptr;
        bestPlan.metaTail = nullptr;

        selectedOrderOut = currOrder;
        return true;
    }

    return false;
}
