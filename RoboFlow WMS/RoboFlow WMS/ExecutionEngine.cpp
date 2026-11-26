
//
//  ExecutionEngine.cpp
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//

#include "Library.h"
#include "ExecutionEngine.h"

using std::cout;
using std::cin;
using std::string;

PickingTripNode* getNextUnexecutedTrip(PickingPlan& plan, TripMetaNode*& metaOut){
    PickingTripNode * t = plan.trips.head;
    TripMetaNode * m = plan.metaHead;
    while(t && m){
        if(!m->isExecuted){
            metaOut = m;
            return t;
        }
        t = t->next;
        m = m->next;
    }
    metaOut = nullptr;
    return nullptr;
}

//phase 1 validate first before changing any data
bool validateTripBeforeExecution(
    const PickingTripNode* trip,
    const TripMetaNode* meta,
    const LocalRobot* robot,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    std::string& failReason
){
    if(trip == nullptr || meta == nullptr || robot == nullptr){
        failReason = "Missing trip or robot record.";
        return false;
    }
    if(robot->status != "Assigned"){
        failReason = "Robot is not in Assigned status.";
        return false;
    }
    if(robot->currentLocationID != trip->startLocationID){
        failReason = "Robot is not at trip start location " + trip->startLocationID;
        return false;
    }
    if(meta->tripWeight > robot->maxCapacity){
        failReason = "Trip weight exceeds robot max capacity.";
        return false;
    }
    if(robot->batteryLevel < meta->tripBattery + 10.0){
        failReason = "Insufficient battery for trip + 10% safety reserve.";
        return false;
    }

    //check all legs are still OPEN and stock is intact
    string prevLoc = trip->startLocationID;
    OrderedPickStop * s = trip->stopsHead;
    while(s){
        if(!graph.isRouteStillOpen(prevLoc, s->locationID) || graph.getShortestDistance(prevLoc, s->locationID) < 0){
            failReason = "Pathway to rack " + s->rackID + " (" + s->locationID + ") is blocked or unreachable.";
            return false;
        }
        if(!s->visited){
            ProductNode * prod = inventory.searchProduct(s->prodID);
            if(prod == nullptr){
                failReason = "Product " + s->prodID + " missing from inventory.";
                return false;
            }
            RackEntry * r = prod->rackHead;
            bool foundRack = false;
            while(r){
                if(r->rackID == s->rackID){
                    foundRack = true;
                    if(r->totalQty < s->pickQty || r->reservedQty < s->pickQty){
                        failReason = "Insufficient stock or reservation at rack " + s->rackID;
                        return false;
                    }
                    break;
                }
                r = r->next;
            }
            if(!foundRack){
                failReason = "Rack " + s->rackID + " not found for product " + s->prodID;
                return false;
            }
        }
        prevLoc = s->locationID;
        s = s->next;
    }

    //check final leg to packing station
    if(!graph.isRouteStillOpen(prevLoc, trip->endLocationID) || graph.getShortestDistance(prevLoc, trip->endLocationID) < 0){
        failReason = "Pathway to packing station " + trip->endLocationID + " is blocked or unreachable.";
        return false;
    }

    return true;
}

bool executeNextTrip(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph
){
    if(!plan.isValid || order == nullptr){
        cout << "No valid active picking plan.\n";
        return false;
    }

    TripMetaNode * meta = nullptr;
    PickingTripNode * trip = getNextUnexecutedTrip(plan, meta);
    if(trip == nullptr){
        cout << "All trips in this plan are already executed.\n";
        return false;
    }

    LocalRobot * robot = fleet.searchRobot(plan.assignedRobotID);
    string failReason = "";

    //validate first
    if(!validateTripBeforeExecution(trip, meta, robot, inventory, graph, failReason)){
        cout << "Trip execution aborted: " << failReason << "\n";
        return false;
    }

    //phase 2 now apply changes (never push to undo stack)
    OrderedPickStop * s = trip->stopsHead;
    while(s){
        //visited flag prevents double deduction
        if(!s->visited){
            inventory.commitPickedStock(s->prodID, s->rackID, order->orderID, s->pickQty);
            s->visited = true;

            OrderItemNode * item = order->itemsHead;
            while(item){
                if(item->prodID == s->prodID){
                    item->deliveredQty += s->pickQty;
                    break;
                }
                item = item->next;
            }
        }
        s = s->next;
    }

    //update robot location & battery
    robot->currentLocationID = trip->endLocationID;
    robot->batteryLevel -= meta->tripBattery;
    if(robot->batteryLevel < 0.0) robot->batteryLevel = 0.0;
    meta->isExecuted = true;

    cout << "Trip " << trip->tripNumber << " completed! Delivered to " << trip->endLocationID << ".\n";

    //check if entire order is delivered
    bool orderDone = true;
    OrderItemNode * item = order->itemsHead;
    while(item){
        if(item->deliveredQty < item->requiredQty){
            orderDone = false;
            break;
        }
        item = item->next;
    }

    if(orderDone){
        order->status = "Completed";
    }else{
        order->status = "Partially Fulfilled";
    }

    //release robot after final trip
    TripMetaNode * nextMeta = nullptr;
    if(getNextUnexecutedTrip(plan, nextMeta) == nullptr){
        fleet.releaseRobot(robot->robotID);
        order->assignedRobotID = "";
        cout << "Final trip finished. Robot " << robot->robotID << " released.\n";
        if(robot->batteryLevel < 20.0){
            cout << "ALERT: Robot " << robot->robotID << " battery is below 20% (" << robot->batteryLevel << "%). Needs charging!\n";
        }
    }

    return true;
}

