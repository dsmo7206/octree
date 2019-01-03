#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>

using LocationCodeType = uint32_t;
using NodeType = uint16_t; // 8 bits child values then 8 bits child exists
using OptionalNodeType = std::optional<NodeType>;
using NodeMapType = std::unordered_map<LocationCodeType, NodeType>;

class Octree
{
public:

    Octree(bool full=false, size_t capacity=0);

    void reserve(size_t capacity);

    void set_root();
    void clear_root();

    void set(LocationCodeType location_code);
    void clear(LocationCodeType location_code);

    float get_volume() const;

    NodeType* get_node_ptr(LocationCodeType location_code);
    OptionalNodeType get_node(LocationCodeType location_code) const;

    const NodeMapType& get_node_map() const { return _nodes; }

    inline constexpr size_t get_max_depth()  { return (8 * sizeof(LocationCodeType)) / 3; }
    inline int get_num_nodes() const { return 1 + _nodes.size(); }

    friend std::ostream& operator<<(std::ostream&, const Octree&);

    std::string to_string() const;

private:

    void erase_node(LocationCodeType location_code);
    LocationCodeType get_node_volume(LocationCodeType location_code, LocationCodeType child_volume) const;

    NodeMapType _nodes;
};

std::ostream& operator<<(std::ostream&, const Octree&);
std::ostream& operator<<(std::ostream&, const NodeMapType&);

#endif
