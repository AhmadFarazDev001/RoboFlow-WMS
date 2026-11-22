#pragma once

#include "library.h"
#include "DomainStructs.h"

class ProductAVL
{
private:
    AVLTree<string, Product> catalog;
    SinglyLinkedList<RackInfo> validRacks;

public:
    ProductAVL() = default;

    // --- 1. File Loading (Startup) ---
    bool loadRacks(const string &filename);
    bool loadProducts(const string &filename);
    bool loadInventory(const string &filename);

    // --- 2. Rack Lookup Helpers ---
    bool isValidRack(const string &rackID) const;
    string getRackLocation(const string &rackID) const;

    // --- 3. Product Catalog CRUD ---
    bool addProduct(const string &productID,
                    const string &name,
                    double unitWeight,
                    int reorderLevel);
    bool deleteProduct(const string &productID);
    bool productExists(const string &productID) const;
    Product *getProduct(const string &productID);
    const Product *getProduct(const string &productID) const;

    // --- 4. Rack & Stock Management ---
    bool addRackToProduct(const string &productID,
                          const string &rackID,
                          int initialQty);
    bool updateRackQuantity(const string &productID,
                            const string &rackID,
                            int newTotalQty,
                            int *outOldQty = nullptr);
    bool removeEmptyRackFromProduct(const string &productID,
                                    const string &rackID);

    // --- 5. Stock Queries ---
    int getTotalStock(const string &productID) const;
    int getTotalReservedStock(const string &productID) const;
    int getAvailableStock(const string &productID) const;

    // --- 6. Order Reservation & Pick Execution (Modules 3, 5, 6) ---
    bool reserveStock(const string &orderID,
                      const string &productID,
                      int qtyNeeded,
                      SinglyLinkedList<RackAllocation> &outAllocations);
    void releaseOrderReservations(const string &orderID);
    void commitOrderPick(const string &orderID);

    // --- 7. Reporting (Uses AVLTree::traverseInOrder) ---
    void displayAllProducts() const;
    void displayProductDetails(const string &productID) const;
    void displayLowStockAlerts() const;
};
