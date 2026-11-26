#pragma once
#include "WarehouseGraph.h"
WarehouseGraph::WarehouseGraph() : vertexHead(nullptr), vertexCount(0) {}
WarehouseGraph::~WarehouseGraph()
{
    LocationVertex *v = vertexHead;
    while (v != nullptr)
    {
        EdgeNode *e = v->edgeHead;
        while (e != nullptr)
        {
            EdgeNode *nextEdge = e->next;
            delete e;
            e = nextEdge;
        }
        LocationVertex *nextVertex = v->next;
        delete v;
        v = nextVertex;
    }
}
// check krna k ID L se start ho aur agy sirf digits hun
bool WarehouseGraph::isValidLocationID(const std::string &id) const
{
    if (id.length() < 2 || id[0] != 'L')
        return false;

    for (std::size_t i = 1; i < id.length(); i++)
    {
        if (id[i] < '0' || id[i] > '9')
            return false;
    }
    return true;
}

bool WarehouseGraph::isValidLocationType(const std::string &type) const
{
    return (type == "RACK" || type == "PACKING" || type == "CHARGING");
}
// ye true hoga agr line ma sirf number likha ho like 15
bool WarehouseGraph::isNumberLine(const std::string &line) const
{
    bool hasDigit = false;
    for (std::size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];
        if (c >= '0' && c <= '9')
            hasDigit = true;
        else if (c != ' ' && c != '\t' && c != '\r')
            return false;
    }
    return hasDigit;
}
// start aur end se extra spaces remove krny k liye
std::string WarehouseGraph::trimText(const std::string &text) const
{
    int start = 0;
    int end = (int)text.length() - 1;

    while (start <= end && (text[start] == ' ' || text[start] == '\t' || text[start] == '\r'))
        start++;

    while (end >= start && (text[end] == ' ' || text[end] == '\t' || text[end] == '\r'))
        end--;

    if (start > end)
        return "";

    return text.substr(start, end - start + 1);
}
int WarehouseGraph::getVertexIndex(const std::string &id) const
{
    LocationVertex *curr = vertexHead;
    int idx = 0;
    while (curr != nullptr)
    {
        if (curr->locationID == id)
            return idx;
        curr = curr->next;
        idx++;
    }
    return -1;
}
LocationVertex *WarehouseGraph::getVertexAt(int index) const
{
    LocationVertex *curr = vertexHead;
    int i = 0;
    while (curr != nullptr)
    {
        if (i == index)
            return curr;
        curr = curr->next;
        i++;
    }
    return nullptr;
}
void WarehouseGraph::addEdgeOneWay(LocationVertex *from, const std::string &toID, int dist, const std::string &status)
{
    EdgeNode *node = new EdgeNode(toID, dist, status);
    if (from->edgeHead == nullptr)
    {
        from->edgeHead = node;
        return;
    }

    EdgeNode *curr = from->edgeHead;
    while (curr->next != nullptr)
        curr = curr->next;
    curr->next = node;
}

