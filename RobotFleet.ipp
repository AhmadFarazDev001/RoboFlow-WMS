#pragma once
#include "RobotFleet.h"

RobotFleet::RobotFleet() : head(nullptr) {}

RobotFleet::~RobotFleet()
{
    RobotNode *curr = head;
    while (curr != nullptr)
    {
        RobotNode *nextNode = curr->next;
        delete curr->data;
        delete curr;
        curr = nextNode;
    }
}

// ye true hoga agr line ma sirf digits hun
bool RobotFleet::isNumberLine(const std::string &line) const
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

std::string RobotFleet::trimText(const std::string &text) const
{
    int start = 0;
    int end = (int)text.length() - 1;

    while (start <= end && (text[start] == ' ' || text[start] == '\t' || text[start] == '\r'))
        start++;

    while (end >= start && (text[end] == ' ' || text[end] == '\t' || text[end] == '\r'))
        end--;

    if (start > end)
        return "";

    return text.substr(start, end - start + 1);
}

bool RobotFleet::registerRobot(const std::string &id, const std::string &type, const std::string &locID,
                               double battery, double maxCap, const WarehouseGraph *graph,
                               const std::string &status)
{
    if (searchRobot(id) != nullptr)
    {
        std::cout << "Robot already exists: " << id << std::endl;
        return false;
    }

    if (graph != nullptr && graph->searchLocation(locID) == nullptr)
    {
        std::cout << "Location not found: " << locID << std::endl;
        return false;
    }

    if (battery < 0.0 || battery > 100.0)
    {
        std::cout << "Invalid battery level for " << id << std::endl;
        return false;
    }

    Robot *newBot = nullptr;

    // type k hisab se capacity check kr k object bnana
    if (type == "Picker")
    {
        if (maxCap <= 0.0 || maxCap > 15.0)
        {
            std::cout << "Picker capacity must be <= 15kg." << std::endl;
            return false;
        }
        newBot = new Picker(id, locID, battery, maxCap, status);
    }
    else if (type == "Carrier")
    {
        if (maxCap < 15.0 || maxCap > 30.0)
        {
            std::cout << "Carrier capacity must be 15 to 30kg." << std::endl;
            return false;
        }
        newBot = new Carrier(id, locID, battery, maxCap, status);
    }
    else if (type == "HeavyDuty")
    {
        if (maxCap <= 0.0 || maxCap > 60.0)
        {
            std::cout << "HeavyDuty capacity must be <= 60kg." << std::endl;
            return false;
        }
        newBot = new HeavyDuty(id, locID, battery, maxCap, status);
    }
    else
    {
        std::cout << "Unknown robot type: " << type << std::endl;
        return false;
    }

    // ye add kry ga at the end of the linked list
    RobotNode *node = new RobotNode(newBot);
    if (head == nullptr)
    {
        head = node;
    }
    else
    {
        RobotNode *curr = head;
        while (curr->next != nullptr)
            curr = curr->next;
        curr->next = node;
    }
    return true;
}

bool RobotFleet::loadRobots(const std::string &filename, const WarehouseGraph *graph)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Cannot open " << filename << std::endl;
        return false;
    }

    // agr first line ma koi count aye like 6 to skip kr do
    std::string line;
    std::getline(file, line);
    if (!isNumberLine(line))
    {
        file.clear();
        file.seekg(0);
    }

    std::string id, type, locID, status;
    double battery, maxCap;

    while (file >> id >> type >> locID >> battery >> maxCap)
    {
        std::getline(file, status);
        status = trimText(status);
        if (status.empty())
            status = "Available";

        registerRobot(id, type, locID, battery, maxCap, graph, status);
    }

    file.close();
    return true;
}

Robot *RobotFleet::searchRobot(const std::string &robotID) const
{
    RobotNode *curr = head;
    while (curr != nullptr)
    {
        if (curr->data->getID() == robotID)
            return curr->data;
        curr = curr->next;
    }
    return nullptr;
}

bool RobotFleet::assignRobot(const std::string &robotID, const std::string &orderID)
{
    Robot *bot = searchRobot(robotID);
    if (bot == nullptr)
    {
        std::cout << "Robot not found: " << robotID << std::endl;
        return false;
    }

    if (bot->getStatus() != "Available" || !bot->getAssignedTask().empty())
    {
        std::cout << "Robot is not available: " << robotID << std::endl;
        return false;
    }

    bot->setAssignedTask(orderID);
    bot->setStatus("Assigned");
    return true;
}

bool RobotFleet::releaseRobot(const std::string &robotID)
{
    Robot *bot = searchRobot(robotID);
    if (bot == nullptr)
    {
        std::cout << "Robot not found: " << robotID << std::endl;
        return false;
    }

    bot->setAssignedTask("");
    bot->setStatus("Available");

    if (bot->getBattery() < 20.0)
    {
        std::cout << "Warning: " << robotID << " battery is low (" << bot->getBattery() << "%), needs charging." << std::endl;
    }
    return true;
}

bool RobotFleet::setMaintenanceStatus(const std::string &robotID, bool toMaintenance, std::string &outOldStatus)
{
    Robot *bot = searchRobot(robotID);
    if (bot == nullptr)
    {
        std::cout << "Robot not found: " << robotID << std::endl;
        return false;
    }

    std::string currStatus = bot->getStatus();
    if (currStatus == "Assigned" || currStatus == "Charging" || currStatus == "Waiting for Charging")
    {
        std::cout << "Cannot change maintenance status while robot is " << currStatus << std::endl;
        return false;
    }

    outOldStatus = currStatus;
    if (toMaintenance)
        bot->setStatus("Unavailable");
    else
        bot->setStatus("Available");
    return true;
}

double RobotFleet::calculateLegBattery(const std::string &robotID, int distance, double carriedWeight) const
{
    Robot *bot = searchRobot(robotID);
    if (bot == nullptr)
        return -1.0;
    return bot->calculateLegBattery(distance, carriedWeight);
}

void RobotFleet::displayAllRobots() const
{
    if (head == nullptr)
    {
        std::cout << "No robots in fleet." << std::endl;
        return;
    }

    std::cout << "\nRobot Fleet:" << std::endl;
    RobotNode *curr = head;
    while (curr != nullptr)
    {
        Robot *b = curr->data;
        std::cout << b->getID() << " | " << b->getType()<< " | Loc: " << b->getLocation()<< " | Bat: " << b->getBattery() << "%"<< " | Cap: " << b->getMaxCapacity() << "kg"<< " | " << b->getStatus();
        if (!b->getAssignedTask().empty())
            std::cout << " (" << b->getAssignedTask() << ")";
        std::cout << std::endl;
        curr = curr->next;
    }
}
