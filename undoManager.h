#pragma once

#include "library.h"
#include "Stack.h"
#include "DomainStructs.h"
#include "productAVL.h"

class UndoManager
{
private:
    Stack<UndoRecord> historyStack;
    ProductAVL *inventory;

    bool isValidActionType(const string &actionType) const;

public:
    // Lifecycle & catalog linking
    explicit UndoManager(ProductAVL *productCatalog = nullptr);
    void setProductCatalog(ProductAVL *productCatalog);

    // Push administrative action snapshots
    bool recordAction(const UndoRecord &record);
    bool recordRackUpdate(const string &productID, const string &rackID, int oldQty, int newQty);
    bool recordPathwayBlock(const string &fromLocationID, const string &toLocationID, bool wasBlocked, bool isBlocked);
    bool recordRobotMaintenance(const string &robotID, const string &oldStatus, const string &newStatus);

    // Stack inspection & LIFO rollback
    bool canUndo() const;
    int getHistoryCount() const;
    const UndoRecord *peekLastAction() const;
    bool undoLastAction(UndoRecord &outRevertedRecord);
    void clearHistory();

    // Diagnostics & reporting
    void displayUndoHistory() const;
};

