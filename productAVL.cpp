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
    if (productID.empty() || name.empty() || unitWeight <= 0 || reorderLevel < 0)
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

    if (newTotalQty < 0 || newTotalQty < r->reservedQty)
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

int ProductAVL::getReservedStockForOrder(const string &orderID, const string &productID) const
{
    if (orderID.empty() || productID.empty())
        return 0;

    const Product *p = catalog.search(productID);
    if (!p)
        return 0;

    int totalRes = 0;
    const ListNode<RackEntry> *r = p->racks.getHead();
    while (r)
    {
        const ReservationRecord *rec = r->data.reservations.findIf([&](const ReservationRecord &rr)
                                                                   { return rr.orderID == orderID; });
        if (rec)
            totalRes += rec->quantity;
        r = r->next;
    }
    return totalRes;
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

bool ProductAVL::reserveRackStock(const string &orderID, const string &productID, const string &rackID, int qty)
{
    if (orderID.empty() || productID.empty() || rackID.empty() || qty <= 0)
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &entry)
                                   { return entry.rackID == rackID; });
    if (!r || r->getAvailableQty() < qty)
        return false;

    r->reservedQty += qty;
    ReservationRecord *resR = r->reservations.findIf([&](const ReservationRecord &rec)
                                                     { return rec.orderID == orderID; });
    if (!resR)
    {
        ReservationRecord newRec;
        newRec.orderID = orderID;
        newRec.quantity = qty;
        r->reservations.insertBack(newRec);
    }
    else
    {
        resR->quantity += qty;
    }
    return true;
}

bool ProductAVL::releaseSingleStopReservation(const string &orderID, const string &productID, const string &rackID, int qty)
{
    if (orderID.empty() || productID.empty() || rackID.empty() || qty <= 0)
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &entry)
                                   { return entry.rackID == rackID; });
    if (!r)
        return false;

    ReservationRecord *resR = r->reservations.findIf([&](const ReservationRecord &rec)
                                                     { return rec.orderID == orderID; });
    if (!resR)
        return false;

    int releaseAmt = (qty < resR->quantity) ? qty : resR->quantity;
    resR->quantity -= releaseAmt;
    r->reservedQty -= releaseAmt;
    if (r->reservedQty < 0)
        r->reservedQty = 0;

    if (resR->quantity <= 0)
    {
        r->reservations.removeIf([&](const ReservationRecord &rec)
                                 { return rec.orderID == orderID && rec.quantity <= 0; });
    }
    return true;
}

bool ProductAVL::commitPickedStock(const string &orderID, const string &productID, const string &rackID, int qty)
{
    if (orderID.empty() || productID.empty() || rackID.empty() || qty <= 0)
        return false;

    Product *p = catalog.search(productID);
    if (!p)
        return false;

    RackEntry *r = p->racks.findIf([&](const RackEntry &entry)
                                   { return entry.rackID == rackID; });
    if (!r || r->totalQty < qty || r->reservedQty < qty)
        return false;

    ReservationRecord *resR = r->reservations.findIf([&](const ReservationRecord &rec)
                                                     { return rec.orderID == orderID; });
    if (!resR || resR->quantity < qty)
        return false;

    resR->quantity -= qty;
    r->reservedQty -= qty;
    r->totalQty -= qty;

    if (resR->quantity <= 0)
    {
        r->reservations.removeIf([&](const ReservationRecord &rec)
                                 { return rec.orderID == orderID && rec.quantity <= 0; });
    }
    return true;
}

