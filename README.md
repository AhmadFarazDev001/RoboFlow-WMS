# RoboFlow-WMS

Console-based **Intelligent Warehouse Robot & Automated Order Fulfillment System** written in **C++17** using custom, manually implemented data structures (zero STL containers). Built for the CL2001 Data Structures course project (`BCS 3B`).

---

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Core Data Structures & Algorithms](#core-data-structures--algorithms)
- [Modules Implemented](#modules-implemented)
- [Robot Fleet Specifications](#robot-fleet-specifications)
- [Project Structure](#project-structure)
- [Team Contributions & Integration](#team-contributions--integration)
- [Dataset Schema](#dataset-schema)
- [Build & Run](#build--run)
- [Validation & Constraints](#validation--constraints)

---

## Overview

RoboFlow-WMS coordinates warehouse floor topology, rack inventory, incoming customer orders, robot dispatch, multi-trip route planning, charging station queues, and reversible administrative actions through a cross-platform ANSI console interface.

Every core container (`AVLTree`, `SinglyLinkedList`, `Queue`, `Stack`, `MinHeap`, and the adjacency-list `WarehouseGraph`) is written manually with explicit dynamic memory management (`new`/`delete` and destructors). Only standard utilities (`iostream`, `string`, `fstream`, `sstream`, `iomanip`, `cmath`, `climits`, `limits`, `cstdlib`, `thread`, `chrono`) are used via the centralized `library.h` header.

---

## System Architecture

Core business logic and physical infrastructure were authored cleanly without tight cross-module coupling and bridged via adapter interfaces (`WarehouseAdapters`):

```
+-------------------------------------------------------------------+
|                        CLI & Workflow Layer                       |
|            main.cpp  |  InputHelper.h  |  UIStyle / LoadingScreen |
+----------------------------------+--------------------------------+
                                   |
          +------------------------+------------------------+
          v                                                 v
+------------------------+                        +------------------------+
|  Infrastructure Layer  |                        |  Business Logic Layer  |
|  - WarehouseGraph      |                        |  - ProductAVL          |
|  - MinHeap             |                        |  - OrderManager        |
|  - Robot / RobotFleet  |                        |  - UndoManager         |
|  - ChargingManager     |                        |  - DomainStructs       |
+-----------+------------+                        +-----------+------------+
            |                                                 |
            +------------------------+------------------------+
                                     v
                        +-------------------------+
                        | Integration & Execution |
                        |  - WarehouseAdapters    |
                        |  - PickingPlan          |
                        |  - ExecutionEngine      |
                        +-------------------------+
```

---

## Core Data Structures & Algorithms

| Component | Implementation | Application |
| :--- | :--- | :--- |
| **AVL Tree** | Self-balancing BST (`AVLTree<K, V>`) with LL/RR/LR/RL rotations | Product catalog indexed by `productID` with sorted in-order traversal |
| **Singly Linked List** | Generic template (`SinglyLinkedList<T>`) with predicate helpers | Product rack entries, per-order reservations, customer orders, order line items, routes |
| **Queue** | Manual FIFO linked queue (`Queue<T>`) with mid-queue deletion & `enqueueFront` | FCFS pending customer orders (`O(n)` cancellation) and per-station charging wait queues |
| **Stack** | Manual LIFO linked stack (`Stack<T>`) | Reversible administrative action history (`RACK_UPDATE`, `BLOCK_PATHWAY`, `ROBOT_MAINTENANCE`) |
| **Graph (Adjacency List)** | Undirected weighted list of `LocationVertex` & `EdgeNode` structs | Warehouse nodes (`RACK`, `PACKING`, `CHARGING`) and `OPEN`/`BLOCKED` pathways |
| **Min-Heap & Routing** | Custom binary heap (`MinHeap`), BFS, and Dijkstra's algorithm | Reachability checks (`OPEN` edges only) and shortest path routing (`distance` -> `hops` -> smaller `locationID`) |

---

## Modules Implemented

1. **Module 1 — Warehouse Layout & Pathway Management**: Load/create locations (`L##`), add/remove/block/reopen pathways, verify connectivity via BFS, and compute shortest weighted paths via Dijkstra.
2. **Module 2 — Product Inventory & Storage Racks**: AVL-backed product CRUD, valid rack linking (`RK##`), manual stock adjustments, low-stock threshold alerts, and reservation-safe product/rack deletion guards.
3. **Module 3 — Customer Order Management**: File and interactive order entry, duplicate-product quantity merging, FIFO pending queue inspection (`peek`, front-to-rear display), and lifecycle cancellation rules.
4. **Module 4 — Robot Fleet Management**: Linked-list registry of `Picker`, `Carrier`, and `HeavyDuty` subclasses with type capacity validation, battery updates, and task-guarded maintenance toggles.
5. **Module 5 — Intelligent Robot Assignment & Picking Plan**:
   - **Deterministic Rack Allocation**: Reachable racks prioritized by shortest Dijkstra distance -> higher available stock -> smaller `rackID`.
   - **Nearest-Reachable-Rack Visit Ordering**: Greedy next-stop selection from the robot's start location, breaking distance ties by smaller `rackID`.
   - **Multi-Trip Splitting**: Splits stops by whole units whenever carrying capacity would be exceeded, unloading at the nearest `PACKING` station between trips.
   - **Robot Selection Rule**: Evaluates all eligible `Available` robots meeting `battery >= estimatedConsumption + 10.0`, selecting fewest trips -> lowest distance -> lowest `robotID`.
   - **Shortage Rule**: Detects stock shortages prior to assignment, keeps orders `Pending` by default, or lets the operator reserve available stock for partial fulfillment.
6. **Module 6 — Order Picking & Delivery Execution**:
   - Enforces **validate-first, apply-second** per trip (verifies routes are `OPEN`, battery reserve holds, capacity fits, and rack reservations match).
   - Tracks a `visited` flag per stop to guarantee no double stock deduction across stops or cancelled trips.
   - Supports on-the-fly **Recalculate Route** when pathways are blocked mid-execution, plus **Cancel Assignment** (preserves delivered items, returns unstarted orders to queue front).
7. **Module 7 — Battery & Charging Management**: Flags `< 20%` battery robots, routes them to the nearest reachable charging station (`C##`) with a free slot (or FIFO wait queue), restores `100%` on completion, and automatically promotes the next queued robot.
8. **Module 8 — Operation History & Undo Management**: Tracks reversible inventory adjustments, pathway blocks, and robot maintenance changes; rejects inconsistent rollbacks (leaving entries on stack until valid or explicitly discarded).

---

## Robot Fleet Specifications

| Type | Capacity Limit | Battery Rate / Unit | Leg Consumption Formula | Eligibility Rule |
| :--- | :--- | :--- | :--- | :--- |
| **Picker** | `0 < cap <= 15 kg` | `0.20` | `dist * rate * (1 + 0.5 * load / cap)` | Rejects any order containing an item with unit weight `> 5 kg` |
| **Carrier** | `15 <= cap <= 30 kg` | `0.25` | `dist * rate * (1 + 0.5 * load / cap)` | No unit weight restriction |
| **HeavyDuty** | `0 < cap <= 60 kg` | `0.35` | `dist * rate * (1 + 0.5 * load / cap)` | Eligible only if total order weight `> 15 kg` or max unit weight `> 10 kg` |

---

## Project Structure

All headers, implementation files, and sample datasets reside directly in the repository root:

```
.
├── README.md
├── library.h                   # Centralized standard C++ headers and namespace std
├── AVLTree.{h,ipp}             # Self-balancing AVL tree template
├── SinglyLinkedList.{h,ipp}    # Generic singly linked list template
├── Stack.{h,ipp}               # Generic LIFO stack template
├── Queue.{h,ipp}               # Generic FIFO queue template
├── DomainStructs.h             # Shared domain data structs
├── InputHelper.h               # Line-buffered strict input validators
├── LoadingScreen.h             # Horizontal progress bar and ASCII logo renderer
├── UIStyle.{h,cpp}             # Cross-platform UTF-8/ANSI styling & box layout
├── MinHeap.{h,cpp}             # Binary min-heap for Dijkstra shortest path search
├── WarehouseGraph.{h,cpp}      # Weighted adjacency-list graph, BFS, and Dijkstra
├── Robot.{h,cpp}               # Robot base class and Picker/Carrier/HeavyDuty subclasses
├── RobotFleet.{h,cpp}          # Robot fleet registry, assignment, and maintenance
├── ChargingManager.{h,cpp}     # Charging station slots and FIFO waiting queues
├── productAVL.{h,cpp}          # AVL product catalog, racks, and stock reservations
├── orderManager.{h,cpp}        # Order catalog and FCFS pending order queue
├── undoManager.{h,cpp}         # LIFO undo stack with consistency validation
├── PickingPlan.{h,cpp}         # Deterministic rack allocation, trip splitting & scheduling
├── ExecutionEngine.{h,cpp}     # Trip execution, route recalculation & cancellation
├── WarehouseAdapters.{h,cpp}   # Adapter bridges integrating core modules with planner/engine
├── main.cpp                    # Interactive CLI main menu and module controllers
├── locations.txt               # Warehouse nodes (15 locations)
├── pathways.txt                # Weighted connections (24 pathways)
├── racks.txt                   # Rack-to-location mappings (10 racks)
├── products.txt                # Product definitions (12 products)
├── inventory.txt               # Initial rack quantities (24 entries)
├── robots.txt                  # Initial robot fleet (6 robots)
├── chargers.txt                # Charging station capacities (2 stations)
└── orders.txt                  # Customer orders dataset (12 orders)
```

---

## Team Contributions & Integration

| Member | Ownership |
| :--- | :--- |
| **Ahmad Faraz** | Core template structures (`AVLTree`, `SinglyLinkedList`, `Queue`, `Stack`), shared domain structs, Modules 2, 3, and 8 (`productAVL`, `orderManager`, `undoManager`), strict input validation (`InputHelper.h`), central `library.h`, and Master CLI Menu & workflow controllers (`main.cpp`). |
| **Hamza** | Modules 1, 4, and 7 (`MinHeap`, `WarehouseGraph`, `Robot`/`RobotFleet`, `ChargingManager`) along with modular CLI UI utilities (`UIStyle.{h,cpp}`, `LoadingScreen.h`). |
| **Ubaid Afzal** | Modules 5 and 6 (`PickingPlan`, `ExecutionEngine`) and cross-module system integration via `WarehouseAdapters.{h,cpp}`. |

---

## Dataset Schema

All startup data is read externally from `.txt` files inside the working directory (no hardcoded inventory, fleet, topology, or orders). The included sample dataset provides 15 locations (3 `PACKING`, 2 `CHARGING`, 10 `RACK`), 24 pathways, 10 racks, 12 products, 24 rack stock records, 6 robots, 2 charging stations, and 12 orders (covering FCFS behavior, low-battery scenarios, a stock shortage on `O105`, and an `80.0 kg` multi-trip order on `O106`).

| File | Record Format (after optional count header line) | Example |
| :--- | :--- | :--- |
| `locations.txt` | `<LocationID> <RACK\|PACKING\|CHARGING> <Name>` | `L01 PACKING Packing_Station_1` |
| `pathways.txt` | `<LocA> <LocB> <Distance> <OPEN\|BLOCKED>` | `L01 L02 8 OPEN` |
| `racks.txt` | `<RackID> <LocationID>` | `RK01 L05` |
| `products.txt` | `<ProductID> <Name> <UnitWeightKg> <ReorderLevel>` | `P101 Wireless_Mouse 0.5 10` |
| `inventory.txt` | `<ProductID> <RackID> <Quantity>` | `P101 RK01 12` |
| `robots.txt` | `<RobotID> <Type> <LocationID> <Battery%> <MaxCapKg> <Status>` | `RB01 Picker L01 92.0 12.0 Available` |
| `chargers.txt` | `<StationID> <LocationID> <SlotCount>` | `C01 L03 1` |
| `orders.txt` | `<OrderID> <CustomerID> <N> [<ProductID> <Qty> ...]` | `O101 CU01 2 P101 4 P103 5` |

---

## Build & Run

Run commands from the repository root so startup `.txt` data files resolve directly.

### Linux / macOS (`g++` or `clang++`)

```bash
g++ -std=c++17 -Wall -Wextra *.cpp -o roboflow
./roboflow
```

With AddressSanitizer & UndefinedBehaviorSanitizer checks:

```bash
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined *.cpp -o roboflow
./roboflow
```

### Windows (MSVC Developer Command Prompt or MinGW)

```powershell
cl /EHsc /std:c++17 *.cpp /Fe:roboflow.exe
roboflow.exe
```

```powershell
g++ -std=c++17 -Wall -Wextra *.cpp -o roboflow.exe
roboflow.exe
```

---

## Validation & Constraints

- **Entity Prefix Rules**: Locations (`L` + digits), Storage Racks (`RK` + digits), Products (`P` + digits), Robots (`RB` + digits), Charging Stations (`C` + digits), Orders (`O` + digits), Customers (`CU` + digits).
- **Input Safety**: Line-based parsing in `InputHelper.h` validates ranges, decimals, ID prefixes, choice sets, and empty/whitespace input without breaking stream state (`cin`).
- **Build & Leak Audit**: Verified clean compilation under `-Wall -Wextra -Werror` and full interactive execution under `-fsanitize=address,undefined` with zero memory leaks or undefined behavior.

---

Repository: [AhmadFarazDev001/RoboFlow-WMS](https://github.com/AhmadFarazDev001/RoboFlow-WMS)