//recalculate route using dijkstra if pathway got blocked
bool recalculateTripRoute(
    PickingTripNode* trip,
    TripMetaNode* meta,
    const LocalRobot* robot,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph
){
    if(trip == nullptr || meta == nullptr || robot == nullptr){
        return false;
    }

    string prevLoc = robot->currentLocationID;
    int newTotalDist = 0;
    double newBattery = 0.0;
    double carriedWeight = 0.0;
    double rate = robot->getBatteryRate();

    OrderedPickStop * s = trip->stopsHead;
    while(s){
        if(!s->visited){
            int d = graph.getShortestDistance(prevLoc, s->locationID);
            if(d < 0){
                cout << "Rack " << s->rackID << " (" << s->locationID << ") is now unreachable! Please Cancel Assignment.\n";
                return false;
            }
            s->legDistance = d;
            newTotalDist += d;
            newBattery += calcLegBattery(d, rate, carriedWeight, robot->maxCapacity);

            ProductNode * prod = inventory.searchProduct(s->prodID);
            if(prod != nullptr){
                carriedWeight += s->pickQty * prod->unitWeight;
            }
            prevLoc = s->locationID;
        }
        s = s->next;
    }

    int packDist = -1;
    string packLoc = graph.getNearestPackingStation(prevLoc, packDist);
    if(packLoc.empty() || packDist < 0){
        cout << "No packing station reachable from " << prevLoc << "! Please Cancel Assignment.\n";
        return false;
    }

    newTotalDist += packDist;
    newBattery += calcLegBattery(packDist, rate, carriedWeight, robot->maxCapacity);

    trip->startLocationID = robot->currentLocationID;
    trip->endLocationID = packLoc;
    trip->totalTripDistance = newTotalDist;
    meta->tripBattery = newBattery;
    meta->tripWeight = carriedWeight;

    cout << "Route recalculated! New distance: " << newTotalDist << " | New est. battery: " << newBattery << "%\n";
    return true;
}

//release uncollected reservations, free robot, return unstarted order to queue front
void cancelAssignment(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    LocalOrderQueue& orderQueue
){
    if(order == nullptr){
        return;
    }

    //only release stops not yet collected
    PickingTripNode * t = plan.trips.head;
    while(t){
        OrderedPickStop * s = t->stopsHead;
        while(s){
            if(!s->visited){
                inventory.releaseSingleStopReservation(s->prodID, s->rackID, order->orderID, s->pickQty);
            }
            s = s->next;
        }
        t = t->next;
    }

    if(!plan.assignedRobotID.empty()){
        fleet.releaseRobot(plan.assignedRobotID);
    }
    order->assignedRobotID = "";

    //check if anything was already delivered
    bool anyDelivered = false;
    OrderItemNode * item = order->itemsHead;
    while(item){
        if(item->deliveredQty > 0){
            anyDelivered = true;
            break;
        }
        item = item->next;
    }

    if(!anyDelivered){
        order->status = "Pending";
        orderQueue.pushFront(order);
        cout << "Assignment cancelled. Order " << order->orderID << " returned to front of Pending queue.\n";
    }else{
        order->status = "Partially Fulfilled";
        cout << "Assignment cancelled. Already delivered items remain delivered.\n";
    }

    plan.clear();
}

void runExecutionSubMenu(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    LocalWarehouseGraph& graph,
    LocalOrderQueue& orderQueue
){
    while(plan.isValid){
        TripMetaNode * meta = nullptr;
        PickingTripNode * nextTrip = getNextUnexecutedTrip(plan, meta);
        if(nextTrip == nullptr){
            break;
        }

        cout << "\n--- EXECUTE PICKING TASK ---\n";
        cout << "Order ID: " << plan.orderID << " | Robot: " << plan.assignedRobotID << "\n";
        cout << "Next Trip: Trip " << nextTrip->tripNumber << " (" << nextTrip->startLocationID
             << " -> " << nextTrip->endLocationID << ") | Dist: " << nextTrip->totalTripDistance
             << " | Weight: " << meta->tripWeight << " kg | Est Battery: " << meta->tripBattery << "%\n";
        cout << "1. Execute Trip\n";
        cout << "2. Recalculate Route\n";
        cout << "3. Cancel Assignment\n";
        cout << "4. Assign but not execute now\n";
        cout << "Choice: ";

        int choice;
        if(!(cin >> choice)){
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input.\n";
            continue;
        }

        if(choice == 1){
            executeNextTrip(plan, order, fleet, inventory, graph);
        }else if(choice == 2){
            LocalRobot * r = fleet.searchRobot(plan.assignedRobotID);
            recalculateTripRoute(nextTrip, meta, r, inventory, graph);
        }else if(choice == 3){
            cancelAssignment(plan, order, fleet, inventory, orderQueue);
            break;
        }else if(choice == 4){
            cout << "Plan saved. You can resume execution from Module 6 menu later.\n";
            break;
        }else{
            cout << "Invalid option.\n";
        }
    }
}
