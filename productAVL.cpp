#include "productAVL.h"

bool ProductAVL::isValidRack(const string &rackID) const
{
    const RackInfo *p = validRacks.findIf([&](const RackInfo &info)
                                          { return info.rackID == rackID; });

    if (p)
        return true;
    return false;
}

string ProductAVL::getRackLocation(const string &rackID) const
{
    const RackInfo *p = validRacks.findIf([&](const RackInfo &info)
                                          { return info.rackID == rackID; });

    if (p)
        return p->locationID;
    return "";
}

bool ProductAVL::addProduct(const string &productID, const string &name, double unitWeight, int reorderLevel)
{
    if (unitWeight <= 0 || reorderLevel < 0)
        return false;

    Product p;
    p.productID = productID;
    p.name = name;
    p.unitWeight = unitWeight;
    p.reorderLevel = reorderLevel;

    return catalog.insert(productID, p);
}

bool ProductAVL::deleteProduct(const string &productID)
{
    Product *p = catalog.search(productID);

    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &rE)
                                   { return rE.reservedQty > 0; });

    if (r)
        return false;

    return catalog.remove(productID);
}

bool ProductAVL::productExists(const string &productID) const
{
    return catalog.contains(productID);
}

Product *ProductAVL::getProduct(const string &productID)
{
    return catalog.search(productID);
}

const Product *ProductAVL::getProduct(const string &productID) const
{
    return catalog.search(productID);
}

bool ProductAVL::addRackToProduct(const string &productID, const string &rackID, int initialQty)
{

    if (initialQty < 0)
        return false;

    if (!isValidRack(rackID))
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &valR)
                                   { return valR.rackID == rackID; });

    if (r)
        return false;

    RackEntry rE;
    rE.rackID = rackID;
    rE.totalQty = initialQty;

    p->racks.insertBack(rE);
    return true;
}

bool ProductAVL::updateRackQuantity(const string &productID, const string &rackID, int newTotalQty, int *outOldQty)
{
    if (!isValidRack(rackID))
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &valR)
                                   { return valR.rackID == rackID; });

    if (!r)
        return false;

    if (newTotalQty < r->reservedQty)
        return false;

    if (outOldQty)
    {
        *outOldQty = r->totalQty;
    }
    r->totalQty = newTotalQty;
    return true;
}

bool ProductAVL::removeEmptyRackFromProduct(const string &productID, const string &rackID)
{
    if (!isValidRack(rackID))
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &valR)
                                   { return (valR.rackID == rackID && valR.totalQty == 0 && valR.reservedQty == 0); });

    if (!r)
        return false;

    return p->racks.removeIf([&](const RackEntry &valR)
                             { return valR.rackID == rackID; });
}

int ProductAVL::getTotalStock(const string &productID) const
{
    const Product *p = catalog.search(productID);
    if (!p)
        return -1;

    int totalS = 0;

    ListNode<RackEntry> *r = p->racks.getHead();
    while (r)
    {
        totalS += r->data.totalQty;
        r = r->next;
    }

    return totalS;
}

int ProductAVL::getTotalReservedStock(const string &productID) const
{
    const Product *p = catalog.search(productID);
    if (!p)
        return -1;

    int totalRS = 0;

    ListNode<RackEntry> *r = p->racks.getHead();
    while (r)
    {
        totalRS += r->data.reservedQty;
        r = r->next;
    }

    return totalRS;
}

int ProductAVL::getAvailableStock(const string &productID) const
{
    const Product *p = catalog.search(productID);
    if (!p)
        return -1;

    int totalAS = 0;

    ListNode<RackEntry> *r = p->racks.getHead();
    while (r)
    {
        totalAS += r->data.getAvailableQty();
        r = r->next;
    }

    return totalAS;
}

bool ProductAVL::reserveStock(const string &orderID, const string &productID, int qtyNeeded, SinglyLinkedList<RackAllocation> &outAllocations)
{
    if (qtyNeeded <= 0 || orderID.empty())
        return false;

    Product *p = catalog.search(productID);

    if (!p)
        return false;

    if (getAvailableStock(productID) < qtyNeeded)
        return false;

    ListNode<RackEntry> *rH = p->racks.getHead();

    if (!rH)
        return false;

    outAllocations.clear();

    while (rH)
    {
        if (rH->data.getAvailableQty() != 0)
        {
            int validQt = rH->data.getAvailableQty();
            if (qtyNeeded < validQt)
            {
                validQt = qtyNeeded;
            }
            qtyNeeded -= validQt;
            rH->data.reservedQty += validQt;
            ReservationRecord *resR = rH->data.reservations.findIf([&](const ReservationRecord &resRU)
                                                                   { return resRU.orderID == orderID; });
            if (!resR)
            {
                ReservationRecord newRec;
                newRec.orderID = orderID;
                newRec.quantity = validQt;
                rH->data.reservations.insertBack(newRec);
            }
            else
            {
                resR->quantity += validQt;
            }

            RackAllocation rackA;
            rackA.rackID = rH->data.rackID;
            rackA.locationID = getRackLocation(rackA.rackID);
            rackA.quantity = validQt;
            outAllocations.insertBack(rackA);
            if (qtyNeeded == 0)
                break;
        }
        rH = rH->next;
    }

    if (outAllocations.getSize() != 0)
        return true;
    else
        return false;
}

