// -- Written by Ahmad: Stack-based undo history manager for administrative actions
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

    // -- Checks whether an action type string is eligible for undo
    bool isValidActionType(const string &actionType) const;

public:
    // -- Constructs the undo manager with an optional product catalog pointer
    explicit UndoManager(ProductAVL *productCatalog = nullptr);
    // -- Attaches the product inventory catalog for rack rollbacks
    void setProductCatalog(ProductAVL *productCatalog);

    // -- Validates and pushes a generic undo record onto the stack
    bool recordAction(const UndoRecord &record);
    // -- Records a product rack quantity modification for later rollback
    bool recordRackUpdate(const string &productID, const string &rackID, int oldQty, int newQty);
    // -- Records a warehouse pathway block or unblock event
    bool recordPathwayBlock(const string &fromLocationID, const string &toLocationID, bool wasBlocked, bool isBlocked);
    // -- Records a robot maintenance status transition
    bool recordRobotMaintenance(const string &robotID, const string &oldStatus, const string &newStatus);

    // -- Checks whether any actions are available to undo
    bool canUndo() const;
    // -- Returns total count of recorded actions in the stack
    int getHistoryCount() const;
    // -- Returns a read-only pointer to the most recent undo record
    const UndoRecord *peekLastAction() const;
    // -- Pops the latest undo record and applies rollback when applicable
    bool undoLastAction(UndoRecord &outRevertedRecord);
    // -- Pops and discards the top undo entry without reverting state
    bool discardTopAction();
    // -- Clears all recorded entries from the undo stack
    void clearHistory();

    // -- Displays all recorded reversible actions in LIFO order
    void displayUndoHistory() const;
};

