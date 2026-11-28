// -- Written by Ahmad: Shared domain data structures for inventory, orders, and undo history
#pragma once
#include "library.h"

struct RackInfo {
    string rackID;
    string locationID;
};

struct ReservationRecord {
    string orderID;
    int quantity = 0;

    // -- Compares two reservation records for equality
    bool operator==(const ReservationRecord& r) const
    {
        if(orderID==r.orderID && quantity==r.quantity)
        return true;

        return false;
    }

    // -- Compares two reservation records for inequality
    bool operator!=(const ReservationRecord& r) const
    {
        return !(*this == r);
    }
};

struct RackEntry {
    string rackID;
    int totalQty = 0;
    int reservedQty = 0;
    SinglyLinkedList<ReservationRecord> reservations;

    // -- Calculates unreserved stock available on this rack
    int getAvailableQty() const {
        return totalQty - reservedQty;
    }
};

struct Product {
    string productID;
    string name;
    double unitWeight = 0.0;
    int reorderLevel = 0;
    SinglyLinkedList<RackEntry> racks;
};

struct RackAllocation {
    string rackID;
    string locationID;
    int quantity = 0;
};

struct OrderItem {
    string productID;
    int requiredQty = 0;
    int reservedQty = 0;
    int deliveredQty = 0;
};

struct Order {
    string orderID;
    string customerID;
    string status = "Pending";
    string assignedRobotID;
    SinglyLinkedList<OrderItem> items;
};

struct UndoRecord {
    string actionType;
    string primaryID;
    string secondaryID;
    int oldIntVal = 0;
    int newIntVal = 0;
    string oldStrVal;
    string newStrVal;
};
