// -- Written by Ahmad: Customer order catalog and FIFO pending queue manager
#pragma once

#include "library.h"
#include "DomainStructs.h"
#include "productAVL.h"

class OrderManager
{
private:
    SinglyLinkedList<Order> masterOrders;
    Queue<string> pendingQueue;
    ProductAVL *inventory;

    // -- Validates whether a status string is a recognized order state
    bool validStatus(const string& status) const;
public:
    // -- Constructs the order manager with an optional product catalog pointer
    explicit OrderManager(ProductAVL *productCatalog = nullptr);
    // -- Attaches a product inventory catalog to this manager
    void setProductCatalog(ProductAVL *productCatalog);
    // -- Loads initial customer orders from file into the list and queue
    bool loadOrders(const string &filename, bool reserveImmediately = true);

    // -- Creates a new customer order and pushes it into the pending queue
    bool createOrder(const string &orderID,
                     const string &customerID,
                     const SinglyLinkedList<OrderItem> &items,
                     bool reserveImmediately = true);
    // -- Checks whether an order ID exists in master orders
    bool orderExists(const string &orderID) const;
    // -- Returns a modifiable pointer to an order record
    Order *getOrder(const string &orderID);
    // -- Returns a read-only pointer to an order record
    const Order *getOrder(const string &orderID) const;
    // -- Returns total count of all recorded orders
    int getTotalOrderCount() const;
    // -- Returns current count of orders waiting in the pending queue
    int getPendingOrderCount() const;

    // -- Checks whether any orders remain in the pending queue
    bool hasPendingOrders() const;
    // -- Returns the next pending order ID without removing it
    string peekNextPendingOrder() const;
    // -- Pops and outputs the next order ID from the pending queue
    bool dequeueNextPendingOrder(string &outOrderID);
    // -- Re-inserts an order ID at the front of the pending queue
    void pushFrontPendingOrder(const string &orderID);
    // -- Removes a specific order ID from anywhere in the pending queue
    bool removeFromPendingQueue(const string &orderID);
    // -- Returns the front node of the pending queue for traversal
    const QueueNode<string> *getPendingQueueFront() const { return pendingQueue.getFrontNode(); }
    // -- Returns the modifiable head node of master orders
    ListNode<Order> *getMasterOrdersHead() { return masterOrders.getHead(); }
    // -- Returns the read-only head node of master orders
    const ListNode<Order> *getMasterOrdersHead() const { return masterOrders.getHead(); }

    // -- Transitions an order to a valid target status
    bool updateOrderStatus(const string &orderID, const string &newStatus);
    // -- Marks a queued order as Assigned
    bool markOrderAssigned(const string &orderID);
    // -- Marks an order as Completed and commits stock picks
    bool completeOrder(const string &orderID);
    // -- Cancels an order and releases reserved stock
    bool cancelOrder(const string &orderID);
    // -- Syncs item reserved quantities with live catalog reservations
    void syncOrderReservations(const string &orderID);

    // -- Displays a summary table of all master orders
    void displayAllOrders() const;
    // -- Displays full item details and status for a single order
    void displayOrderDetails(const string &orderID) const;
    // -- Displays all orders matching a given status filter
    void displayOrdersByStatus(const string &status) const;
    // -- Displays the current FCFS pending order queue
    void displayPendingQueue() const;
};
