#pragma once
#include "ChargingManager.h"

ChargingManager::ChargingManager() : head(nullptr) {}

ChargingManager::~ChargingManager()
{
    StationNode *curr = head;
    while (curr != nullptr)
    {
        StationNode *nextStation = curr->next;
        delete curr;
        curr = nextStation;
    }
}

// ye true hoga if the line contains only digits e.g 2
bool ChargingManager::isNumberLine(const std::string &line) const
{
    bool hasDigit = false;
    for (std::size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];
        if (c >= '0' && c <= '9')
            hasDigit = true;
        else if (c != ' ' && c != '\t' && c != '\r')
            return false;
    }
    return hasDigit;
}

bool ChargingManager::addStation(const std::string &stationID, const std::string &locID, int numSlots, const WarehouseGraph &graph)
{
    if (numSlots <= 0)
    {
        std::cout << "Station must have at least 1 slot." << std::endl;
        return false;
    }

    LocationVertex *loc = graph.searchLocation(locID);
    if (loc == nullptr || loc->type != "CHARGING")
    {
        std::cout << "Invalid charging location: " << locID << std::endl;
        return false;
    }

    if (searchStation(stationID) != nullptr)
    {
        std::cout << "Station already exists: " << stationID << std::endl;
        return false;
    }

    // list k end pr add krna
    StationNode *node = new StationNode(stationID, locID, numSlots);
    if (head == nullptr)
    {
        head = node;
    }
    else
    {
        StationNode *curr = head;
        while (curr->next != nullptr)
            curr = curr->next;
        curr->next = node;
    }
    return true;
}

bool ChargingManager::loadChargers(const std::string &filename, const WarehouseGraph &graph)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Cannot open " << filename << std::endl;
        return false;
    }

    // agr first line sirf count ho to skip kr do
    std::string line;
    std::getline(file, line);
    if (!isNumberLine(line))
    {
        file.clear();
        file.seekg(0);
    }

    std::string sID, locID;
    int numSlots;
    while (file >> sID >> locID >> numSlots)
    {
        addStation(sID, locID, numSlots, graph);
    }

    file.close();
    return true;
}

StationNode *ChargingManager::searchStation(const std::string &stationID) const
{
    StationNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->stationID == stationID)
            return curr;
        curr = curr->next;
    }
    return nullptr;
}

bool ChargingManager::sendRobotToCharge(const std::string &robotID, RobotFleet &fleet, const WarehouseGraph &graph)
{
    Robot *bot = fleet.searchRobot(robotID);
    if (bot == nullptr)
    {
        std::cout << "Robot not found: " << robotID << std::endl;
        return false;
    }

    if (bot->getStatus() != "Available")
    {
        std::cout << "Robot must be Available to charge." << std::endl;
        return false;
    }

    if (bot->getBattery() >= 20.0)
    {
        std::cout << "Robot battery is " << bot->getBattery() << "%, must be below 20%." << std::endl;
        return false;
    }

    // best station jis k paas free slot ho
    StationNode *bestFree = nullptr;
    int bestFreeDist = 1000000000;
    double bestFreeCost = 0.0;

    // best full station (queue k liye)
    StationNode *bestFull = nullptr;
    int bestFullDist = 1000000000;
    double bestFullCost = 0.0;

    StationNode *curr = head;
    while (curr != nullptr)
    {
        RouteResult route;
        if (graph.findShortestRouteDijkstra(bot->getLocation(), curr->locationID, route))
        {
            double cost = bot->calculateLegBattery(route.totalDistance, 0.0);
            if (bot->getBattery() >= cost)
            {
                if (curr->slots.findFreeSlot() != -1)
                {
                    if (bestFree == nullptr || route.totalDistance < bestFreeDist ||
                        (route.totalDistance == bestFreeDist && curr->stationID < bestFree->stationID))
                    {
                        bestFree = curr;
                        bestFreeDist = route.totalDistance;
                        bestFreeCost = cost;
                    }
                }
                else
                {
                    if (bestFull == nullptr || route.totalDistance < bestFullDist ||
                        (route.totalDistance == bestFullDist && curr->stationID < bestFull->stationID))
                    {
                        bestFull = curr;
                        bestFullDist = route.totalDistance;
                        bestFullCost = cost;
                    }
                }
            }
        }
        curr = curr->next;
    }

    if (bestFree != nullptr)
    {
        int slot = bestFree->slots.findFreeSlot();
        bestFree->slots.assignSlot(slot, robotID);
        bot->setBattery(bot->getBattery() - bestFreeCost);
        bot->setLocation(bestFree->locationID);
        bot->setStatus("Charging");

        std::cout << robotID << " charging at " << bestFree->stationID << " (Slot " << (slot + 1) << ")" << std::endl;
        return true;
    }

    if (bestFull != nullptr)
    {
        bestFull->slots.enqueueRobot(robotID);
        bot->setBattery(bot->getBattery() - bestFullCost);
        bot->setLocation(bestFull->locationID);
        bot->setStatus("Waiting for Charging");

        std::cout << robotID << " added to waiting queue at " << bestFull->stationID << std::endl;
        return true;
    }

    // agr koi station reachable nahi hai
    bot->setStatus("Unavailable");
    std::cout << robotID << " cannot reach any charging station. Marked Unavailable." << std::endl;
    return false;
}