bool WarehouseGraph::removeEdgeOneWay(LocationVertex *from, const std::string &toID)
{
    EdgeNode *curr = from->edgeHead;
    EdgeNode *prev = nullptr;
    while (curr != nullptr)
    {
        if (curr->targetLocationID == toID)
        {
            if (prev == nullptr)
                from->edgeHead = curr->next;
            else
                prev->next = curr->next;
            delete curr;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

bool WarehouseGraph::addLocation(const std::string &id, const std::string &type, const std::string &name)
{
    if (!isValidLocationID(id))
    {
        std::cout << "Invalid location ID: " << id << std::endl;
        return false;
    }

    if (!isValidLocationType(type))
    {
        std::cout << "Invalid location type: " << type << std::endl;
        return false;
    }

    if (searchLocation(id) != nullptr)
    {
        std::cout << "Location already exists: " << id << std::endl;
        return false;
    }

    // new location ko list k end pr add krna
    LocationVertex *newNode = new LocationVertex(id, type, name);
    if (vertexHead == nullptr)
    {
        vertexHead = newNode;
    }
    else
    {
        LocationVertex *curr = vertexHead;
        while (curr->next != nullptr)
            curr = curr->next;
        curr->next = newNode;
    }

    vertexCount++;
    return true;
}

LocationVertex *WarehouseGraph::searchLocation(const std::string &id) const
{
    LocationVertex *curr = vertexHead;
    while (curr != nullptr)
    {
        if (curr->locationID == id)
            return curr;
        curr = curr->next;
    }
    return nullptr;
}

bool WarehouseGraph::addRackMapping(const std::string &rackID, const std::string &locID)
{
    if (searchLocation(locID) == nullptr)
    {
        std::cout << "Location not found for rack " << rackID << std::endl;
        return false;
    }

    if (rackMappings.exists(rackID))
    {
        std::cout << "Rack already mapped: " << rackID << std::endl;
        return false;
    }

    rackMappings.insertBack(rackID, locID);
    return true;
}

std::string WarehouseGraph::getLocationForRack(const std::string &rackID) const
{
    return rackMappings.getValue(rackID);
}

bool WarehouseGraph::addPathway(const std::string &locA, const std::string &locB, int dist, const std::string &status)
{
    if (locA == locB || dist <= 0)
    {
        std::cout << "Invalid pathway details." << std::endl;
        return false;
    }

    if (status != "OPEN" && status != "BLOCKED")
    {
        std::cout << "Status must be OPEN or BLOCKED." << std::endl;
        return false;
    }

    LocationVertex *vA = searchLocation(locA);
    LocationVertex *vB = searchLocation(locB);
    if (vA == nullptr || vB == nullptr)
    {
        std::cout << "Location not found for pathway." << std::endl;
        return false;
    }

    // duplicate check
    EdgeNode *check = vA->edgeHead;
    while (check != nullptr)
    {
        if (check->targetLocationID == locB)
        {
            std::cout << "Pathway already exists: " << locA << " - " << locB << std::endl;
            return false;
        }
        check = check->next;
    }

    // undirected graph hai so dono sides pr add kry gy
    addEdgeOneWay(vA, locB, dist, status);
    addEdgeOneWay(vB, locA, dist, status);
    return true;
}

bool WarehouseGraph::removePathway(const std::string &locA, const std::string &locB)
{
    LocationVertex *vA = searchLocation(locA);
    LocationVertex *vB = searchLocation(locB);
    if (vA == nullptr || vB == nullptr)
    {
        std::cout << "Location not found." << std::endl;
        return false;
    }

    bool okA = removeEdgeOneWay(vA, locB);
    bool okB = removeEdgeOneWay(vB, locA);

    if (!okA || !okB)
    {
        std::cout << "Pathway not found between " << locA << " and " << locB << std::endl;
        return false;
    }
    return true;
}

bool WarehouseGraph::setPathwayStatus(const std::string &locA, const std::string &locB, const std::string &status)
{
    if (status != "OPEN" && status != "BLOCKED")
    {
        std::cout << "Status must be OPEN or BLOCKED." << std::endl;
        return false;
    }

    LocationVertex *vA = searchLocation(locA);
    LocationVertex *vB = searchLocation(locB);
    if (vA == nullptr || vB == nullptr)
    {
        std::cout << "Location not found." << std::endl;
        return false;
    }

    EdgeNode *eA = vA->edgeHead;
    while (eA != nullptr && eA->targetLocationID != locB)
        eA = eA->next;

    EdgeNode *eB = vB->edgeHead;
    while (eB != nullptr && eB->targetLocationID != locA)
        eB = eB->next;

    if (eA == nullptr || eB == nullptr)
    {
        std::cout << "Pathway not found." << std::endl;
        return false;
    }

    eA->status = status;
    eB->status = status;
    return true;
}

bool WarehouseGraph::loadLocationsAndPathways(const std::string &locFile, const std::string &pathFile, const std::string &racksFile)
{
    std::string line;

    // 1 locations file read krna
    std::ifstream fLoc(locFile);
    if (!fLoc.is_open())
    {
        std::cout << "Cannot open " << locFile << std::endl;
        return false;
    }

    // agr first line ma sirf count ho to skip kr do
    std::getline(fLoc, line);
    if (!isNumberLine(line))
    {
        fLoc.clear();
        fLoc.seekg(0);
    }

    std::string id, type, name;
    while (fLoc >> id >> type)
    {
        std::getline(fLoc, name);
        name = trimText(name);
        if (name.empty())
            name = id;
        addLocation(id, type, name);
    }
    fLoc.close();

    // 2 pathways file read krna
    std::ifstream fPath(pathFile);
    if (!fPath.is_open())
    {
        std::cout << "Cannot open " << pathFile << std::endl;
        return false;
    }

    std::getline(fPath, line);
    if (!isNumberLine(line))
    {
        fPath.clear();
        fPath.seekg(0);
    }

    std::string locA, locB, status;
    int dist;
    while (fPath >> locA >> locB >> dist)
    {
        std::getline(fPath, status);
        status = trimText(status);
        if (status.empty())
            status = "OPEN";
        addPathway(locA, locB, dist, status);
    }
    fPath.close();

    // 3 racks file read krna
    std::ifstream fRack(racksFile);
    if (!fRack.is_open())
    {
        std::cout << "Cannot open " << racksFile << std::endl;
        return false;
    }

    std::getline(fRack, line);
    if (!isNumberLine(line))
    {
        fRack.clear();
        fRack.seekg(0);
    }

    std::string rackID, locID;
    while (fRack >> rackID >> locID)
    {
        addRackMapping(rackID, locID);
    }
    fRack.close();

    return true;
}

bool WarehouseGraph::isConnectedBFS(const std::string &startID, const std::string &endID) const
{
    int startIdx = getVertexIndex(startID);
    int endIdx = getVertexIndex(endID);

    if (startIdx == -1 || endIdx == -1)
        return false;
    if (startIdx == endIdx)
        return true;

    bool *visited = new bool[vertexCount];
    for (int i = 0; i < vertexCount; i++)
        visited[i] = false;

    IntQueue q;
    visited[startIdx] = true;
    q.enqueue(startIdx);

    bool found = false;
    while (!q.isEmpty())
    {
        int currIdx = q.dequeue();
        if (currIdx == endIdx)
        {
            found = true;
            break;
        }

        LocationVertex *v = getVertexAt(currIdx);
        EdgeNode *edge = v->edgeHead;
        while (edge != nullptr)
        {
            // sirf OPEN pathways check krny hain
            if (edge->status == "OPEN")
            {
                int nextIdx = getVertexIndex(edge->targetLocationID);
                if (nextIdx != -1 && !visited[nextIdx])
                {
                    visited[nextIdx] = true;
                    q.enqueue(nextIdx);
                }
            }
            edge = edge->next;
        }
    }

    delete[] visited;
    return found;
}

bool WarehouseGraph::findShortestRouteDijkstra(const std::string &startID, const std::string &endID, RouteResult &outRoute) const
{
    outRoute.clear();

    int startIdx = getVertexIndex(startID);
    int endIdx = getVertexIndex(endID);
    if (startIdx == -1 || endIdx == -1)
        return false;

    if (startIdx == endIdx)
    {
        outRoute.path.insertFront(startID);
        outRoute.totalDistance = 0;
        outRoute.hopCount = 1;
        outRoute.reachable = true;
        return true;
    }

    const int INF = 1000000000;
    bool *visited = new bool[vertexCount];
    int *dist = new int[vertexCount];
    int *hops = new int[vertexCount];
    int *parent = new int[vertexCount];

    for (int i = 0; i < vertexCount; i++)
    {
        visited[i] = false;
        dist[i] = INF;
        hops[i] = INF;
        parent[i] = -1;
    }

    MinHeap pq(vertexCount);
    dist[startIdx] = 0;
    hops[startIdx] = 1;
    pq.push(0, 1, startIdx);

    while (!pq.isEmpty())
    {
        HeapNode curr = pq.pop();
        int u = curr.index;

        if (visited[u])
            continue;
        visited[u] = true;

        LocationVertex *vU = getVertexAt(u);
        EdgeNode *edge = vU->edgeHead;

        while (edge != nullptr)
        {
            if (edge->status == "OPEN")
            {
                int v = getVertexIndex(edge->targetLocationID);
                if (v != -1 && !visited[v])
                {
                    int newDist = dist[u] + edge->distance;
                    int newHops = hops[u] + 1;

                    // tie break: pehly chota distance phr kam hops phr choti location ID
                    bool better = false;
                    if (newDist < dist[v])
                    {
                        better = true;
                    }
                    else if (newDist == dist[v])
                    {
                        if (newHops < hops[v])
                        {
                            better = true;
                        }
                        else if (newHops == hops[v] && parent[v] != -1)
                        {
                            LocationVertex *currParent = getVertexAt(parent[v]);
                            if (vU->locationID < currParent->locationID)
                                better = true;
                        }
                    }

                    if (better)
                    {
                        dist[v] = newDist;
                        hops[v] = newHops;
                        parent[v] = u;
                        pq.push(newDist, newHops, v);
                    }
                }
            }
            edge = edge->next;
        }
    }

    // agr end tak rasta mila to path wapas trace kr lo
    if (dist[endIdx] < INF)
    {
        outRoute.reachable = true;
        outRoute.totalDistance = dist[endIdx];
        outRoute.hopCount = hops[endIdx];

        int curr = endIdx;
        while (curr != -1)
        {
            outRoute.path.insertFront(getVertexAt(curr)->locationID);
            curr = parent[curr];
        }
    }

    delete[] visited;
    delete[] dist;
    delete[] hops;
    delete[] parent;

    return outRoute.reachable;
}

void WarehouseGraph::displayAllLocations() const
{
    if (vertexHead == nullptr)
    {
        std::cout << "No locations found" << std::endl;
        return;
    }

    std::cout << "\nWarehouse Locations:" << std::endl;
    LocationVertex *v = vertexHead;
    while (v != nullptr)
    {
        std::cout << v->locationID << " (" << v->type << ", " << v->name << ") -> ";
        EdgeNode *e = v->edgeHead;
        if (e == nullptr)
        {
            std::cout << "None";
        }
        else
        {
            while (e != nullptr)
            {
                std::cout << e->targetLocationID << "(" << e->distance << "m)";
                if (e->status == "BLOCKED")
                    std::cout << "[BLOCKED]";
                if (e->next != nullptr)
                    std::cout << ", ";
                e = e->next;
            }
        }
        std::cout << std::endl;
        v = v->next;
    }
}

void WarehouseGraph::displayRoute(const RouteResult &route) const
{
    if (!route.reachable || route.path.isEmpty())
    {
        std::cout << "No route available." << std::endl;
        return;
    }

    ListNode *curr = route.path.getHead();
    while (curr != nullptr)
    {
        std::cout << curr->data;
        if (curr->next != nullptr)
            std::cout << " -> ";
        curr = curr->next;
    }
    std::cout << " (Distance: " << route.totalDistance << "m)" << std::endl;
}
