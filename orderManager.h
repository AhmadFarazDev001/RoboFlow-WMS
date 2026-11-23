#pragma once

#include "library.h"
#include "DomainStructs.h"
#include "productAVL.h"

class OrderManager
{
private:
    SinglyLinkedList<Order> masterOrders;
    Queue<string> pendingQueue;
    ProductAVL *inventory; // Pointer to shared catalog for stock reservation/commit/release
    bool validStatus(const string& status) const;
public:
    // ---Section 1: Constructor & Startup File Loading---
    explicit OrderManager(ProductAVL *productCatalog = nullptr);
    void setProductCatalog(ProductAVL *productCatalog);
    bool loadOrders(const string &filename);

    // ---Section 2: Order Creation & Lookup---
    bool createOrder(const string &orderID,
                     const string &customerID,
                     const SinglyLinkedList<OrderItem> &items);
    bool orderExists(const string &orderID) const;
    Order *getOrder(const string &orderID);
    const Order *getOrder(const string &orderID) const;
    int getTotalOrderCount() const;
    int getPendingOrderCount() const;

    // ---Section 3: Pending Queue Operations---
    bool hasPendingOrders() const;
    string peekNextPendingOrder() const;
    bool dequeueNextPendingOrder(string &outOrderID);

    // ---Section 4: Order Lifecycle & Status Transitions---
    bool updateOrderStatus(const string &orderID, const string &newStatus);
    bool markOrderAssigned(const string &orderID);
    bool completeOrder(const string &orderID);
    bool cancelOrder(const string &orderID);

    // ---Section 5: Display & Reporting---
    void displayAllOrders() const;
    void displayOrderDetails(const string &orderID) const;
    void displayOrdersByStatus(const string &status) const;
    void displayPendingQueue() const;
};
