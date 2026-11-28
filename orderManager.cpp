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
    if (status != "Pending" && status != "Reserved" && status != "Assigned" &&
        status != "Partially Fulfilled" && status != "Completed" && status != "Cancelled")
        return false;

    return true;
}

bool OrderManager::loadOrders(const string &filename, bool reserveImmediately)
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
    if (!(fin >> total))
    {
        fin.close();
        return false;
    }
    for (int i = 0; i < total; i++)
    {
        string orderId, custId;
        int pNo = 0;
        if (!(fin >> orderId >> custId >> pNo))
            break;
        SinglyLinkedList<OrderItem> list;
        for (int j = 0; j < pNo; j++)
        {
            string pId;
            int reqQty = 0;
            OrderItem oItem;
            if (!(fin >> pId >> reqQty))
                break;
            oItem.productID = pId;
            oItem.requiredQty = reqQty;
            list.insertBack(oItem);
        }

        createOrder(orderId, custId, list, reserveImmediately);
    }

    fin.close();
    return true;
}

bool OrderManager::createOrder(const string &orderID, const string &customerID, const SinglyLinkedList<OrderItem> &items, bool reserveImmediately)
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
        if (p->data.requiredQty <= 0)
        {
            if (reserveImmediately)
                inventory->releaseOrderReservations(orderID);
            return false;
        }
        int itemReserved = 0;
        if (reserveImmediately)
        {
            SinglyLinkedList<RackAllocation> outAllocations;
            bool isRes = inventory->reserveStock(orderID, p->data.productID, p->data.requiredQty, outAllocations);
            if (!isRes)
            {
                inventory->releaseOrderReservations(orderID);
                return false;
            }
            itemReserved = p->data.requiredQty;
        }
        else if (!inventory->productExists(p->data.productID))
        {
            return false;
        }

        // -- Merge quantities when the same product appears multiple times
        OrderItem *existing = o.items.findIf([&](const OrderItem &it)
                                             { return it.productID == p->data.productID; });
        if (existing)
        {
            existing->requiredQty += p->data.requiredQty;
            existing->reservedQty += itemReserved;
        }
        else
        {
            OrderItem itemCopy;
            itemCopy.productID = p->data.productID;
            itemCopy.requiredQty = p->data.requiredQty;
            itemCopy.reservedQty = itemReserved;
            itemCopy.deliveredQty = 0;
            o.items.insertBack(itemCopy);
        }

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

void OrderManager::pushFrontPendingOrder(const string &orderID)
{
    if (orderID.empty())
        return;
    pendingQueue.removeByValue(orderID);
    pendingQueue.enqueueFront(orderID);
}

bool OrderManager::removeFromPendingQueue(const string &orderID)
{
    if (orderID.empty())
        return false;
    return pendingQueue.removeByValue(orderID);
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

    pendingQueue.removeByValue(orderID);
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
    syncOrderReservations(orderID);
    o->status = "Completed";
    return true;
}

bool OrderManager::cancelOrder(const string &orderID)
{
    if (orderID.empty() || !inventory)
        return false;

    Order *o = getOrder(orderID);
    if (!o || (o->status != "Pending" && o->status != "Reserved" && o->status != "Assigned"))
        return false;

    if (o->status == "Pending")
        pendingQueue.removeByValue(orderID);

    inventory->releaseOrderReservations(orderID);
    syncOrderReservations(orderID);
    o->status = "Cancelled";
    return true;
}

void OrderManager::syncOrderReservations(const string &orderID)
{
    if (orderID.empty() || !inventory)
        return;

    Order *o = getOrder(orderID);
    if (!o)
        return;

    ListNode<OrderItem> *it = o->items.getHead();
    while (it)
    {
        it->data.reservedQty = inventory->getReservedStockForOrder(orderID, it->data.productID);
        it = it->next;
    }
}

void OrderManager::displayAllOrders() const
{
    if (masterOrders.isEmpty())
    {
        UI::printWarning("No customer orders recorded yet.");
        return;
    }

    UI::printSectionBanner("CUSTOMER ORDERS REGISTRY");
    const ListNode<Order> *p = masterOrders.getHead();
    while (p)
    {
        string stColor = (p->data.status == "Completed") ? UI::okColor()
                            : ((p->data.status == "Cancelled") ? UI::errColor() : UI::warnColor());
        string row = UI::accent() + p->data.orderID + UI::reset() +
                          " | Customer: " + p->data.customerID +
                          " | Status: " + stColor + p->data.status + UI::reset();
        if (!p->data.assignedRobotID.empty())
            row += " | Robot: " + p->data.assignedRobotID;
        UI::printRow(row);
        p = p->next;
    }
    UI::drawDecoration('_', '=', '_');
}

void OrderManager::displayOrderDetails(const string &orderID) const
{
    if (orderID.empty())
    {
        UI::printError("Order ID cannot be empty.");
        return;
    }

    const Order *o = getOrder(orderID);
    if (!o)
    {
        UI::printError("Order " + orderID + " does not exist.");
        return;
    }

    UI::printSectionBanner("ORDER DETAILS : " + o->orderID);
    UI::printRow("Order ID    : " + UI::accent() + o->orderID + UI::reset());
    UI::printRow("Customer ID : " + o->customerID);
    UI::printRow("Status      : " + UI::bold() + o->status + UI::reset() +
                 (o->assignedRobotID.empty() ? "" : " (Robot: " + o->assignedRobotID + ")"));
    UI::drawDecoration('+', '-', '+');
    UI::printCentered(UI::infoColor() + "LINE ITEMS" + UI::reset(), '|');
    UI::drawDecoration('|', '-', '|');

    const ListNode<OrderItem> *p = o->items.getHead();
    while (p)
    {
        UI::printRow(UI::accent() + p->data.productID + UI::reset() +
                     " | Required: " + to_string(p->data.requiredQty) +
                     " | Reserved: " + to_string(p->data.reservedQty) +
                     " | Delivered: " + to_string(p->data.deliveredQty));
        p = p->next;
    }
    UI::drawDecoration('_', '=', '_');
}

void OrderManager::displayOrdersByStatus(const string &status) const
{
    if (masterOrders.isEmpty() || !validStatus(status))
    {
        UI::printWarning("No orders match status '" + status + "' or status is invalid.");
        return;
    }

    UI::printSectionBanner("ORDERS FILTERED BY STATUS : " + status);
    const ListNode<Order> *o = masterOrders.getHead();
    bool any = false;
    while (o)
    {
        if (o->data.status == status)
        {
            any = true;
            displayOrderDetails(o->data.orderID);
        }
        o = o->next;
    }
    if (!any)
        UI::printInfo("No orders currently have status '" + status + "'.");
}

void OrderManager::displayPendingQueue() const
{
    if (pendingQueue.isEmpty())
    {
        UI::printWarning("Pending order queue is currently empty.");
        return;
    }

    UI::printSectionBanner("PENDING ORDER QUEUE (FCFS : FRONT -> REAR)");
    const QueueNode<string> *curr = pendingQueue.getFrontNode();
    int pos = 1;
    bool any = false;
    while (curr)
    {
        const Order *o = getOrder(curr->data);
        if (o && o->status == "Pending")
        {
            any = true;
            UI::printRow(UI::infoColor() + "#" + to_string(pos++) + UI::reset() +
                         " | " + UI::accent() + o->orderID + UI::reset() +
                         " | Customer: " + o->customerID);
        }
        curr = curr->next;
    }
    if (!any)
        UI::printWarning("No active Pending orders in queue.");
    UI::drawDecoration('_', '=', '_');
}