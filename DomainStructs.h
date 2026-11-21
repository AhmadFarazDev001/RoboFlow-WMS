#pragma once
#include "library.h"


// Module 2: Valid rack mapping loaded from racks.txt
struct RackInfo {
    string rackID;
    string locationID;
};

// Module 2: Order reservation attached to a specific rack
struct ReservationRecord {
    string orderID;
    int quantity = 0;
};

// Module 2: Storage rack entry attached to a product
struct RackEntry {
    string rackID;
    int totalQty = 0;
    int reservedQty = 0;
    SinglyLinkedList<ReservationRecord> reservations;

    int getAvailableQty() const {
        return totalQty - reservedQty;
    }
};

// Module 2: Product record stored as value in AVLTree<string, Product>
struct Product {
    string productID;
    string name;
    double unitWeight = 0.0;
    int reorderLevel = 0;
    SinglyLinkedList<RackEntry> racks;
};

// Module 2 <-> Module 5: Returned when reserving stock across racks
struct RackAllocation {
    string rackID;
    string locationID;
    int quantity = 0;
};

// Module 3: Single line item inside a customer order
struct OrderItem {
    string productID;
    int requiredQty = 0;
    int reservedQty = 0;
};

// Module 3: Customer order stored in master SinglyLinkedList<Order>
struct Order {
    string orderID;
    string customerID;
    string status = "Pending"; // Pending, Reserved, Assigned, Completed, Cancelled
    SinglyLinkedList<OrderItem> items;
};

// Module 8: Reversible admin action stored in Stack<UndoRecord>
struct UndoRecord {
    string actionType;   // "RACK_UPDATE", "BLOCK_PATHWAY", "ROBOT_MAINTENANCE"
    string primaryID;    // productID, locationA, or robotID
    string secondaryID;  // rackID, locationB, or ""
    int oldIntVal = 0;
    int newIntVal = 0;
    string oldStrVal;
    string newStrVal;
};
