// Map.hpp
#ifndef MAP_HPP
#define MAP_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

struct MapNode {
    int id;                 // stable ID for movement later
    std::string name;       // display name
    bool shrine;            // true if a shrine room
};

class TempleMap {
public:
    // create nodes + edges for current design
    void populateFromDesign();

    // text outputs
    void printAscii() const;                 // compact visual clusters
    void printAdjacency() const;             // room graph (for debugging/movement)

    // data access for future movement
    const std::vector<MapNode>& nodes() const { return nodes_; }
    const std::vector<std::vector<int>>& edges() const { return edges_; }
    const std::unordered_map<std::string,int>& nameToId() const { return nameToId_; }

private:
    std::vector<MapNode> nodes_;
    std::vector<std::vector<int>> edges_;    // adjacency by node index
    std::unordered_map<std::string,int> nameToId_;

    int addNode(const std::string& name, bool shrine);
    void addEdgeByName(const std::string& a, const std::string& b);
};

#endif