void ProductAVL::releaseOrderReservations(const string &orderID)
{
    if (orderID.empty())
        return;
    catalog.traverseInOrder([&](string &id, Product &p)
                            {
                                (void)id;
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
                                (void)id;
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
        UI::printWarning("Product catalog is currently empty.");
        return;
    }

    UI::printSectionBanner("PRODUCT CATALOG (AVL IN-ORDER)");
    catalog.traverseInOrder([&](const string &id, const Product &p)
                            {
        ostringstream wStr;
        wStr << fixed << setprecision(2) << p.unitWeight;
        UI::printRow(UI::accent() + id + UI::reset() +
                     " | " + UI::bold() + p.name + UI::reset() +
                     " | Weight: " + wStr.str() + " kg" +
                     " | Reorder Level: " + to_string(p.reorderLevel) +
                     " | Avail: " + to_string(getAvailableStock(id))); });
    UI::drawDecoration('_', '=', '_');
}

void ProductAVL::displayProductDetails(const string &productID) const
{
    if (productID.empty())
    {
        UI::printError("Product ID cannot be empty.");
        return;
    }

    const Product *p = catalog.search(productID);
    if (!p)
    {
        UI::printError("Product " + productID + " does not exist in catalog.");
        return;
    }

    ostringstream wStr;
    wStr << fixed << setprecision(2) << p->unitWeight;

    UI::printSectionBanner("PRODUCT DETAILS : " + p->productID);
    UI::printRow("Product Name  : " + UI::bold() + p->name + UI::reset());
    UI::printRow("Unit Weight   : " + wStr.str() + " kg");
    UI::printRow("Reorder Level : " + to_string(p->reorderLevel) + " units");

    ListNode<RackEntry> *r = p->racks.getHead();
    if (!r)
    {
        UI::drawDecoration('+', '-', '+');
        UI::printRow(UI::warnColor() + "No storage racks currently hold this product." + UI::reset());
        UI::drawDecoration('_', '=', '_');
        return;
    }

    UI::drawDecoration('+', '-', '+');
    UI::printCentered(UI::infoColor() + "STORAGE RACK BREAKDOWN" + UI::reset(), '|');
    UI::drawDecoration('|', '-', '|');

    while (r)
    {
        UI::printRow(UI::accent() + r->data.rackID + UI::reset() +
                     " (Loc: " + getRackLocation(r->data.rackID) + ")" +
                     " | Total: " + to_string(r->data.totalQty) +
                     " | Reserved: " + to_string(r->data.reservedQty) +
                     " | Available: " + (r->data.getAvailableQty() > 0 ? UI::okColor() : UI::errColor()) +
                     to_string(r->data.getAvailableQty()) + UI::reset());
        r = r->next;
    }
    UI::drawDecoration('_', '=', '_');
}

void ProductAVL::displayLowStockAlerts() const
{
    if (catalog.isEmpty())
    {
        UI::printWarning("Product catalog is currently empty.");
        return;
    }
    bool anyAlert = false;
    catalog.traverseInOrder([&](const string &id, const Product &p)
                            {
         (void)id;
         int thresh = p.reorderLevel;
         int totalQuant = 0;
         ListNode<RackEntry>*r=p.racks.getHead();
         while(r)
         {
            totalQuant+=r->data.getAvailableQty();
            r=r->next;
         }

         if(totalQuant < thresh)
         {
            anyAlert=true;
            UI::printWarning("Low Stock Alert for " + p.productID + " (" + p.name + "): Available " +
                             to_string(totalQuant) + " < Reorder Level " + to_string(thresh));
            displayProductDetails(p.productID);
         } });

    if (!anyAlert)
    {
        UI::printSuccess("All products meet or exceed their reorder thresholds.");
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
    if (!(fin >> total))
    {
        fin.close();
        return false;
    }
    for (int i = 0; i < total; i++)
    {
        string rackId, rackLoc;
        if (!(fin >> rackId >> rackLoc))
            break;
        if (!rackId.empty() && !rackLoc.empty() && !isValidRack(rackId))
        {
            RackInfo rack;
            rack.rackID = rackId;
            rack.locationID = rackLoc;
            validRacks.insertBack(rack);
        }
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
    if (!(fin >> total))
    {
        fin.close();
        return false;
    }
    for (int i = 0; i < total; i++)
    {
        string id, name;
        double weight;
        int level;

        if (!(fin >> id >> name >> weight >> level))
            break;
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
    if (!(fin >> total))
    {
        fin.close();
        return false;
    }
    for (int i = 0; i < total; i++)
    {
        string pId, rId;
        int qty;

        if (!(fin >> pId >> rId >> qty))
            break;
        addRackToProduct(pId, rId, qty);
    }

    fin.close();
    return true;
}