void ProductAVL::releaseOrderReservations(const string &orderID)
{
    if (orderID.empty())
        return;
    catalog.traverseInOrder([&](string &id, Product &p)
                            {
                                ListNode<RackEntry> *r = p.racks.getHead();
                                while (r)
                                {
                                    int qty = 0;
                                    ListNode<ReservationRecord> *rr = r->data.reservations.getHead();
                                    while(rr)
                                    {
                                        ListNode<ReservationRecord>*temp=rr->next;
                                        if (rr->data.orderID == orderID)
                                        {
                                            qty+=rr->data.quantity;
                                            r->data.reservations.removeByValue(rr->data);
                                        }
                                        rr=temp;
                                    }
                                    r->data.reservedQty-=qty;
                                    if(r->data.reservedQty<0) r->data.reservedQty = 0;
                                    r=r->next;
                                } });
}

void ProductAVL::commitOrderPick(const string &orderID)
{
    if (orderID.empty())
        return;
    catalog.traverseInOrder([&](string &id, Product &p)
                            {
                                ListNode<RackEntry> *r = p.racks.getHead();
                                while (r)
                                {
                                    int qty = 0;
                                    ListNode<ReservationRecord> *rr = r->data.reservations.getHead();
                                    while(rr)
                                    {
                                        ListNode<ReservationRecord>*temp=rr->next;
                                        if (rr->data.orderID == orderID)
                                        {
                                            qty+=rr->data.quantity;
                                            r->data.reservations.removeByValue(rr->data);
                                        }
                                        rr=temp;
                                    }
                                    r->data.reservedQty-=qty;
                                    r->data.totalQty-=qty;
                                    if(r->data.reservedQty<0) r->data.reservedQty = 0;
                                    if(r->data.totalQty<0) r->data.totalQty = 0;
                                    r=r->next;
                                } });
}

void ProductAVL::displayAllProducts() const
{
    if (catalog.isEmpty())
    {
        cout << "Empty Catalog!" << endl;
        return;
    }

    cout << "===== All Products ======" << endl;
    catalog.traverseInOrder([&](const string &id, const Product &p)
                            {
        cout << endl << "Product Id: " << id << endl;
        cout << "Product Name: " << p.name << endl; });
}

void ProductAVL::displayProductDetails(const string &productID) const
{
    if (productID.empty())
    {
        cout << "Invalid Product Id!" << endl;
        return;
    }

    const Product *p = catalog.search(productID);
    if (!p)
    {
        cout << "Product does not exist!" << endl;
        return;
    }

    cout << "===== Product: " << productID << " Information =====" << endl;
    cout << "Product ID: " << p->productID << endl;
    cout << "Product Name: " << p->name << endl;
    cout << "Unit Weight: " << p->unitWeight << endl;
    cout << "Reorder Level: " << p->reorderLevel << endl;

    ListNode<RackEntry> *r = p->racks.getHead();
    if (!r)
    {
        cout << "No Rack Allocated!" << endl;
        return;
    }
    cout << "===== Rack Allocation Information =====" << endl;

    while (r)
    {
        cout << "\nRack ID: " << r->data.rackID << endl;
        cout << "Rack Location: " << getRackLocation(r->data.rackID) << endl;
        cout << "Total Quantity: " << r->data.totalQty << endl;
        cout << "Reserved Quantity: " << r->data.reservedQty << endl;
        cout << "Available Quantity: " << r->data.getAvailableQty() << endl;
        r = r->next;
    }
}

void ProductAVL::displayLowStockAlerts() const
{
    if (catalog.isEmpty())
    {
        cout << "Empty Catalog!" << endl;
        return;
    }
    bool anyAlert = false;
    catalog.traverseInOrder([&](const string &id, const Product &p)
                            {
         int thresh = p.reorderLevel;
         int totalQuant = 0;
         ListNode<RackEntry>*r=p.racks.getHead();
         while(r)
         {
            totalQuant+=r->data.getAvailableQty();
            r=r->next;
         }

         if(totalQuant <= thresh)
         {
            anyAlert=true;
            cout << "=== Low Stock " << p.productID << " alert ===" << endl;
            displayProductDetails(p.productID);
         } });

    if (!anyAlert)
    {
        cout << "No Low Stock Alert!" << endl;
    }
}

bool ProductAVL::loadRacks(const string &filename)
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
        string rackId, rackLoc;
        fin >> rackId >> rackLoc;
        RackInfo rack;
        rack.rackID = rackId;
        rack.locationID = rackLoc;
        validRacks.insertBack(rack);
    }

    fin.close();
    return true;
}

bool ProductAVL::loadProducts(const string &filename)
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
        string id, name;
        double weight;
        int level;

        fin >> id >> name >> weight >> level;
        addProduct(id, name, weight, level);
    }

    fin.close();
    return true;
}

bool ProductAVL::loadInventory(const string &filename)
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
        string pId, rId;
        int qty;

        fin >> pId >> rId >> qty;
        addRackToProduct(pId, rId, qty);
    }

    fin.close();
    return true;
}