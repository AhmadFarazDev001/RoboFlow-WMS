#pragma once
#include "library.h"

struct EdgeNode
{
    std::string targetLocationID;
    int distance;
    std::string status; // OPEN ya BLOCKED
    EdgeNode *next;

    EdgeNode(const std::string &target, int dist, const std::string &st = "OPEN")
        : targetLocationID(target), distance(dist), status(st), next(nullptr) {}
};

struct LocationVertex
{
    std::string locationID;
    std::string type; // RACK, PACKING, CHARGING
    std::string name;
    EdgeNode *edgeHead;
    LocationVertex *next;

    LocationVertex(const std::string &id, const std::string &t, const std::string &n)
        : locationID(id), type(t), name(n), edgeHead(nullptr), next(nullptr) {}
};

struct RouteResult
{
    LinkedList path;
    int totalDistance;
    int hopCount;
    bool reachable;

    RouteResult() : totalDistance(0), hopCount(0), reachable(false) {}
    void clear()
    {
        path.clear();
        totalDistance = 0;
        hopCount = 0;
        reachable = false;
    }
};

class WarehouseGraph
{
private:
    LocationVertex *vertexHead;
    PairList rackMappings;
    int vertexCount;

    bool isValidLocationID(const std::string &id) const;
    bool isValidLocationType(const std::string &type) const;
    bool isNumberLine(const std::string &line) const;
    std::string trimText(const std::string &text) const;
    int getVertexIndex(const std::string &id) const;
    LocationVertex *getVertexAt(int index) const;
    void addEdgeOneWay(LocationVertex *from, const std::string &toID, int dist, const std::string &status);
    bool removeEdgeOneWay(LocationVertex *from, const std::string &toID);

public:
    WarehouseGraph();
    ~WarehouseGraph();

    WarehouseGraph(const WarehouseGraph &) = delete;
    WarehouseGraph &operator=(const WarehouseGraph &) = delete;

    bool loadLocationsAndPathways(const std::string &locFile, const std::string &pathFile, const std::string &racksFile);
    bool addLocation(const std::string &id, const std::string &type, const std::string &name);
    LocationVertex *searchLocation(const std::string &id) const;

    bool addRackMapping(const std::string &rackID, const std::string &locID);
    std::string getLocationForRack(const std::string &rackID) const;

    bool addPathway(const std::string &locA, const std::string &locB, int dist, const std::string &status = "OPEN");
    bool removePathway(const std::string &locA, const std::string &locB);
    bool setPathwayStatus(const std::string &locA, const std::string &locB, const std::string &status);

    bool isConnectedBFS(const std::string &startID, const std::string &endID) const;
    bool findShortestRouteDijkstra(const std::string &startID, const std::string &endID, RouteResult &outRoute) const;

    void displayAllLocations() const;
    void displayRoute(const RouteResult &route) const;
    int getLocationCount() const { return vertexCount; }
};

#include "WarehouseGraph.ipp"
