// -- Written by Ahmad: Product catalog, rack storage, and stock reservation manager
#pragma once

#include "library.h"
#include "DomainStructs.h"

class ProductAVL
{
private:
    AVLTree<string, Product> catalog;
    SinglyLinkedList<RackInfo> validRacks;

public:
    // -- Initializes an empty product inventory manager
    ProductAVL() = default;

    // -- Loads valid rack-to-location mappings from file
    bool loadRacks(const string &filename);
    // -- Loads product catalog definitions from file
    bool loadProducts(const string &filename);
    // -- Loads initial rack quantities for products from file
    bool loadInventory(const string &filename);

    // -- Checks whether a rack ID exists in valid racks
    bool isValidRack(const string &rackID) const;
    // -- Returns the location ID mapped to a rack
    string getRackLocation(const string &rackID) const;

    // -- Adds a new product to the catalog tree
    bool addProduct(const string &productID,
                    const string &name,
                    double unitWeight,
                    int reorderLevel);
    // -- Deletes a product with zero stock and no reservations
    bool deleteProduct(const string &productID);
    // -- Checks whether a product ID exists in the catalog
    bool productExists(const string &productID) const;
    // -- Returns a modifiable pointer to a product record
    Product *getProduct(const string &productID);
    // -- Returns a read-only pointer to a product record
    const Product *getProduct(const string &productID) const;

    // -- Links a valid storage rack with initial quantity to a product
    bool addRackToProduct(const string &productID,
                          const string &rackID,
                          int initialQty);
    // -- Updates total quantity on a product rack and optionally returns old quantity
    bool updateRackQuantity(const string &productID,
                            const string &rackID,
                            int newTotalQty,
                            int *outOldQty = nullptr);
    // -- Removes an unreserved empty rack entry from a product
    bool removeEmptyRackFromProduct(const string &productID,
                                    const string &rackID);

    // -- Returns quantity reserved by a specific order for a product
    int getReservedStockForOrder(const string &orderID, const string &productID) const;
    // -- Calculates total physical quantity across all racks for a product
    int getTotalStock(const string &productID) const;
    // -- Calculates total reserved quantity across all racks for a product
    int getTotalReservedStock(const string &productID) const;
    // -- Calculates total unreserved quantity available for a product
    int getAvailableStock(const string &productID) const;

    // -- Reserves stock across available racks for an order and outputs allocations
    bool reserveStock(const string &orderID,
                      const string &productID,
                      int qtyNeeded,
                      SinglyLinkedList<RackAllocation> &outAllocations);
    // -- Reserves a specific quantity on one named rack for an order
    bool reserveRackStock(const string &orderID,
                          const string &productID,
                          const string &rackID,
                          int qty);
    // -- Releases a partial reservation from a single rack stop
    bool releaseSingleStopReservation(const string &orderID,
                                      const string &productID,
                                      const string &rackID,
                                      int qty);
    // -- Deducts picked stock and its reservation from a specific rack
    bool commitPickedStock(const string &orderID,
                           const string &productID,
                           const string &rackID,
                           int qty);
    // -- Releases all stock reservations associated with an order
    void releaseOrderReservations(const string &orderID);
    // -- Commits all remaining reserved picks associated with an order
    void commitOrderPick(const string &orderID);

    // -- Returns a read-only reference to the product AVL catalog
    const AVLTree<string, Product> &getCatalog() const { return catalog; }

    // -- Displays all products sorted by product ID
    void displayAllProducts() const;
    // -- Displays detailed info and rack breakdown for a single product
    void displayProductDetails(const string &productID) const;
    // -- Displays products whose available stock is below reorder threshold
    void displayLowStockAlerts() const;
};
