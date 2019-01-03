#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "octree.h"

constexpr NodeType ALL_CHILDREN_SET = 0xff00;

inline bool get_child_exists(NodeType node, int i)
{
    return node & (1 << i);
}

inline bool get_child_set_if_not_exists(NodeType node, int i)
{
    return node & (1 << (i + 8));
}

inline bool get_child_set(NodeType node, int i)
{
    return !get_child_exists(node, i) && node & (1 << (i + 8));
}

inline void set_child_exists(NodeType& node, int i)
{
    node |= (1 << i); // Set exists
    node &= ~(1 << (i + 8)); // Clear value
}

inline void set_child_value(NodeType& node, int i)
{
    node |= (1 << (i + 8)); // Set value
    node &= ~(1 << i); // Clear exists
}

inline int get_depth(LocationCodeType location_code)
{
#if defined(__GNUC__)
    return (8 * sizeof(LocationCodeType) - 1 - __builtin_clz(location_code)) / 3;
#elif defined(_MSC_VER)
    long msb;
    _BitScanReverse(&msb, location_code);
    return msb / 3;
#endif
}

inline LocationCodeType get_parent_code(LocationCodeType location_code)
{
    return location_code >> 3;
}

inline LocationCodeType get_child_code(LocationCodeType location_code, LocationCodeType i)
{
    return (location_code << 3) | i;
}

inline int get_final_child_index(LocationCodeType location_code)
{
    return location_code & 0x7;
}

template <typename T>
inline std::string to_binary(T location_code)
{
    return std::bitset<8*sizeof(T)>(location_code).to_string();
}

Octree::Octree(bool full, size_t capacity)
{
    full ? set_root() : clear_root();
    reserve(capacity);
}

void Octree::reserve(size_t capacity)
{
    _nodes.reserve(capacity);
}

void Octree::set_root()
{
    _nodes.clear();
    _nodes.emplace(1, ALL_CHILDREN_SET);
}

void Octree::clear_root()
{
    _nodes.clear();
    _nodes.emplace(1, 0);
}

void Octree::set(LocationCodeType location_code)
{
    if (location_code == 1) // Root - there is no parent
    {
        set_root();
        return;
    }

    // Find the parent code of the desired location; this is the Node that will need
    // to be created/modified.
    const LocationCodeType parent_location_code = get_parent_code(location_code);
    const int parent_depth = get_depth(parent_location_code);

    // Iterate down from the root to the parent
    for (int depth = 0; depth < parent_depth; ++depth)
    {
        const LocationCodeType ancestor_location_code = location_code >> 3 * (parent_depth + 1 - depth);
        const int child_index = get_final_child_index(location_code >> 3 * (parent_depth - depth));
        const auto result = _nodes.emplace(ancestor_location_code, 1 << child_index);

        if (!result.second)
        {
            if (get_child_set(result.first->second, child_index))
            {
                return; // This child is already set fully; no need to traverse
            }

            set_child_exists(result.first->second, child_index);
        }
    }

    // Now we are at the parent depth
    const auto result = _nodes.emplace(parent_location_code, 1 << (get_final_child_index(location_code) + 8));

    if (result.second) // The parent node didn't exist
    {
        return;
    }

    // The parent node already existed
    const int child_index = get_final_child_index(location_code);
    set_child_value(result.first->second, child_index);

    // If the node itself exists, erase it and all its children
    erase_node(location_code);

    // If we get to this point, we know that all ancestors already existed, 
    // otherwise we would have triggered the early return above.
    // There's a chance that one or more ancestors are now fully set, in which case
    // we must erase them and tweak their parents.
    LocationCodeType ancestor_location_code = parent_location_code;
    NodeType* node = &result.first->second;

    for (int depth = parent_depth; depth > 0; --depth)
    {
        if (*node == ALL_CHILDREN_SET)
        {
            // Erase and set the parent as set
            erase_node(ancestor_location_code);
            const int child_index = get_final_child_index(ancestor_location_code);
            ancestor_location_code = get_parent_code(ancestor_location_code);
            node = get_node_ptr(ancestor_location_code);
            set_child_value(*node, child_index);
        }
        else
        {
            break;
        }
    }
}


void Octree::clear(LocationCodeType location_code)
{

}

NodeType* Octree::get_node_ptr(LocationCodeType location_code)
{
    const auto it = _nodes.find(location_code);
    return (it == _nodes.end()) ? nullptr : &(it->second);
}

OptionalNodeType Octree::get_node(LocationCodeType location_code) const
{
    const auto it = _nodes.find(location_code);
    return (it == _nodes.end()) ? std::nullopt : OptionalNodeType(it->second);
}

float Octree::get_volume() const
{
    constexpr LocationCodeType biggest = 1 << (8 * sizeof(LocationCodeType) - 1);
    return (float)get_node_volume(1, biggest >> 3) / biggest;
}

void Octree::erase_node(LocationCodeType location_code)
{
    const auto it = _nodes.find(location_code);
    if (it == _nodes.end())
    {
        return;
    }

    for (int i = 0; i < 8; ++i)
    {
        if (get_child_exists(it->second, i))
        {
            erase_node(get_child_code(location_code, i));
        }
    }

    _nodes.erase(it);
}

LocationCodeType Octree::get_node_volume(LocationCodeType location_code, LocationCodeType child_volume) const
{
    const std::optional<NodeType> node = get_node(location_code);
    LocationCodeType result = 0;

    for (int i = 0; i < 8; ++i)
    {
        if (get_child_exists(node.value(), i))
        {
            result += get_node_volume(get_child_code(location_code, i), child_volume >> 3);
        }
        else if (get_child_set_if_not_exists(node.value(), i))
        {
            result += child_volume;
        }
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, const Octree& tree)
{
    os << "Octree with volume " << tree.get_volume() << ":\n" << tree._nodes;
    return os;
}

std::ostream& operator<<(std::ostream& os, const NodeMapType& nodes)
{
    const auto show_node = [&os](int code_padding, LocationCodeType location_code, NodeType node)
    {
        std::vector<LocationCodeType> children, filled_values;

        os << std::setw(code_padding) << location_code << " (D" << get_depth(location_code) << "): ";
        for (int i = 0; i < 8; ++i)
        {
            if (get_child_exists(node, i)) 
            {
                children.push_back(get_child_code(location_code, i));
            }
            else if (get_child_set_if_not_exists(node, i))
            {
                filled_values.push_back(get_child_code(location_code, i));
            }
        }

        os << "SC: [";
        for (LocationCodeType child_code : filled_values)
        {
            os << child_code << ' ';
        }
        os << "], OC: [";

        for (LocationCodeType child_code : children)
        {
            os << child_code << ' ';
        }
        os << "]\n";
    };

    std::vector<LocationCodeType> keys;
    keys.reserve(nodes.size());
    for (const auto& it : nodes)
    {
        keys.push_back(it.first);
    }
    std::sort(keys.begin(), keys.end());

    const int code_padding = (keys.empty()) ? 0 : std::to_string(keys[keys.size() - 1]).size();

    for (LocationCodeType location_code : keys)
    {
        show_node(code_padding, location_code, nodes.find(location_code)->second);
    }

    return os;
}

std::string Octree::to_string() const
{
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}
