#pragma once
#include "library.h"

struct RobotNode
{
    Robot *data;
    RobotNode *next;

    RobotNode(Robot *r) : data(r), next(nullptr) {}
};

class RobotFleet
{
private:
    RobotNode *head;
    bool isNumberLine(const std::string &line) const;
    std::string trimText(const std::string &text) const;

public:
    RobotFleet();
    ~RobotFleet();

    RobotFleet(const RobotFleet &) = delete;
    RobotFleet &operator=(const RobotFleet &) = delete;

    bool loadRobots(const std::string &filename, const WarehouseGraph *graph = nullptr);
    bool registerRobot(const std::string &id, const std::string &type, const std::string &locID,
                       double battery, double maxCap, const WarehouseGraph *graph = nullptr,
                       const std::string &status = "Available");

    Robot *searchRobot(const std::string &robotID) const;
    bool assignRobot(const std::string &robotID, const std::string &orderID);
    bool releaseRobot(const std::string &robotID);
    bool setMaintenanceStatus(const std::string &robotID, bool toMaintenance, std::string &outOldStatus);

    double calculateLegBattery(const std::string &robotID, int distance, double carriedWeight) const;
    void displayAllRobots() const;
    RobotNode *getHead() const { return head; }
};

#include "RobotFleet.ipp"
