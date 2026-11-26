#pragma once
#include "library.h"

struct StationNode
{
    std::string stationID;
    std::string locationID;
    SlotQueue slots;
    StationNode *next;

    StationNode(const std::string &sID, const std::string &lID, int numSlots)
        : stationID(sID), locationID(lID), slots(numSlots), next(nullptr) {}
};

class ChargingManager
{
private:
    StationNode *head;
    bool isNumberLine(const std::string &line) const;

public:
    ChargingManager();
    ~ChargingManager();

    ChargingManager(const ChargingManager &) = delete;
    ChargingManager &operator=(const ChargingManager &) = delete;

    bool loadChargers(const std::string &filename, const WarehouseGraph &graph);
    bool addStation(const std::string &stationID, const std::string &locID, int numSlots, const WarehouseGraph &graph);
    StationNode *searchStation(const std::string &stationID) const;

    bool sendRobotToCharge(const std::string &robotID, RobotFleet &fleet, const WarehouseGraph &graph);
    bool completeCharging(const std::string &stationID, const std::string &robotID, RobotFleet &fleet);
    bool removeFromChargingQueue(const std::string &robotID, RobotFleet &fleet);

    void displayChargingStations() const;
};

#include "ChargingManager.ipp"
