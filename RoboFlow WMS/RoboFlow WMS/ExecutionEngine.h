//
//  ExecutionEngine.h
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 26/11/2026.
//

//
//  ExecutionEngine.h
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//

#ifndef EXECUTIONENGINE_H
#define EXECUTIONENGINE_H

#include "PickingPlan.h"

//module 6 order picking and delivery execution

PickingTripNode* getNextUnexecutedTrip(PickingPlan& plan, TripMetaNode*& metaOut);

bool validateTripBeforeExecution(
    const PickingTripNode* trip,
    const TripMetaNode* meta,
    const LocalRobot* robot,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    std::string& failReason
);

bool executeNextTrip(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph
);

bool recalculateTripRoute(
    PickingTripNode* trip,
    TripMetaNode* meta,
    const LocalRobot* robot,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph
);

void cancelAssignment(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    LocalOrderQueue& orderQueue
);

void runExecutionSubMenu(
    PickingPlan& plan,
    OrderNode* order,
    LocalRobotFleet& fleet,
    LocalInventoryTree& inventory,
    LocalWarehouseGraph& graph,
    LocalOrderQueue& orderQueue
);

#endif // EXECUTIONENGINE_H
