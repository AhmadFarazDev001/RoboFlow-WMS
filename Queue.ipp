#pragma once
#include "Queue.h"

IntQueue::IntQueue() : front(nullptr), rear(nullptr) {}

IntQueue::~IntQueue()
{
    while (!isEmpty())
        dequeue();
}

void IntQueue::enqueue(int val)
{
    IntQueueNode *node = new IntQueueNode(val);
    if (rear == nullptr)
    {
        front = rear = node;
        return;
    }
    rear->next = node;
    rear = node;
}

int IntQueue::dequeue()
{
    if (isEmpty())
        return -1;

    IntQueueNode *temp = front;
    int val = temp->data;
    front = front->next;
    if (front == nullptr)
        rear = nullptr;

    delete temp;
    return val;
}

// SlotQueue functions

SlotQueue::SlotQueue(int slots) : totalSlots(slots), waitFront(nullptr), waitRear(nullptr)
{
    if (totalSlots > 0)
        activeSlots = new std::string[totalSlots];
    else
        activeSlots = nullptr;
}

SlotQueue::~SlotQueue()
{
    delete[] activeSlots;
    while (!isQueueEmpty())
        dequeueRobot();
}

int SlotQueue::findFreeSlot() const
{
    for (int i = 0; i < totalSlots; i++)
    {
        if (activeSlots[i].empty())
            return i;
    }
    return -1;
}

int SlotQueue::findRobotInSlots(const std::string &robotID) const
{
    for (int i = 0; i < totalSlots; i++)
    {
        if (activeSlots[i] == robotID)
            return i;
    }
    return -1;
}

void SlotQueue::assignSlot(int slotIndex, const std::string &robotID)
{
    if (slotIndex >= 0 && slotIndex < totalSlots)
        activeSlots[slotIndex] = robotID;
}

void SlotQueue::clearSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < totalSlots)
        activeSlots[slotIndex] = "";
}

std::string SlotQueue::getSlotOccupant(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < totalSlots)
        return activeSlots[slotIndex];
    return "";
}

void SlotQueue::enqueueRobot(const std::string &robotID)
{
    WaitNode *node = new WaitNode(robotID);
    if (waitRear == nullptr)
    {
        waitFront = waitRear = node;
        return;
    }
    waitRear->next = node;
    waitRear = node;
}

std::string SlotQueue::dequeueRobot()
{
    if (isQueueEmpty())
        return "";

    WaitNode *temp = waitFront;
    std::string id = temp->robotID;
    waitFront = waitFront->next;
    if (waitFront == nullptr)
        waitRear = nullptr;

    delete temp;
    return id;
}

bool SlotQueue::removeFromQueue(const std::string &robotID)
{
    if (isQueueEmpty())
        return false;

    WaitNode *curr = waitFront;
    WaitNode *prev = nullptr;
    while (curr != nullptr)
    {
        if (curr->robotID == robotID)
        {
            if (prev == nullptr)
                waitFront = curr->next;
            else
                prev->next = curr->next;

            if (curr == waitRear)
                waitRear = prev;

            delete curr;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

void SlotQueue::displayQueue() const
{
    if (isQueueEmpty())
    {
        std::cout << "Empty";
        return;
    }

    WaitNode *curr = waitFront;
    while (curr != nullptr)
    {
        std::cout << curr->robotID;
        if (curr->next != nullptr)
            std::cout << " -> ";
        curr = curr->next;
    }
}
