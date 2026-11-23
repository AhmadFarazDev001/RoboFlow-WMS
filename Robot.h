#pragma once
#include "library.h"
class Robot
{
protected:
    std::string robotID;
    std::string type;
    std::string currentLocationID;
    double batteryLevel;
    double maxCapacity;
    std::string status; //can use fr available, assigned, waiting for Charging, Charging, unavailable
    std::string assignedTaskID;

public:
    Robot(const std::string &id, const std::string &t, const std::string &loc,
          double bat, double cap, const std::string &st = "Available");
    virtual ~Robot() {}

    virtual double getBatteryRate() const = 0;
    virtual bool isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const = 0;

    // Formula used  distance * rate * (1 + 0.5 * carriedWeight / maxCapacity)
    double calculateLegBattery(int distance, double carriedWeight) const;

    std::string getID() const { return robotID; }
    std::string getType() const { return type; }
    std::string getLocation() const { return currentLocationID; }
    double getBattery() const { return batteryLevel; }
    double getMaxCapacity() const { return maxCapacity; }
    std::string getStatus() const { return status; }
    std::string getAssignedTask() const { return assignedTaskID; }

    void setLocation(const std::string &loc) { currentLocationID = loc; }
    void setBattery(double bat) { batteryLevel = (bat < 0.0) ? 0.0 : (bat > 100.0 ? 100.0 : bat); }
    void setStatus(const std::string &st) { status = st; }
    void setAssignedTask(const std::string &taskID) { assignedTaskID = taskID; }
};

class Picker : public Robot
{
public:
    Picker(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st = "Available");
    double getBatteryRate() const override;
    bool isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const override;
};

class Carrier : public Robot
{
public:
    Carrier(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st = "Available");
    double getBatteryRate() const override;
    bool isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const override;
};

class HeavyDuty : public Robot
{
public:
    HeavyDuty(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st = "Available");
    double getBatteryRate() const override;
    bool isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const override;
};

#include "Robot.ipp"