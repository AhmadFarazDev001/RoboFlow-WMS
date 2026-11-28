#include "undoManager.h"

bool UndoManager::isValidActionType(const string &actionType) const
{
    if (actionType != "RACK_UPDATE" && actionType != "BLOCK_PATHWAY" && actionType != "ROBOT_MAINTENANCE")
    {
        return false;
    }

    return true;
}

UndoManager::UndoManager(ProductAVL *productCatalog)
{
    inventory = productCatalog;
}

void UndoManager::setProductCatalog(ProductAVL *productCatalog)
{
    if (!productCatalog)
        return;

    inventory = productCatalog;
}

bool UndoManager::recordAction(const UndoRecord &record)
{
    if (!isValidActionType(record.actionType) || record.primaryID.empty())
        return false;
    historyStack.push(record);
    return true;
}

bool UndoManager::recordRackUpdate(const string &productID, const string &rackID, int oldQty, int newQty)
{
    if (productID.empty() || rackID.empty() || oldQty < 0 || newQty < 0 || oldQty == newQty)
        return false;

    UndoRecord rec;
    rec.actionType = "RACK_UPDATE";
    rec.primaryID = productID;
    rec.secondaryID = rackID;
    rec.newIntVal = newQty;
    rec.oldIntVal = oldQty;

    return recordAction(rec);
}

bool UndoManager::recordPathwayBlock(const string &fromLocationID, const string &toLocationID, bool wasBlocked, bool isBlocked)
{
    if (fromLocationID.empty() || toLocationID.empty() || isBlocked == wasBlocked)
    {
        return false;
    }

    UndoRecord rec;
    rec.actionType = "BLOCK_PATHWAY";
    rec.primaryID = fromLocationID;
    rec.secondaryID = toLocationID;
    rec.newIntVal = isBlocked;
    rec.oldIntVal = wasBlocked;

    return recordAction(rec);
}

bool UndoManager::recordRobotMaintenance(const string &robotID, const string &oldStatus, const string &newStatus)
{
    if (robotID.empty() || oldStatus.empty() || newStatus.empty() || oldStatus == newStatus)
        return false;

    UndoRecord rec;
    rec.actionType = "ROBOT_MAINTENANCE";
    rec.newStrVal = newStatus;
    rec.oldStrVal = oldStatus;
    rec.primaryID = robotID;

    return recordAction(rec);
}

bool UndoManager::canUndo() const
{
    if (historyStack.isEmpty())
        return false;

    return true;
}

int UndoManager::getHistoryCount() const
{
    return historyStack.getSize();
}

const UndoRecord *UndoManager::peekLastAction() const
{
    const UndoRecord *rec;
    rec = historyStack.top();
    return rec;
}

bool UndoManager::undoLastAction(UndoRecord &outRevertedRecord)
{
    if (!canUndo())
        return false;

    UndoRecord *rec = historyStack.top();
    if (!rec)
        return false;
    outRevertedRecord = *rec;
    historyStack.pop();

    if (outRevertedRecord.actionType == "RACK_UPDATE")
    {
        if (!inventory)
        {
            historyStack.push(outRevertedRecord);
            return false;
        }
        bool isUp = false;
        isUp = inventory->updateRackQuantity(outRevertedRecord.primaryID, outRevertedRecord.secondaryID, outRevertedRecord.oldIntVal);
        if (!isUp)
        {
            historyStack.push(outRevertedRecord);
            return false;
        }
    }
    return true;
}

bool UndoManager::discardTopAction()
{
    if (!canUndo())
        return false;
    return historyStack.pop();
}

void UndoManager::clearHistory()
{
    historyStack.clear();
}

void UndoManager::displayUndoHistory() const
{
    if (historyStack.isEmpty())
    {
        UI::printWarning("No reversible operations in undo history.");
        return;
    }

    UI::printSectionBanner("UNDO HISTORY STACK (MOST RECENT FIRST)");
    Stack<UndoRecord> st;
    st = historyStack;

    int entryNum = 1;
    while (!st.isEmpty())
    {
        UndoRecord *rec = st.top();
        if (!rec)
            break;
        if (rec->actionType == "RACK_UPDATE")
        {
            UI::printRow(UI::accent() + "#" + to_string(entryNum) + " [" + rec->actionType + "] " + UI::reset() +
                         "Product: " + UI::bold() + rec->primaryID + UI::reset() +
                         " | Rack: " + rec->secondaryID +
                         " | Old Qty: " + to_string(rec->oldIntVal) +
                         " -> New Qty: " + to_string(rec->newIntVal));
        }
        else if (rec->actionType == "BLOCK_PATHWAY")
        {
            UI::printRow(UI::accent() + "#" + to_string(entryNum) + " [" + rec->actionType + "] " + UI::reset() +
                         "Pathway: " + UI::bold() + rec->primaryID + " <-> " + rec->secondaryID + UI::reset() +
                         " | Previous: " + ((rec->oldIntVal) ? "BLOCKED" : "OPEN") +
                         " -> Current: " + ((rec->newIntVal) ? "BLOCKED" : "OPEN"));
        }
        else
        {
            UI::printRow(UI::accent() + "#" + to_string(entryNum) + " [" + rec->actionType + "] " + UI::reset() +
                         "Robot: " + UI::bold() + rec->primaryID + UI::reset() +
                         " | Old Status: " + rec->oldStrVal +
                         " -> New Status: " + rec->newStrVal);
        }
        entryNum++;
        st.pop();
    }
    UI::drawDecoration('_', '=', '_');
}