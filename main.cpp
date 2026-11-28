// -- Written by Ahmad: Interactive console menu and module workflow controllers

#include "library.h"
#include "WarehouseGraph.h"
#include "RobotFleet.h"
#include "ChargingManager.h"
#include "productAVL.h"
#include "orderManager.h"
#include "undoManager.h"
#include "PickingPlan.h"
#include "ExecutionEngine.h"
#include "WarehouseAdapters.h"
#include "LoadingScreen.h"
#include "InputHelper.h"

namespace
{
    void pauseScreen()
    {
        if (cin.eof())
            return;
        cout << '\n';
        UI::printSpaces(UI::getBoxPadding() + 2);
        cout << UI::dim() << ">> Press ENTER to continue..." << UI::reset() << flush;
        string dummy;
        getline(cin, dummy);
    }

    void clearForAction(const string &bannerTitle)
    {
        UI::clearScreen();
        UI::verticalPadding(1);
        UI::printSectionBanner(bannerTitle);
        cout << '\n';
    }

    void drawSubMenuFrame(const string &moduleTitle, const string &moduleSubtitle,
                          const string options[], int count, const string &statusInfo = "")
    {
        UI::clearScreen();
        UI::verticalPadding(2);
        UI::drawDecoration('.', '=', '.');
        UI::drawDecoration('|', ' ', '|');
        UI::printCentered(UI::accent() + moduleTitle + UI::reset(), '|');
        if (!moduleSubtitle.empty())
            UI::printCentered(UI::dim() + moduleSubtitle + UI::reset(), '|');
        UI::drawDecoration('|', ' ', '|');
        UI::drawDecoration('+', '-', '+');
        if (!statusInfo.empty())
        {
            UI::printCentered(statusInfo, '|');
            UI::drawDecoration('|', '-', '|');
        }
        UI::drawDecoration('|', ' ', '|');
        for (int i = 0; i < count; i++)
        {
            UI::printRow(options[i], '|', 5);
        }
        UI::drawDecoration('|', ' ', '|');
        UI::drawDecoration('_', '=', '_');
        cout << '\n';
    }

    int countRobotsByStatus(const RobotFleet &fleet, const string &targetStatus)
    {
        int cnt = 0;
        RobotNode *curr = fleet.getHead();
        while (curr)
        {
            if (curr->data->getStatus() == targetStatus)
                cnt++;
            curr = curr->next;
        }
        return cnt;
    }

    int countTotalRobots(const RobotFleet &fleet)
    {
        int cnt = 0;
        RobotNode *curr = fleet.getHead();
        while (curr)
        {
            cnt++;
            curr = curr->next;
        }
        return cnt;
    }

    EdgeNode *findEdge(WarehouseGraph &graph, const string &locA, const string &locB)
    {
        LocationVertex *vA = graph.searchLocation(locA);
        if (!vA)
            return nullptr;
        EdgeNode *e = vA->edgeHead;
        while (e)
        {
            if (e->targetLocationID == locB)
                return e;
            e = e->next;
        }
        return nullptr;
    }

