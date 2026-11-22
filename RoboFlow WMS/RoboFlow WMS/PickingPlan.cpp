//
//  PickingPlan.cpp
//  RoboFlow WMS
//
//  Created by Ubaid Afzal on 20/11/2026.
//
#include "Library.h"
#include "PickingPlan.h"

using std::cout;
using  std::cin;
using std::string;

AllocationList::~AllocationList() {
    clearAllocationList(*this);
}
void clearAllocationList(AllocationList& list) {
    AllocationNode* current = list.head;
    while (current != nullptr) {
        AllocationNode* nodeToDelete = current;
        current = current->next;
        delete nodeToDelete;
    }

    list.head = nullptr;
    list.tail = nullptr;
    list.totalAllocated = 0;
    list.isFullyAllocated = false;
}

void clearCandidateList(CandidateRack*& head) {
    while (head != nullptr) {
        CandidateRack* nodeToDelete = head;
        head = head->next;
        delete nodeToDelete;
    }
    head = nullptr;
}
CandidateRack * bestcandidate(CandidateRack * r1, CandidateRack * r2){
    if (r1 == nullptr) return r2;
    if (r2 == nullptr) return r1;
    if(r1->distance < r2->distance){
        return r1;
    }
    if(r1->availableQty > r2->availableQty){
        return r1;
    }
    return r2;
}

bool allocateRacksForProduct(
    const std::string& prodID,
    int reqQty,
    const std::string& refLocationID,
    const LocalInventoryTree& inventory,
    const LocalWarehouseGraph& graph,
    AllocationList& outList
                             ){
    if(reqQty <= 0 || refLocationID.empty()){
        return false;
    }
    ProductNode* temp = inventory.searchProduct(prodID);
    if (temp == nullptr) {
        return false;
    }
    RackEntry* temprack = temp->rackHead;
    
    CandidateList candidatelist;
   
    while(temprack){
        int availableQty = temprack->totalQty - temprack->reservedQty;
            
            string rackLocationID = graph.getRackLocation(temprack->rackID);
            int shortestdistance = graph.getShortestDistance(refLocationID , rackLocationID);
        
            //if the rack is unreachable
            if(shortestdistance == -1){
                temprack = temprack->next;
                continue;
            }
            
            //check the available qty
        if(availableQty <= 0){
            temprack = temprack->next;
            continue;
        }
        CandidateRack * candidaterack = nullptr;
        //check tha available qty is qualified
        candidaterack = new CandidateRack(temprack->rackID, rackLocationID, availableQty, shortestdistance);
        if(candidatelist.head == nullptr){
            candidatelist.head = candidaterack;
            candidatelist.tail = candidaterack;
            candidatelist.tail->next = nullptr;
        }else{
            candidatelist.tail->next = candidaterack;
            candidatelist.tail = candidaterack;
            candidatelist.tail->next = nullptr;
        }
        temprack = temprack->next;
    }
    //now candidatelist has full
    CandidateRack * tempcandidaterack = candidatelist.head;
    if (tempcandidaterack == nullptr) {
        outList.isFullyAllocated = false;
        return false;
    }
    AllocationList allocatedlist;
    AllocationNode * tempallocatednode = nullptr;
    int remainingQty = reqQty;
    int totalQty = reqQty;
    
    while(true){
        if(remainingQty == 0){
            break;
        }
        if(tempcandidaterack->used){
            if (tempcandidaterack->next != nullptr) {
                tempcandidaterack = tempcandidaterack->next;
                continue;
            }
            break;
        }
        //if a single rack can fulfill the reqQty
        bool hassinglerack = false;
        for (CandidateRack* it = tempcandidaterack->next; it != nullptr; it = it->next) {
            if(!it->used && it->availableQty == remainingQty){
                hassinglerack = true;
                tempallocatednode = new AllocationNode(it->rackID, it->locationID, it->availableQty, it->distance);
                if(allocatedlist.head == nullptr){
                    allocatedlist.head = tempallocatednode;
                    allocatedlist.tail = tempallocatednode;
                    allocatedlist.tail->next = nullptr;
                }else{
                    allocatedlist.tail->next = tempallocatednode;
                    allocatedlist.tail = tempallocatednode;
                    allocatedlist.tail->next = nullptr;
                }
                it->used = true;
                remainingQty -= it->availableQty;
                break;
            }
        }
        if (hassinglerack || remainingQty == 0) {
            if (remainingQty == 0) break;
            continue;
        }
        
        //check best rack
        CandidateRack * bestrack = nullptr;
        CandidateRack * checkrack = candidatelist.head;
        while(checkrack){
            if(!checkrack->used){
                bestrack = bestcandidate(bestrack, checkrack);
            }
            checkrack = checkrack->next;
        }
        if (bestrack == nullptr) {
            break;
        }
        int takeQty = (bestrack->availableQty < remainingQty) ? bestrack->availableQty : remainingQty;
        tempallocatednode = new AllocationNode(bestrack->rackID, bestrack->locationID, takeQty, bestrack->distance);
        remainingQty -= takeQty;
        bestrack->used = true;
        if(allocatedlist.head == nullptr){
            allocatedlist.head = tempallocatednode;
            allocatedlist.tail = tempallocatednode;
            allocatedlist.tail->next = nullptr;
        }else{
            allocatedlist.tail->next = tempallocatednode;
            allocatedlist.tail = tempallocatednode;
            allocatedlist.tail->next = nullptr;
        }
    }
    outList.head = allocatedlist.head;
    outList.tail = allocatedlist.tail;
    outList.totalAllocated = totalQty - remainingQty;
    outList.isFullyAllocated = (remainingQty == 0);
    allocatedlist.head = nullptr;
    allocatedlist.tail = nullptr;
    clearCandidateList(candidatelist.head);
    return outList.isFullyAllocated;
}


