#pragma once
#include "Robot.h"

// ~~~ Base Robot ~~~

inline Robot::Robot(const std::string &id, const std::string &t, const std::string &loc,double bat, double cap, const std::string &st)
    : robotID(id), type(t), currentLocationID(loc), batteryLevel(bat),maxCapacity(cap), status(st), assignedTaskID("") {}

inline double Robot::calculateLegBattery(int distance, double carriedWeight) const
{
    if (distance <= 0 || maxCapacity <= 0.0)
        return 0.0;
    return distance * getBatteryRate() * (1.0 + 0.5 * (carriedWeight / maxCapacity));
}

// --- picker ---

inline Picker::Picker(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "Picker", loc, bat, cap, st) {}

inline double Picker::getBatteryRate() const
{
    return 0.20;
}

inline bool Picker::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
    return maxUnitWeight <= 5.0;
}

// --- carrier ---

inline Carrier::Carrier(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "Carrier", loc, bat, cap, st) {}

inline double Carrier::getBatteryRate() const
{
    return 0.25;
}

inline bool Carrier::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
       return true;
}

// --- heavyDuty ---

inline HeavyDuty::HeavyDuty(const std::string &id, const std::string &loc, double bat, double cap, const std::string &st)
    : Robot(id, "HeavyDuty", loc, bat, cap, st) {}

inline double HeavyDuty::getBatteryRate() const
{
    return 0.35;
}

inline bool HeavyDuty::isEligibleForOrder(double totalOrderWeight, double maxUnitWeight) const
{
    return (totalOrderWeight > 15.0 || maxUnitWeight > 10.0);
}