    // -- Module 1: warehouse layout and pathway menu
    void menuModule1(WarehouseGraph &graph, UndoManager &undoMgr)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View Warehouse Map          - Show all locations and OPEN/BLOCKED pathways",
            UI::accent() + "[ 2 ]" + UI::reset() + "  Add New Location            - Register a new RACK, PACKING, or CHARGING node",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Add Pathway                 - Connect two existing locations with weighted distance",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Remove Pathway              - Permanently delete a pathway from the graph",
            UI::accent() + "[ 5 ]" + UI::reset() + "  Block Pathway               - Temporarily close a pathway (saved to Undo stack)",
            UI::accent() + "[ 6 ]" + UI::reset() + "  Reopen Blocked Pathway      - Restore a blocked pathway back to OPEN status",
            UI::accent() + "[ 7 ]" + UI::reset() + "  Search Location by ID       - Look up location type and name by ID",
            UI::accent() + "[ 8 ]" + UI::reset() + "  Check Reachability (BFS)    - Test if two locations have an OPEN connection",
            UI::accent() + "[ 9 ]" + UI::reset() + "  Shortest Route (Dijkstra)   - Calculate shortest OPEN weighted path & distance",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 1 : WAREHOUSE LAYOUT & PATHWAY MANAGEMENT",
                "Undirected Weighted Graph  |  BFS Reachability  |  Dijkstra Shortest Path",
                opts, 10,
                "Active Locations: " + UI::accent() + to_string(graph.getLocationCount()) + UI::reset());

            int ch = InputHelper::readIntInRange("Enter your choice (0-9): ", 0, 9);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                graph.displayAllLocations();
            }
            else if (ch == 2)
            {
                clearForAction("MODULE 1 : ADD NEW WAREHOUSE LOCATION");
                string id = InputHelper::readEntityID("Enter Location ID (e.g., L16): ", "L");
                string types[3] = {"RACK", "PACKING", "CHARGING"};
                string type = InputHelper::readChoice("Enter Location Type (RACK / PACKING / CHARGING): ", types, 3);
                string name = InputHelper::readNonEmptyToken("Enter Location Name (no spaces, e.g., Zone_E): ");

                if (graph.addLocation(id, type, name))
                    UI::printSuccess("Location " + id + " (" + type + ", " + name + ") added to warehouse graph.");
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 1 : ADD NEW PATHWAY");
                string locA = InputHelper::readEntityID("Enter First Location ID (e.g., L01): ", "L");
                string locB = InputHelper::readEntityID("Enter Second Location ID (e.g., L05): ", "L");
                int dist = InputHelper::readIntInRange("Enter Pathway Distance in meters (> 0): ", 1, 100000);

                if (graph.addPathway(locA, locB, dist, "OPEN"))
                    UI::printSuccess("Pathway " + locA + " <-> " + locB + " (" + to_string(dist) + "m, OPEN) created.");
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 1 : REMOVE PATHWAY PERMANENTLY");
                string locA = InputHelper::readEntityID("Enter First Location ID: ", "L");
                string locB = InputHelper::readEntityID("Enter Second Location ID: ", "L");
                if (graph.removePathway(locA, locB))
                    UI::printSuccess("Pathway " + locA + " <-> " + locB + " permanently deleted.");
            }
            else if (ch == 5)
            {
                clearForAction("MODULE 1 : BLOCK PATHWAY");
                string locA = InputHelper::readEntityID("Enter First Location ID: ", "L");
                string locB = InputHelper::readEntityID("Enter Second Location ID: ", "L");
                EdgeNode *edge = findEdge(graph, locA, locB);
                if (!edge)
                {
                    UI::printError("Pathway between " + locA + " and " + locB + " does not exist.");
                }
                else if (edge->status == "BLOCKED")
                {
                    UI::printWarning("Pathway " + locA + " <-> " + locB + " is already BLOCKED.");
                }
                else if (graph.setPathwayStatus(locA, locB, "BLOCKED"))
                {
                    undoMgr.recordPathwayBlock(locA, locB, false, true);
                    UI::printSuccess("Pathway " + locA + " <-> " + locB + " blocked (recorded in Undo stack).");
                }
            }
            else if (ch == 6)
            {
                clearForAction("MODULE 1 : REOPEN BLOCKED PATHWAY");
                string locA = InputHelper::readEntityID("Enter First Location ID: ", "L");
                string locB = InputHelper::readEntityID("Enter Second Location ID: ", "L");
                EdgeNode *edge = findEdge(graph, locA, locB);
                if (!edge)
                {
                    UI::printError("Pathway between " + locA + " and " + locB + " does not exist.");
                }
                else if (edge->status == "OPEN")
                {
                    UI::printWarning("Pathway " + locA + " <-> " + locB + " is already OPEN.");
                }
                else if (graph.setPathwayStatus(locA, locB, "OPEN"))
                {
                    UI::printSuccess("Pathway " + locA + " <-> " + locB + " reopened.");
                }
            }
            else if (ch == 7)
            {
                clearForAction("MODULE 1 : SEARCH LOCATION BY ID");
                string id = InputHelper::readEntityID("Enter Location ID to look up: ", "L");
                LocationVertex *v = graph.searchLocation(id);
                if (!v)
                    UI::printError("Location " + id + " was not found.");
                else
                    UI::printSuccess("Found -> " + v->locationID + " | Type: " + v->type + " | Name: " + v->name);
            }
            else if (ch == 8)
            {
                clearForAction("MODULE 1 : CHECK CONNECTIVITY (BFS)");
                string startID = InputHelper::readEntityID("Enter Start Location ID: ", "L");
                string endID = InputHelper::readEntityID("Enter Destination Location ID: ", "L");
                if (!graph.searchLocation(startID) || !graph.searchLocation(endID))
                    UI::printError("One or both locations do not exist.");
                else if (graph.isConnectedBFS(startID, endID))
                    UI::printSuccess(startID + " and " + endID + " ARE connected via OPEN pathways (BFS).");
                else
                    UI::printWarning("No OPEN connection exists between " + startID + " and " + endID + ".");
            }
            else if (ch == 9)
            {
                clearForAction("MODULE 1 : FIND SHORTEST ROUTE (DIJKSTRA)");
                string startID = InputHelper::readEntityID("Enter Start Location ID: ", "L");
                string endID = InputHelper::readEntityID("Enter Destination Location ID: ", "L");
                if (!graph.searchLocation(startID) || !graph.searchLocation(endID))
                {
                    UI::printError("Start or destination location does not exist.");
                }
                else
                {
                    RouteResult route;
                    if (graph.findShortestRouteDijkstra(startID, endID, route))
                    {
                        UI::printSuccess("Optimal route found via Dijkstra:");
                        cout << '\n';
                        graph.displayRoute(route);
                    }
                    else
                    {
                        UI::printWarning("No open route exists between " + startID + " and " + endID + ".");
                    }
                }
            }
            pauseScreen();
        }
    }

    // -- Module 2: product inventory and storage rack menu
    void menuModule2(ProductAVL &catalog, UndoManager &undoMgr)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View Product Catalog        - List all products sorted by ID (AVL In-Order)",
            UI::accent() + "[ 2 ]" + UI::reset() + "  Inspect Product & Racks     - Show product details and rack stock breakdown",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Add New Product             - Insert a new unique product into AVL catalog",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Delete Product              - Remove unreserved product while balancing AVL tree",
            UI::accent() + "[ 5 ]" + UI::reset() + "  Assign Rack to Product      - Link a valid storage rack with initial stock",
            UI::accent() + "[ 6 ]" + UI::reset() + "  Adjust Rack Stock Quantity  - Manual stock adjustment (saved to Undo stack)",
            UI::accent() + "[ 7 ]" + UI::reset() + "  Remove Empty Rack Entry     - Unlink a rack with 0 total and 0 reserved units",
            UI::accent() + "[ 8 ]" + UI::reset() + "  Check Stock Totals          - Calculate total, reserved, and available stock",
            UI::accent() + "[ 9 ]" + UI::reset() + "  Low-Stock Reorder Alerts    - List products whose available stock < reorder level",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 2 : PRODUCT INVENTORY & STORAGE RACK MANAGEMENT",
                "Height-Balanced AVL Tree  |  Nested Rack & Per-Order Reservation Lists",
                opts, 10);

            int ch = InputHelper::readIntInRange("Enter your choice (0-9): ", 0, 9);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                catalog.displayAllProducts();
            }
            else if (ch == 2)
            {
                clearForAction("MODULE 2 : INSPECT PRODUCT & STORAGE RACKS");
                string pid = InputHelper::readEntityID("Enter Product ID (e.g., P101): ", "P");
                catalog.displayProductDetails(pid);
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 2 : ADD NEW PRODUCT");
                string pid = InputHelper::readEntityID("Enter New Product ID (e.g., P113): ", "P");
                string name = InputHelper::readNonEmptyToken("Enter Product Name (no spaces, e.g., Optical_Sensor): ");
                double weight = InputHelper::readDoubleInRange("Enter Unit Weight in kg (> 0): ", 0.01, 500.0, true);
                int reorder = InputHelper::readIntInRange("Enter Reorder Level (>= 0): ", 0, 100000);

                if (catalog.addProduct(pid, name, weight, reorder))
                    UI::printSuccess("Product " + pid + " (" + name + ") inserted into AVL catalog.");
                else
                    UI::printError("Could not add product (duplicate ID or invalid values).");
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 2 : DELETE PRODUCT");
                string pid = InputHelper::readEntityID("Enter Product ID to delete: ", "P");
                if (catalog.deleteProduct(pid))
                    UI::printSuccess("Product " + pid + " deleted from AVL tree.");
                else
                    UI::printError("Deletion rejected (" + pid + " not found or holds active reservations).");
            }
            else if (ch == 5)
            {
                clearForAction("MODULE 2 : ASSIGN RACK TO PRODUCT");
                string pid = InputHelper::readEntityID("Enter Product ID: ", "P");
                string rid = InputHelper::readEntityID("Enter Rack ID (e.g., RK01): ", "RK");
                int qty = InputHelper::readIntInRange("Enter Initial Quantity (>= 0): ", 0, 100000);

                if (catalog.addRackToProduct(pid, rid, qty))
                    UI::printSuccess("Rack " + rid + " linked to " + pid + " with " + to_string(qty) + " units.");
                else
                    UI::printError("Failed to add rack (invalid product/rack ID or rack already linked).");
            }
            else if (ch == 6)
            {
                clearForAction("MODULE 2 : MANUAL RACK QUANTITY ADJUSTMENT");
                string pid = InputHelper::readEntityID("Enter Product ID: ", "P");
                string rid = InputHelper::readEntityID("Enter Rack ID: ", "RK");
                int newQty = InputHelper::readIntInRange("Enter New Total Quantity (>= 0): ", 0, 100000);

                int oldQty = -1;
                if (catalog.updateRackQuantity(pid, rid, newQty, &oldQty))
                {
                    if (undoMgr.recordRackUpdate(pid, rid, oldQty, newQty))
                        UI::printSuccess("Rack " + rid + " for " + pid + " updated: " + to_string(oldQty) +
                                         " -> " + to_string(newQty) + " (recorded in Undo stack).");
                    else
                        UI::printInfo("Quantity remained " + to_string(newQty) + " (no change recorded in Undo stack).");
                }
                else
                {
                    UI::printError("Adjustment rejected (missing product/rack or new total < reserved quantity).");
                }
            }
            else if (ch == 7)
            {
                clearForAction("MODULE 2 : REMOVE EMPTY RACK ENTRY");
                string pid = InputHelper::readEntityID("Enter Product ID: ", "P");
                string rid = InputHelper::readEntityID("Enter Empty Rack ID to remove: ", "RK");
                if (catalog.removeEmptyRackFromProduct(pid, rid))
                    UI::printSuccess("Empty rack " + rid + " unlinked from product " + pid + ".");
                else
                    UI::printError("Removal rejected (rack must have 0 total and 0 reserved units).");
            }
            else if (ch == 8)
            {
                clearForAction("MODULE 2 : STOCK SUMMARY QUERY");
                string pid = InputHelper::readEntityID("Enter Product ID: ", "P");
                if (!catalog.productExists(pid))
                {
                    UI::printError("Product " + pid + " does not exist.");
                }
                else
                {
                    int tot = catalog.getTotalStock(pid);
                    int res = catalog.getTotalReservedStock(pid);
                    int avail = catalog.getAvailableStock(pid);
                    UI::printSuccess(pid + " -> Total: " + to_string(tot) +
                                     " | Reserved: " + to_string(res) +
                                     " | Available: " + to_string(avail));
                }
            }
            else if (ch == 9)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                catalog.displayLowStockAlerts();
            }
            pauseScreen();
        }
    }

    // -- Module 3: customer order management menu
    void menuModule3(OrderManager &orderMgr, ProductAVL &catalog, RobotFleet &fleet,
                     PickingPlan &activePlan, OrderNode *&activeOrder)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View All Customer Orders    - List every order with customer ID & status",
            UI::accent() + "[ 2 ]" + UI::reset() + "  View Pending Queue (FCFS)   - Inspect pending orders from front to rear",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Peek Front & Queue Size     - Show oldest pending order without removing it",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Search Order Details        - View full line items and quantities by Order ID",
            UI::accent() + "[ 5 ]" + UI::reset() + "  Create New Customer Order   - Add order with multiple items & enqueue at rear",
            UI::accent() + "[ 6 ]" + UI::reset() + "  Add Items to Pending Order  - Append or merge products in an existing Pending order",
            UI::accent() + "[ 7 ]" + UI::reset() + "  Cancel Customer Order       - Cancel Pending/Reserved/Assigned order & free stock",
            UI::accent() + "[ 8 ]" + UI::reset() + "  View Pending & Completed    - Display Pending orders alongside Completed orders",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 3 : CUSTOMER ORDER MANAGEMENT",
                "Master Order Linked List  |  Manual FCFS Pending Queue",
                opts, 9,
                "Total Orders: " + UI::accent() + to_string(orderMgr.getTotalOrderCount()) + UI::reset() +
                "  |  Pending in Queue: " + UI::warnColor() + to_string(orderMgr.getPendingOrderCount()) + UI::reset());

            int ch = InputHelper::readIntInRange("Enter your choice (0-8): ", 0, 8);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                orderMgr.displayAllOrders();
            }
            else if (ch == 2)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                orderMgr.displayPendingQueue();
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 3 : PEEK FRONT ORDER & QUEUE SIZE");
                int qCount = orderMgr.getPendingOrderCount();
                string frontID = orderMgr.peekNextPendingOrder();
                if (frontID.empty())
                    UI::printWarning("Pending order queue is empty (Size: 0).");
                else
                    UI::printSuccess("Next Pending Order (Front): " + frontID +
                                     " | Queue Size: " + to_string(qCount));
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 3 : SEARCH ORDER DETAILS");
                string oid = InputHelper::readEntityID("Enter Order ID (e.g., O101): ", "O");
                orderMgr.displayOrderDetails(oid);
            }
            else if (ch == 5)
            {
                clearForAction("MODULE 3 : CREATE NEW CUSTOMER ORDER");
                string oid = InputHelper::readEntityID("Enter New Order ID (e.g., O113): ", "O");
                if (orderMgr.orderExists(oid))
                {
                    UI::printError("Duplicate Order ID rejected: " + oid + " already exists.");
                    pauseScreen();
                    continue;
                }
                string cid = InputHelper::readEntityID("Enter Customer ID (e.g., CU13): ", "CU");

                SinglyLinkedList<OrderItem> itemList;
                bool adding = true;
                while (adding)
                {
                    string pid = InputHelper::readEntityID("Enter Product ID to add: ", "P");
                    if (!catalog.productExists(pid))
                    {
                        UI::printError("Product " + pid + " does not exist in catalog.");
                    }
                    else
                    {
                        int qty = InputHelper::readIntInRange("Enter Quantity (> 0): ", 1, 100000);
                        OrderItem it;
                        it.productID = pid;
                        it.requiredQty = qty;
                        itemList.insertBack(it);
                        UI::printSuccess("Added " + pid + " x " + to_string(qty) + " to draft order.");
                    }
                    adding = InputHelper::readYesNo("Add another product to this order? (y/n): ");
                }

                if (itemList.isEmpty())
                    UI::printWarning("Order cancelled because no valid products were added.");
                else if (orderMgr.createOrder(oid, cid, itemList, false))
                    UI::printSuccess("Order " + oid + " created and placed at rear of Pending queue.");
                else
                    UI::printError("Failed to create order " + oid + ".");
            }
            else if (ch == 6)
            {
                clearForAction("MODULE 3 : ADD ITEMS TO EXISTING PENDING ORDER");
                string oid = InputHelper::readEntityID("Enter Pending Order ID: ", "O");
                Order *ord = orderMgr.getOrder(oid);
                if (!ord)
                {
                    UI::printError("Order " + oid + " not found.");
                }
                else if (ord->status != "Pending")
                {
                    UI::printError("Items can only be added to Pending orders (current status: " + ord->status + ").");
                }
                else
                {
                    bool adding = true;
                    while (adding)
                    {
                        string pid = InputHelper::readEntityID("Enter Product ID to add: ", "P");
                        if (!catalog.productExists(pid))
                        {
                            UI::printError("Product " + pid + " does not exist in catalog.");
                        }
                        else
                        {
                            int qty = InputHelper::readIntInRange("Enter Quantity (> 0): ", 1, 100000);
                            OrderItem *existing = ord->items.findIf([&](const OrderItem &it)
                                                                    { return it.productID == pid; });
                            if (existing)
                            {
                                existing->requiredQty += qty;
                                UI::printSuccess("Merged " + pid + " quantity -> new total: " + to_string(existing->requiredQty) + ".");
                            }
                            else
                            {
                                OrderItem item;
                                item.productID = pid;
                                item.requiredQty = qty;
                                ord->items.insertBack(item);
                                UI::printSuccess("Added " + pid + " x " + to_string(qty) + " to order " + oid + ".");
                            }
                        }
                        adding = InputHelper::readYesNo("Add another product to " + oid + "? (y/n): ");
                    }
                }
            }
            else if (ch == 7)
            {
                clearForAction("MODULE 3 : CANCEL CUSTOMER ORDER");
                string oid = InputHelper::readEntityID("Enter Order ID to cancel: ", "O");
                Order *ord = orderMgr.getOrder(oid);
                if (!ord)
                {
                    UI::printError("Order " + oid + " not found.");
                }
                else if (ord->status == "Partially Fulfilled" || ord->status == "Completed" || ord->status == "Cancelled")
                {
                    UI::printError("Cancellation rejected: order status is '" + ord->status + "'.");
                }
                else
                {
                    bool startedTrip = false;
                    const ListNode<OrderItem> *itNode = ord->items.getHead();
                    while (itNode)
                    {
                        if (itNode->data.deliveredQty > 0)
                        {
                            startedTrip = true;
                            break;
                        }
                        itNode = itNode->next;
                    }

                    if (startedTrip)
                    {
                        UI::printError("Cancellation rejected: deliveries have already begun for this order.");
                    }
                    else
                    {
                        string assignedBot = ord->assignedRobotID;
                        if (orderMgr.cancelOrder(oid))
                        {
                            if (!assignedBot.empty())
                            {
                                fleet.releaseRobot(assignedBot);
                                ord->assignedRobotID = "";
                            }
                            if (activePlan.isValid && activePlan.orderID == oid)
                            {
                                activePlan.clear();
                                activeOrder = nullptr;
                            }
                            UI::printSuccess("Order " + oid + " cancelled, reservations released, and assigned robot freed.");
                        }
                        else
                        {
                            UI::printError("Could not cancel order " + oid + ".");
                        }
                    }
                }
            }
            else if (ch == 8)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                orderMgr.displayOrdersByStatus("Pending");
                cout << '\n';
                orderMgr.displayOrdersByStatus("Completed");
            }
            pauseScreen();
        }
    }

    // -- Module 4: robot fleet management menu
    void menuModule4(RobotFleet &fleet, const WarehouseGraph &graph, UndoManager &undoMgr)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View Entire Robot Fleet     - Display all robots with locations, battery & status",
            UI::accent() + "[ 2 ]" + UI::reset() + "  View Available Robots Only  - Filter fleet to show robots ready for assignment",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Register New Robot          - Add Picker, Carrier, or HeavyDuty robot to fleet",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Search Robot by ID          - Inspect a single robot's metrics & active task",
            UI::accent() + "[ 5 ]" + UI::reset() + "  Update Robot Battery Level  - Manually adjust robot battery percentage (0-100%)",
            UI::accent() + "[ 6 ]" + UI::reset() + "  Toggle Maintenance Status   - Set Unavailable/Available (saved to Undo stack)",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 4 : ROBOT FLEET MANAGEMENT",
                "OOP Inheritance (Picker / Carrier / HeavyDuty)  |  Linked List Fleet",
                opts, 7,
                "Available: " + UI::okColor() + to_string(countRobotsByStatus(fleet, "Available")) + UI::reset() +
                " / Total Fleet: " + UI::accent() + to_string(countTotalRobots(fleet)) + UI::reset());

            int ch = InputHelper::readIntInRange("Enter your choice (0-6): ", 0, 6);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                fleet.displayAllRobots();
            }
            else if (ch == 2)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                RobotNode *curr = fleet.getHead();
                bool any = false;
                UI::printSectionBanner("AVAILABLE ROBOTS READY FOR ASSIGNMENT");
                while (curr)
                {
                    Robot *b = curr->data;
                    if (b->getStatus() == "Available")
                    {
                        any = true;
                        ostringstream batStr, capStr;
                        batStr << fixed << setprecision(1) << b->getBattery();
                        capStr << fixed << setprecision(1) << b->getMaxCapacity();
                        UI::printRow(UI::accent() + b->getID() + UI::reset() +
                                     " | " + b->getType() +
                                     " | Loc: " + b->getLocation() +
                                     " | Bat: " + (b->getBattery() < 20.0 ? UI::errColor() : UI::okColor()) + batStr.str() + "%" + UI::reset() +
                                     " | Cap: " + capStr.str() + " kg");
                    }
                    curr = curr->next;
                }
                if (!any)
                    UI::printRow(UI::warnColor() + "No robots are currently Available." + UI::reset());
                UI::drawDecoration('_', '=', '_');
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 4 : REGISTER NEW ROBOT");
                string id = InputHelper::readEntityID("Enter New Robot ID (e.g., RB07): ", "RB");
                string types[3] = {"Picker", "Carrier", "HeavyDuty"};
                string type = InputHelper::readChoice("Enter Type (Picker / Carrier / HeavyDuty): ", types, 3);
                string locID = InputHelper::readEntityID("Enter Initial Location ID (e.g., L01): ", "L");
                double bat = InputHelper::readDoubleInRange("Enter Battery % (0.0 - 100.0): ", 0.0, 100.0, true);

                double minCap = (type == "Carrier") ? 15.0 : 0.1;
                double maxCap = (type == "Picker") ? 15.0 : ((type == "Carrier") ? 30.0 : 60.0);
                string capHint = (type == "Carrier") ? "15 - 30" : ((type == "Picker") ? "0.1 - 15" : "0.1 - 60");
                double cap = InputHelper::readDoubleInRange("Enter Max Capacity kg (" + capHint + "): ", minCap, maxCap, true);

                if (fleet.registerRobot(id, type, locID, bat, cap, &graph, "Available"))
                    UI::printSuccess("Robot " + id + " (" + type + ") registered at " + locID + ".");
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 4 : SEARCH ROBOT BY ID");
                string id = InputHelper::readEntityID("Enter Robot ID to search: ", "RB");
                Robot *b = fleet.searchRobot(id);
                if (!b)
                {
                    UI::printError("Robot " + id + " not found.");
                }
                else
                {
                    ostringstream batStr, capStr;
                    batStr << fixed << setprecision(1) << b->getBattery();
                    capStr << fixed << setprecision(1) << b->getMaxCapacity();

                    UI::printSectionBanner("ROBOT PROFILE : " + b->getID());
                    UI::printRow("Type      : " + b->getType());
                    UI::printRow("Location  : " + b->getLocation());
                    UI::printRow("Battery   : " + batStr.str() + "%");
                    UI::printRow("Capacity  : " + capStr.str() + " kg");
                    UI::printRow("Status    : " + b->getStatus() +
                                 (b->getAssignedTask().empty() ? "" : " (Task: " + b->getAssignedTask() + ")"));
                    UI::drawDecoration('_', '=', '_');
                }
            }
            else if (ch == 5)
            {
                clearForAction("MODULE 4 : UPDATE ROBOT BATTERY LEVEL");
                string id = InputHelper::readEntityID("Enter Robot ID: ", "RB");
                Robot *b = fleet.searchRobot(id);
                if (!b)
                {
                    UI::printError("Robot " + id + " not found.");
                }
                else
                {
                    double bat = InputHelper::readDoubleInRange("Enter New Battery % (0.0 - 100.0): ", 0.0, 100.0, true);
                    b->setBattery(bat);
                    ostringstream batStr;
                    batStr << fixed << setprecision(1) << b->getBattery();
                    UI::printSuccess("Battery for " + id + " updated to " + batStr.str() + "%.");
                    if (b->getBattery() < 20.0)
                        UI::printWarning("Robot " + id + " is below 20% and requires charging.");
                }
            }
            else if (ch == 6)
            {
                clearForAction("MODULE 4 : TOGGLE MAINTENANCE STATUS");
                string id = InputHelper::readEntityID("Enter Robot ID: ", "RB");
                string allowed[2] = {"MAINT", "ACTIVE"};
                string mode = InputHelper::readChoice("Enter Mode (MAINT = Set Unavailable, ACTIVE = Set Available): ", allowed, 2);
                bool toMaint = (mode == "MAINT");
                string oldStatus;
                if (fleet.setMaintenanceStatus(id, toMaint, oldStatus))
                {
                    string newStatus = toMaint ? "Unavailable" : "Available";
                    undoMgr.recordRobotMaintenance(id, oldStatus, newStatus);
                    UI::printSuccess("Robot " + id + " transitioned " + oldStatus + " -> " + newStatus + " (recorded in Undo stack).");
                }
            }
            pauseScreen();
        }
    }

    // -- Module 7: robot battery and charging management menu
    void menuModule7(ChargingManager &chargingMgr, RobotFleet &fleet, const WarehouseGraph &graph)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View Charging Stations      - Show slot occupancy and FIFO waiting queues",
            UI::accent() + "[ 2 ]" + UI::reset() + "  Find Low-Battery Robots     - Identify all robots with battery below 20%",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Send Robot to Charge        - Route robot (< 20%) to nearest reachable station",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Complete Charging           - Set battery to 100%, free slot & promote queue",
            UI::accent() + "[ 5 ]" + UI::reset() + "  Remove from Charging Queue  - Withdraw a waiting robot & return it to Available",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 7 : ROBOT BATTERY & CHARGING MANAGEMENT",
                "Charging Slot Allocation  |  Nearest Station Routing  |  Per-Station FIFO Queues",
                opts, 6);

            int ch = InputHelper::readIntInRange("Enter your choice (0-5): ", 0, 5);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                chargingMgr.displayChargingStations();
            }
            else if (ch == 2)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                RobotNode *curr = fleet.getHead();
                bool any = false;
                UI::printSectionBanner("LOW-BATTERY ROBOTS (< 20.0%)");
                while (curr)
                {
                    Robot *b = curr->data;
                    if (b->getBattery() < 20.0)
                    {
                        any = true;
                        ostringstream batStr;
                        batStr << fixed << setprecision(1) << b->getBattery();
                        UI::printRow(UI::accent() + b->getID() + UI::reset() +
                                     " | " + b->getType() +
                                     " | Loc: " + b->getLocation() +
                                     " | Bat: " + UI::errColor() + batStr.str() + "%" + UI::reset() +
                                     " | Status: " + b->getStatus());
                    }
                    curr = curr->next;
                }
                if (!any)
                    UI::printRow(UI::okColor() + "All robots currently have >= 20.0% battery." + UI::reset());
                UI::drawDecoration('_', '=', '_');
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 7 : SEND ROBOT TO CHARGING STATION");
                string rid = InputHelper::readEntityID("Enter Robot ID to charge (e.g., RB02): ", "RB");
                chargingMgr.sendRobotToCharge(rid, fleet, graph);
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 7 : COMPLETE ROBOT CHARGING");
                string sid = InputHelper::readEntityID("Enter Charging Station ID (e.g., C01): ", "C");
                string rid = InputHelper::readEntityID("Enter Charging Robot ID: ", "RB");
                chargingMgr.completeCharging(sid, rid, fleet);
            }
            else if (ch == 5)
            {
                clearForAction("MODULE 7 : REMOVE ROBOT FROM WAITING QUEUE");
                string rid = InputHelper::readEntityID("Enter Queued Robot ID to remove: ", "RB");
                chargingMgr.removeFromChargingQueue(rid, fleet);
            }
            pauseScreen();
        }
    }

    // -- Module 8: operation history and undo management menu
    void menuModule8(UndoManager &undoMgr, WarehouseGraph &graph, RobotFleet &fleet)
    {
        const string opts[] = {
            UI::accent() + "[ 1 ]" + UI::reset() + "  View Full Undo History      - Display all recorded reversible actions (LIFO)",
            UI::accent() + "[ 2 ]" + UI::reset() + "  Peek Latest Operation       - Inspect the top undo record without popping",
            UI::accent() + "[ 3 ]" + UI::reset() + "  Undo Most Recent Operation  - Validate consistency & revert the top operation",
            UI::accent() + "[ 4 ]" + UI::reset() + "  Discard Top Undo Entry      - Pop and discard top record without reverting",
            UI::warnColor() + "[ 0 ]" + UI::reset() + "  Back to Main Menu           - Return to main control console"
        };

        while (true)
        {
            drawSubMenuFrame(
                "MODULE 8 : OPERATION HISTORY & UNDO MANAGEMENT",
                "Custom LIFO Stack  |  Consistency-Checked Rollback",
                opts, 5,
                "Recorded Undo Entries: " + UI::accent() + to_string(undoMgr.getHistoryCount()) + UI::reset());

            int ch = InputHelper::readIntInRange("Enter your choice (0-4): ", 0, 4);
            if (ch == 0)
                return;

            if (ch == 1)
            {
                UI::clearScreen();
                UI::verticalPadding(1);
                undoMgr.displayUndoHistory();
            }
            else if (ch == 2)
            {
                clearForAction("MODULE 8 : PEEK LATEST OPERATION");
                const UndoRecord *top = undoMgr.peekLastAction();
                if (!top)
                {
                    UI::printWarning("Undo stack is empty.");
                }
                else
                {
                    string summary = "Top Record -> [" + top->actionType + "] Primary: " + top->primaryID;
                    if (!top->secondaryID.empty())
                        summary += " | Secondary: " + top->secondaryID;
                    if (top->actionType == "RACK_UPDATE")
                        summary += " | Old Qty: " + to_string(top->oldIntVal) + " -> New Qty: " + to_string(top->newIntVal);
                    else if (top->actionType == "BLOCK_PATHWAY")
                        summary += " | Old Blocked: " + to_string(top->oldIntVal) + " -> New Blocked: " + to_string(top->newIntVal);
                    else if (top->actionType == "ROBOT_MAINTENANCE")
                        summary += " | Old Status: " + top->oldStrVal + " -> New Status: " + top->newStrVal;
                    UI::printInfo(summary);
                }
            }
            else if (ch == 3)
            {
                clearForAction("MODULE 8 : UNDO MOST RECENT ELIGIBLE OPERATION");
                const UndoRecord *top = undoMgr.peekLastAction();
                if (!top)
                {
                    UI::printWarning("Undo stack is empty.");
                }
                else if (top->actionType == "ROBOT_MAINTENANCE")
                {
                    Robot *bot = fleet.searchRobot(top->primaryID);
                    if (!bot)
                    {
                        UI::printError("Undo rejected: Robot " + top->primaryID + " no longer exists (entry kept on stack).");
                    }
                    else
                    {
                        string st = bot->getStatus();
                        if (st == "Assigned" || st == "Charging" || st == "Waiting for Charging")
                        {
                            UI::printError("Undo rejected: Robot " + top->primaryID + " is currently '" + st + "' (entry kept on stack).");
                        }
                        else
                        {
                            UndoRecord reverted;
                            if (undoMgr.undoLastAction(reverted))
                            {
                                bot->setStatus(reverted.oldStrVal);
                                UI::printSuccess("Reverted ROBOT_MAINTENANCE: " + reverted.primaryID + " restored to '" + reverted.oldStrVal + "'.");
                            }
                        }
                    }
                }
                else if (top->actionType == "BLOCK_PATHWAY")
                {
                    UndoRecord reverted;
                    if (undoMgr.undoLastAction(reverted))
                    {
                        string restoredStatus = reverted.oldIntVal ? "BLOCKED" : "OPEN";
                        if (graph.setPathwayStatus(reverted.primaryID, reverted.secondaryID, restoredStatus))
                            UI::printSuccess("Reverted BLOCK_PATHWAY: " + reverted.primaryID + " <-> " + reverted.secondaryID + " restored to " + restoredStatus + ".");
                    }
                }
                else
                {
                    UndoRecord reverted;
                    if (undoMgr.undoLastAction(reverted))
                    {
                        UI::printSuccess("Reverted RACK_UPDATE: " + reverted.primaryID + " at " + reverted.secondaryID +
                                         " restored to " + to_string(reverted.oldIntVal) + " units.");
                    }
                    else
                    {
                        UI::printError("Undo rejected: Restoring quantity " + to_string(top->oldIntVal) +
                                       " violates reserved stock or rack no longer exists (entry kept on stack).");
                    }
                }
            }
            else if (ch == 4)
            {
                clearForAction("MODULE 8 : DISCARD TOP UNDO ENTRY");
                if (undoMgr.discardTopAction())
                    UI::printSuccess("Top undo record discarded without modifying warehouse state.");
                else
                    UI::printWarning("Undo stack is already empty.");
            }
            pauseScreen();
        }
    }
}

