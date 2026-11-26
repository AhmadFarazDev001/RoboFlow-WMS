//
//  main.cpp
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//

#include "Library.h"
#include "PickingPlan.h"
#include "ExecutionEngine.h"

//include Hamza's & Ahmed's headers during final merge:
//#include "WarehouseGraph.h"
//#include "RobotFleet.h"
//#include "ChargingManager.h"
//#include "ProductAVL.h"
//#include "OrderManager.h"
//#include "UndoStack.h"

using std::cout;
using std::cin;
using std::string;

int getSafeInt(const string& prompt){
    int val;
    while(true){
        cout << prompt;
        if(cin >> val){
            return val;
        }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid number. Try again.\n";
    }
}

int main(){
    //active plan state held in main for Module 5 & 6
    PickingPlan activePlan;
    OrderNode * activeOrder = nullptr;

    //pointers to team module managers (wire to Hamza's & Ahmed's objects on merge)
    LocalWarehouseGraph * graph = nullptr;
    LocalInventoryTree * inventory = nullptr;
    LocalOrderQueue * orderQueue = nullptr;
    LocalRobotFleet * fleet = nullptr;

    cout << "====================================================\n";
    cout << "  ROBOFLOW WMS - INTELLIGENT WAREHOUSE SYSTEM\n";
    cout << "====================================================\n";

    //load files at startup (locations.txt, pathways.txt, racks.txt, products.txt, inventory.txt, robots.txt, chargers.txt, orders.txt)

    int choice = -1;
    while(choice != 0){
        cout << "\n================ MAIN MENU ================\n";
        cout << "1. Module 1: Warehouse Layout & Pathways\n";
        cout << "2. Module 2: Product Inventory & Racks\n";
        cout << "3. Module 3: Customer Order Management\n";
        cout << "4. Module 4: Robot Fleet Management\n";
        cout << "5. Module 5: Generate Picking Plan & Assign Robot\n";
        cout << "6. Module 6: Execute Assigned Picking Plan\n";
        cout << "7. Module 7: Robot Battery & Charging\n";
        cout << "8. Module 8: Operation History & Undo\n";
        cout << "0. Exit\n";

        choice = getSafeInt("Select option: ");

        if(choice == 1){
            //call Hamza's Module 1 menu; push to UndoStack when blocking a pathway
            cout << "[Module 1: Warehouse Layout & Pathways]\n";
        }else if(choice == 2){
            //call Ahmed's Module 2 menu; push to UndoStack on manual rack quantity update
            cout << "[Module 2: Product Inventory & Racks]\n";
        }else if(choice == 3){
            //call Ahmed's Module 3 menu
            cout << "[Module 3: Customer Order Management]\n";
        }else if(choice == 4){
            //call Hamza's Module 4 menu; push to UndoStack on maintenance toggle
            cout << "[Module 4: Robot Fleet Management]\n";
        }else if(choice == 5){
            if(activePlan.isValid){
                cout << "An active plan for Order " << activePlan.orderID << " already exists. Finish or cancel it in Module 6 first.\n";
                continue;
            }
            if(orderQueue && inventory && fleet && graph){
                if(selectBestRobotAndGeneratePlan(*orderQueue, *inventory, *fleet, *graph, activePlan, activeOrder)){
                    runExecutionSubMenu(activePlan, activeOrder, *fleet, *inventory, *graph, *orderQueue);
                }
            }else{
                cout << "System managers not initialized yet.\n";
            }
        }else if(choice == 6){
            if(!activePlan.isValid || activeOrder == nullptr){
                cout << "No active assigned picking plan. Generate one in Module 5 first.\n";
            }else if(fleet && inventory && graph && orderQueue){
                runExecutionSubMenu(activePlan, activeOrder, *fleet, *inventory, *graph, *orderQueue);
            }
        }else if(choice == 7){
            //call Hamza's Module 7 menu
            cout << "[Module 7: Robot Battery & Charging]\n";
        }else if(choice == 8){
            //call Ahmed's Module 8 menu
            cout << "[Module 8: Operation History & Undo]\n";
        }else if(choice == 0){
            cout << "Shutting down RoboFlow WMS...\n";
        }else{
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}
