#include "orderManager.h"

OrderManager::OrderManager(ProductAVL *productCatalog)
{
    inventory = productCatalog;
}

void OrderManager::setProductCatalog(ProductAVL *productCatalog)
{
    if (!productCatalog)
        return;

    inventory = productCatalog;
}

bool OrderManager::validStatus(const string &status) const
{
    if (status != "Pending" && status != "Reserved" && status != "Assigned" && status != "Completed" && status != "Cancelled")
        return false;

    return true;
}

bool OrderManager::loadOrders(const string &filename)
{
    if (filename.empty())
        return false;

    ifstream fin;
    fin.open(filename);
    if (!fin)
    {
        return false;
    }

    int total = 0;
    fin >> total;
    for (int i = 0; i < total; i++)
    {
        string orderId, custId;
        int pNo = 0;
        fin >> orderId >> custId >> pNo;
        SinglyLinkedList<OrderItem> list;
        for (int j = 0; j < pNo; j++)
        {
            string pId;
            int reqQty = 0;
            OrderItem oItem;
            fin >> pId >> reqQty;
            oItem.productID = pId;
            oItem.requiredQty = reqQty;
            list.insertBack(oItem);
        }

        createOrder(orderId, custId, list);
    }

    fin.close();
    return true;
}

bool OrderManager::createOrder(const string &orderID, const string &customerID, const SinglyLinkedList<OrderItem> &items)
{
    if (orderID.empty() || customerID.empty() || !inventory || orderExists(orderID))
        return false;

    if (items.isEmpty())
        return false;

    Order o;
    o.orderID = orderID;
    o.customerID = customerID;
    o.status = "Pending";

    const ListNode<OrderItem> *p = items.getHead();
    while (p)
    {
        SinglyLinkedList<RackAllocation> outAllocations;
        bool isRes = inventory->reserveStock(orderID, p->data.productID, p->data.requiredQty, outAllocations);
        if (!isRes)
        {
            inventory->releaseOrderReservations(orderID);
            return false;
        }

        OrderItem itemCopy;
        itemCopy.productID = p->data.productID;
        itemCopy.requiredQty = p->data.requiredQty;
        itemCopy.reservedQty = p->data.requiredQty;
        o.items.insertBack(itemCopy);

        p = p->next;
    }

    masterOrders.insertBack(o);
    pendingQueue.enqueue(o.orderID);
    return true;
}

bool OrderManager::orderExists(const string &orderID) const
{
    if (orderID.empty())
        return false;

    const Order *o = masterOrders.findIf([&](const Order &od)
                                         { return od.orderID == orderID; });

    return o != nullptr;
}

Order *OrderManager::getOrder(const string &orderID)
{
    if (orderID.empty())
        return nullptr;

    Order *o = masterOrders.findIf([&](const Order &od)
                                   { return od.orderID == orderID; });

    return o;
}

const Order *OrderManager::getOrder(const string &orderID) const
{
    if (orderID.empty())
        return nullptr;

    const Order *o = masterOrders.findIf([&](const Order &od)
                                         { return od.orderID == orderID; });

    return o;
}

int OrderManager::getTotalOrderCount() const
{
    return masterOrders.getSize();
}

int OrderManager::getPendingOrderCount() const
{
    const ListNode<Order> *p = masterOrders.getHead();
    int count = 0;
    while (p)
    {
        if (p->data.status == "Pending")
        {
            count++;
        }
        p = p->next;
    }

    return count;
}

bool OrderManager::hasPendingOrders() const
{
    return !pendingQueue.isEmpty();
}
string OrderManager::peekNextPendingOrder() const
{
    if (pendingQueue.isEmpty())
        return "";

    const string *id = pendingQueue.peekFront();
    if (!id)
        return "";

    return *id;
}

bool OrderManager::dequeueNextPendingOrder(string &outOrderID)
{
    if (pendingQueue.isEmpty())
        return false;

    while (!pendingQueue.isEmpty())
    {
        string *id = pendingQueue.peekFront();
        if (!id)
            return false;
        string order = *id;

        pendingQueue.dequeue();
        Order *o = getOrder(order);
        if (o && o->status == "Pending")
        {
            outOrderID = order;
            return true;
        }
    }

    return false;
}

bool OrderManager::updateOrderStatus(const string &orderID, const string &newStatus)
{
    if (orderID.empty() || !validStatus(newStatus))
        return false;

    if (newStatus == "Assigned")
        return markOrderAssigned(orderID);
    if (newStatus == "Completed")
        return completeOrder(orderID);
    if (newStatus == "Cancelled")
        return cancelOrder(orderID);

    Order *o = getOrder(orderID);
    if (!o)
        return false;

    o->status = newStatus;
    return true;
}

bool OrderManager::markOrderAssigned(const string &orderID)
{
    if (orderID.empty())
        return false;

    Order *o = getOrder(orderID);
    if (!o || o->status != "Pending")
        return false;

    o->status = "Assigned";
    return true;
}

bool OrderManager::completeOrder(const string &orderID)
{
    if (orderID.empty() || !inventory)
        return false;

    Order *o = getOrder(orderID);
    if (!o || o->status != "Assigned")
        return false;

    inventory->commitOrderPick(orderID);
    o->status = "Completed";
    return true;
}

bool OrderManager::cancelOrder(const string &orderID)
{
    if (orderID.empty() || !inventory)
        return false;

    Order *o = getOrder(orderID);
    if (!o || o->status != "Pending" && o->status != "Assigned")
        return false;

    inventory->releaseOrderReservations(orderID);
    o->status = "Cancelled";
    return true;
}

void OrderManager::displayAllOrders() const
{
    if (masterOrders.isEmpty())
    {
        cout << "No Orders Yet!" << endl;
        return;
    }

    cout << "===== All Orders Information =====" << endl;
    const ListNode<Order> *p = masterOrders.getHead();
    while (p)
    {
        cout << "\nOrder Id: " << p->data.orderID << endl;
        cout << "Customer Id: " << p->data.customerID << endl;
        cout << "Status: " << p->data.status << endl;
        p = p->next;
    }
}

void OrderManager::displayOrderDetails(const string &orderID) const
{
    if (orderID.empty())
    {
        cout << "Invalid Order ID!" << endl;
        return;
    }

    const Order *o = getOrder(orderID);
    if (!o)
    {
        cout << "Order does not exists!" << endl;
        return;
    }

    cout << "===== Order " << o->orderID << " Details =====" << endl;
    cout << "Order Id: " << o->orderID << endl;
    cout << "Customer Id: " << o->customerID << endl;
    cout << "Status: " << o->status << endl;
    cout << "--- Items Information ---" << endl;
    const ListNode<OrderItem> *p = o->items.getHead();
    while (p)
    {
        cout << "\nProduct ID: " << p->data.productID << endl;
        cout << "Required Quantity: " << p->data.requiredQty << endl;
        cout << "Reserved Quantity: " << p->data.reservedQty << endl;
        p = p->next;
    }
}

void OrderManager::displayOrdersByStatus(const string &status) const
{
    if (masterOrders.isEmpty() || !validStatus(status))
    {
        cout << "Invalid Configuration!" << endl;
        return;
    }

    const ListNode<Order> *o = masterOrders.getHead();
    cout << "===== Order of status " << status << " Information =====" << endl;
    while(o)
    {
        if(o->data.status==status)
        {
            displayOrderDetails(o->data.orderID);
        }
        o=o->next;
    }
}

void OrderManager::displayPendingQueue() const
{
    displayOrdersByStatus("Pending");
}