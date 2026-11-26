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
// ye true hoga if the line contains only digits or spaces
bool RobotFleet::isNumberLine(const std::string &line) const
{
    bool foundDigit = false;
    for (std::size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];
        if (c >= '0' && c <= '9')
            foundDigit = true;
        else if (c != ' ' && c != '\t' && c != '\r')
            return false;
    }
    return foundDigit;
}

bool RobotFleet::registerRobot(const std::string &id, const std::string &type, const std::string &locID, double battery, double maxCap, const WarehouseGraph *graph,
                               const std::string &status)
{
    if (searchRobot(id) != nullptr)
    {
        std::cout << "Error: Robot ID '" << id << "' already exists." << std::endl;
        return false;
    }

    if (graph != nullptr && graph->searchLocation(locID) == nullptr)
    {
        std::cout << "Error: Location '" << locID << "' does not exist." << std::endl;
        return false;
    }

    if (battery < 0.0 || battery > 100.0)
    {
        std::cout << "Error: Battery level must be between 0 and 100." << std::endl;
        return false;
    }

    Robot *newBot = nullptr;

    if (type == "Picker")
    {
        if (maxCap <= 0.0 || maxCap > 15.0)
        {
            std::cout << "Error: Picker capacity must be > 0 and <= 15 kg." << std::endl;
            return false;
        }
        newBot = new Picker(id, locID, battery, maxCap, status);
    }
    else if (type == "Carrier")
    {
        if (maxCap < 15.0 || maxCap > 30.0)
        {
            std::cout << "Error: Carrier capacity must be between 15 and 30 kg." << std::endl;
            return false;
        }
        newBot = new Carrier(id, locID, battery, maxCap, status);
    }
    else if (type == "HeavyDuty")
    {
        if (maxCap <= 0.0 || maxCap > 60.0)
        {
            std::cout << "Error: HeavyDuty capacity must be > 0 and <= 60 kg." << std::endl;
            return false;
        }
        newBot = new HeavyDuty(id, locID, battery, maxCap, status);
    }
    else
    {
        std::cout << "Error: Unknown robot type '" << type << "'." << std::endl;
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
        std::cout << "Error: Could not open " << filename << std::endl;
        return false;
    }

    // agr tu first line  ma koi  count aye like 5 skip it
    // otherwise go back to the start of the file
    std::string line;
    std::getline(file, line);
    if (!isNumberLine(line))
    {
        file.clear();
        file.seekg(0);
    }

    std::string id, type, locID, status;
    double battery, maxCap;

    // read the 5 fixed values straight from the file
    while (file >> id >> type >> locID >> battery >> maxCap)
    {
           std::getline(file, status);
        // remove  spaces
        std::size_t start = status.find_first_not_of(" \t");
        if (start == std::string::npos)
            status = "";
        else
            status = status.substr(start);
        // remove trailing \r and spaces
        while (!status.empty() && (status[status.length() - 1] == '\r' || status[status.length() - 1] == ' '))
            status.erase(status.length() - 1);
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
        std::cout << "Error: Robot '" << robotID << "' not found." << std::endl;
        return false;
    }

    if (bot->getStatus() != "Available" || !bot->getAssignedTask().empty())
    {
        std::cout << "Error: Robot '" << robotID << "' is not available." << std::endl;
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
        std::cout << "Error: Robot '" << robotID << "' not found." << std::endl;
        return false;
    }

    bot->setAssignedTask("");
    bot->setStatus("Available");

    if (bot->getBattery() < 20.0)
    {
        std::cout << "Alert: Robot " << robotID << " battery is low ("
                  << std::fixed << std::setprecision(1) << bot->getBattery()
                  << "%). Needs charging." << std::endl;
    }
    return true;
}

bool RobotFleet::setMaintenanceStatus(const std::string &robotID, bool toMaintenance, std::string &outOldStatus)
{
    Robot *bot = searchRobot(robotID);
    if (bot == nullptr)
    {
        std::cout << "Error: Robot '" << robotID << "' not found." << std::endl;
        return false;
    }

    std::string currStatus = bot->getStatus();
    if (currStatus == "Assigned" || currStatus == "Charging" || currStatus == "Waiting for Charging")
    {
        std::cout << "Error: Cannot change maintenance status while robot is " << currStatus << "." << std::endl;
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
        std::cout << "No robots registered in the fleet." << std::endl;
        return;
    }

    std::cout << "\n~~~ Robot Fleet ~~~" << std::endl;
    RobotNode *curr = head;
    while (curr != nullptr)
    {
        Robot *b = curr->data;
        std::cout << b->getID() << " | " << b->getType()
                  << " | Loc: " << b->getLocation()
                  << " | Battery: " << std::fixed << std::setprecision(1) << b->getBattery() << "%"
                  << " | MaxCap: " << b->getMaxCapacity() << "kg"
                  << " | Status: " << b->getStatus();
        if (!b->getAssignedTask().empty())
            std::cout << " (Task: " << b->getAssignedTask() << ")";
        std::cout << std::endl;
        curr = curr->next;
    }
}