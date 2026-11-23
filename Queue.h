#pragma once
#include "library.h"

struct IntQueueNode
{
    int data;
    IntQueueNode *next;

    IntQueueNode(int val) : data(val), next(nullptr) {}
};

// FIFO queue of vertex indices iused for BFS
class IntQueue
{
private:
    IntQueueNode *front;
    IntQueueNode *rear;

public:
    IntQueue();
    ~IntQueue();

    bool isEmpty() const { return front == nullptr; }
    void enqueue(int val);
    int dequeue();
};

struct WaitNode
{
    std::string robotID;
    WaitNode *next;

    WaitNode(const std::string &id) : robotID(id), next(nullptr) {}
};

// Manages active charging slots & the fiFo waiting queue for a station
class SlotQueue
{
private:
    int totalSlots;
    std::string *activeSlots;
    WaitNode *waitFront;
    WaitNode *waitRear;

public:
    SlotQueue(int slots = 0);
    ~SlotQueue();

    SlotQueue(const SlotQueue &) = delete;
    SlotQueue &operator=(const SlotQueue &) = delete;

    // Active slot operations
    int getTotalSlots() const { return totalSlots; }
    int findFreeSlot() const;
    int findRobotInSlots(const std::string &robotID) const;
    void assignSlot(int slotIndex, const std::string &robotID);
    void clearSlot(int slotIndex);
    std::string getSlotOccupant(int slotIndex) const;

    // Waiting queue operations
    bool isQueueEmpty() const { return waitFront == nullptr; }
    void enqueueRobot(const std::string &robotID);
    std::string dequeueRobot();
    bool removeFromQueue(const std::string &robotID);
    void displayQueue() const;
};

#include "Queue.ipp"