bool ChargingManager::completeCharging(const std::string &stationID, const std::string &robotID, RobotFleet &fleet)
{
    StationNode *st = searchStation(stationID);
    if (st == nullptr)
    {
        std::cout << "Station not found: " << stationID << std::endl;
        return false;
    }

    int slotIdx = st->slots.findRobotInSlots(robotID);
    if (slotIdx == -1)
    {
        std::cout << robotID << " is not charging at " << stationID << std::endl;
        return false;
    }

    Robot *bot = fleet.searchRobot(robotID);
    if (bot != nullptr)
    {
        bot->setBattery(100.0);
        bot->setLocation(st->locationID);
        bot->setStatus("Available");
    }

    st->slots.clearSlot(slotIdx);
    std::cout << robotID << " finished charging at " << stationID << " (100%)" << std::endl;

    // queue ma koi robot wait kr raha ho to usay slot de do
    if (!st->slots.isQueueEmpty())
    {
        std::string nextBotID = st->slots.dequeueRobot();
        st->slots.assignSlot(slotIdx, nextBotID);

        Robot *nextBot = fleet.searchRobot(nextBotID);
        if (nextBot != nullptr)
            nextBot->setStatus("Charging");

        std::cout << nextBotID << " moved from queue to slot " << (slotIdx + 1) << " at " << stationID << std::endl;
    }

    return true;
}

bool ChargingManager::removeFromChargingQueue(const std::string &robotID, RobotFleet &fleet)
{
    StationNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->slots.removeFromQueue(robotID))
        {
            Robot *bot = fleet.searchRobot(robotID);
            if (bot != nullptr)
                bot->setStatus("Available");

            std::cout << robotID << " removed from queue at " << curr->stationID << std::endl;
            return true;
        }
        curr = curr->next;
    }

    std::cout << robotID << " not found in any charging queue." << std::endl;
    return false;
}

void ChargingManager::displayChargingStations() const
{
    if (head == nullptr)
    {
        std::cout << "No charging stations." << std::endl;
        return;
    }

    std::cout << "\nCharging Stations:" << std::endl;
    StationNode *curr = head;
    while (curr != nullptr)
    {
        int total = curr->slots.getTotalSlots();
        std::cout << curr->stationID << " (" << curr->locationID << ") | Slots: [";
        for (int i = 0; i < total; i++)
        {
            std::string occ = curr->slots.getSlotOccupant(i);
            if (occ.empty())
                std::cout << "FREE";
            else
                std::cout << occ;
            if (i < total - 1)
                std::cout << ", ";
        }
        std::cout << "] | Queue: ";
        curr->slots.displayQueue();
        std::cout << std::endl;

        curr = curr->next;
    }
}