int main()
{
    UI::initConsole();

    // -- Initialize core module managers
    WarehouseGraph warehouseGraph;
    RobotFleet robotFleet;
    ChargingManager chargingManager;
    ProductAVL productCatalog;
    OrderManager orderManager(&productCatalog);
    UndoManager undoManager(&productCatalog);

    // -- Run startup loading progress screen while reading data files
    LoadingScreen::renderStep(15, "INITIALIZING WAREHOUSE GRAPH & PATHWAYS...", 140);
    bool okLoc = warehouseGraph.loadLocationsAndPathways("locations.txt", "pathways.txt", "racks.txt");

    LoadingScreen::renderStep(35, "REGISTERING AUTONOMOUS ROBOT FLEET...", 140);
    bool okBot = robotFleet.loadRobots("robots.txt", &warehouseGraph);

    LoadingScreen::renderStep(55, "CONFIGURING BATTERY CHARGING STATIONS...", 140);
    bool okChg = chargingManager.loadChargers("chargers.txt", warehouseGraph);

    LoadingScreen::renderStep(75, "INDEXING AVL PRODUCT CATALOG & RACK INVENTORY...", 140);
    bool okRck = productCatalog.loadRacks("racks.txt");
    bool okPrd = productCatalog.loadProducts("products.txt");
    bool okInv = productCatalog.loadInventory("inventory.txt");

    LoadingScreen::renderStep(92, "LOADING CUSTOMER ORDERS INTO FCFS QUEUE...", 140);
    bool okOrd = orderManager.loadOrders("orders.txt", false);

    LoadingScreen::renderStep(100, "ROBOFLOW WMS READY!", 200);

    // -- Initialize adapter objects for planning and execution modules
    WarehouseGraphAdapter graphAdapter(warehouseGraph);
    InventoryTreeAdapter inventoryAdapter(productCatalog);
    RobotFleetAdapter fleetAdapter(robotFleet);
    OrderQueueAdapter orderQueueAdapter(orderManager);

    PickingPlan activePlan;
    OrderNode *activeOrder = nullptr;

    LocalWarehouseGraph *graph = &graphAdapter;
    LocalInventoryTree *inventory = &inventoryAdapter;
    LocalOrderQueue *orderQueue = &orderQueueAdapter;
    LocalRobotFleet *fleet = &fleetAdapter;

    const string mainMenuOptions[] = {
        UI::accent() + "[ 1 ]" + UI::reset() + "  WAREHOUSE LAYOUT & PATHWAYS     - Manage locations, open/block paths & shortest routes",
        UI::accent() + "[ 2 ]" + UI::reset() + "  PRODUCT INVENTORY & RACKS       - Manage AVL catalog, rack stock & low-stock alerts",
        UI::accent() + "[ 3 ]" + UI::reset() + "  CUSTOMER ORDER MANAGEMENT       - Create, view, queue (FCFS) & cancel customer orders",
        UI::accent() + "[ 4 ]" + UI::reset() + "  ROBOT FLEET MANAGEMENT          - Register robots, update battery & toggle maintenance",
        UI::okColor() + "[ 5 ]" + UI::reset() + "  ASSIGN ROBOT & PICKING PLAN     - Auto-select best robot & plan multi-trip routes",
        UI::okColor() + "[ 6 ]" + UI::reset() + "  EXECUTE ASSIGNED PICKING PLAN   - Execute trips, recalculate routes or cancel task",
        UI::accent() + "[ 7 ]" + UI::reset() + "  BATTERY & CHARGING STATIONS     - Route low-battery robots & manage charging queues",
        UI::accent() + "[ 8 ]" + UI::reset() + "  OPERATION HISTORY & UNDO        - Inspect undo stack & revert eligible changes",
        UI::errColor() + "[ 0 ]" + UI::reset() + "  EXIT ROBOFLOW WMS               - Save nothing extra & shut down console cleanly"
    };

    int choice = -1;
    while (choice != 0)
    {
        UI::clearScreen();
        UI::verticalPadding(2);
        UI::drawDecoration('.', '=', '.');
        UI::drawDecoration('|', ' ', '|');
        LoadingScreen::printLogo();
        UI::drawDecoration('|', ' ', '|');
        UI::printCentered(UI::bold() + "INTELLIGENT WAREHOUSE ROBOT & AUTOMATED ORDER FULFILLMENT SYSTEM" + UI::reset(), '|');
        UI::printCentered(UI::dim() + "C++ CONSOLE EDITION" + UI::reset(), '|');
        UI::drawDecoration('|', ' ', '|');
        UI::drawDecoration('+', '-', '+');
        UI::printCentered(UI::accent() + "M A I N   M E N U" + UI::reset(), '|');

        string activeTag = activePlan.isValid
            ? (UI::okColor() + activePlan.orderID + " (" + activePlan.assignedRobotID + ")" + UI::reset())
            : (UI::dim() + "None" + UI::reset());

        string summaryBar = "Locations: " + UI::accent() + to_string(warehouseGraph.getLocationCount()) + UI::reset() +
                            "   |   Pending Orders: " + UI::warnColor() + to_string(orderManager.getPendingOrderCount()) + UI::reset() +
                            "   |   Avail Bots: " + UI::okColor() + to_string(countRobotsByStatus(robotFleet, "Available")) +
                            "/" + to_string(countTotalRobots(robotFleet)) + UI::reset() +
                            "   |   Active Plan: " + activeTag +
                            "   |   Undo: " + UI::infoColor() + to_string(undoManager.getHistoryCount()) + UI::reset();

        UI::printCentered(summaryBar, '|');
        UI::drawDecoration('|', '-', '|');
        UI::drawDecoration('|', ' ', '|');

        for (int i = 0; i < 9; i++)
        {
            UI::printRow(mainMenuOptions[i], '|', 5);
        }

        UI::drawDecoration('|', ' ', '|');
        UI::drawDecoration('_', '=', '_');
        cout << '\n';

        if (!(okLoc && okBot && okChg && okRck && okPrd && okInv && okOrd))
        {
            UI::printWarning("Some startup data files were missing in the current working directory.");
            cout << '\n';
        }

        choice = InputHelper::readIntInRange("ENTER YOUR CHOICE (0-8): ", 0, 8);

        if (choice == 1)
        {
            menuModule1(warehouseGraph, undoManager);
        }
        else if (choice == 2)
        {
            menuModule2(productCatalog, undoManager);
        }
        else if (choice == 3)
        {
            menuModule3(orderManager, productCatalog, robotFleet, activePlan, activeOrder);
        }
        else if (choice == 4)
        {
            menuModule4(robotFleet, warehouseGraph, undoManager);
        }
        else if (choice == 5)
        {
            UI::clearScreen();
            UI::verticalPadding(2);
            UI::printSectionBanner("MODULE 5 : INTELLIGENT ROBOT ASSIGNMENT & PICKING PLAN",
                                     "Nearest-Reachable-Rack  |  Multi-Trip Splitting  |  10% Safety Reserve");
            if (activePlan.isValid)
            {
                UI::printWarning("Active plan for Order " + activePlan.orderID + " already exists. Execute or cancel it in Module 6 first.");
                pauseScreen();
                continue;
            }
            inventoryAdapter.refresh();
            fleetAdapter.refresh();
            orderQueueAdapter.refreshPending();
            if (orderQueue && inventory && fleet && graph)
            {
                if (selectBestRobotAndGeneratePlan(*orderQueue, *inventory, *fleet, *graph, activePlan, activeOrder))
                {
                    runExecutionSubMenu(activePlan, activeOrder, *fleet, *inventory, *graph, *orderQueue);
                    fleetAdapter.syncToReal();
                    orderQueueAdapter.syncOrderToReal(activeOrder);
                    if (!activePlan.isValid)
                        activeOrder = nullptr;
                }
            }
            else
            {
                UI::printError("Warehouse adapters are not initialized.");
            }
            pauseScreen();
        }
        else if (choice == 6)
        {
            UI::clearScreen();
            UI::verticalPadding(2);
            UI::printSectionBanner("MODULE 6 : ORDER PICKING & DELIVERY EXECUTION",
                                     "Validate First Then Apply  |  Route Recalculation  |  Trip Execution");
            if (!activePlan.isValid || activeOrder == nullptr)
            {
                UI::printWarning("No active assigned picking plan. Generate one in Module 5 first.");
            }
            else if (fleet && inventory && graph && orderQueue)
            {
                inventoryAdapter.refresh();
                fleetAdapter.syncFromReal();
                runExecutionSubMenu(activePlan, activeOrder, *fleet, *inventory, *graph, *orderQueue);
                fleetAdapter.syncToReal();
                orderQueueAdapter.syncOrderToReal(activeOrder);
                if (!activePlan.isValid)
                    activeOrder = nullptr;
            }
            pauseScreen();
        }
        else if (choice == 7)
        {
            menuModule7(chargingManager, robotFleet, warehouseGraph);
        }
        else if (choice == 8)
        {
            menuModule8(undoManager, warehouseGraph, robotFleet);
        }
        else if (choice == 0)
        {
            LoadingScreen::renderStep(100, "SHUTTING DOWN ROBOFLOW WMS...", 220);
            cout << '\n';
            UI::printSuccess("Session closed cleanly. Goodbye!");
            cout << '\n';
        }
    }

    return 0;
}
