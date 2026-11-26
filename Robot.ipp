#pragma once
#include "Robot.h"

Robot::Robot(const std::string &id, const std::string &t, const std::string &loc,double bat, double cap, const std::string &st)
    : robotID(id), type(t), currentLocationID(loc), batteryLevel(bat),maxCapacity(cap), status(st), assignedTaskID("") {}
void Robot::setBattery(double bat)
{
    if (bat < 0.0)
        batteryLevel = 0.0;
    else if (bat > 100.0)
        batteryLevel = 100.0;
    else
        batteryLevel = bat;
}

double Robot::calculateLegBattery(int distance, double carriedWeight) const
{
    if (distance <= 0 || maxCapacity <= 0.0)
        return 0.0;
    return distance * getBatteryRate() * (1.0 + 0.5 * (carriedWeight / maxCapacity));
}

// Picker: rate 0.20 max unit weight 5kg
Picker::Picker(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "Picker", loc, bat, cap, st) {}

double Picker::getBatteryRate() const
{
    return 0.20;
}

bool Picker::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
    return maxUnitWeight <= 5.0;
}

// Carrier: rate 0.25 koi item weight limit nahi
Carrier::Carrier(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "Carrier", loc, bat, cap, st) {}

double Carrier::getBatteryRate() const
{
    return 0.25;
}

bool Carrier::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
    return true;
}

// HeavyDuty: rate 0.35 sirf heavy orders k liye
HeavyDuty::HeavyDuty(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "HeavyDuty", loc, bat, cap, st) {}

double HeavyDuty::getBatteryRate() const
{
    return 0.35;
}

bool HeavyDuty::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
    return (totalOrderWeight > 15.0 || maxUnitWeight > 10.0);
}
