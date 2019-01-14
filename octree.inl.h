#include <algorithm>
#include <fstream>
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

template <typename LocationCode, typename MapType>
OctreeBase<LocationCode, MapType>::OctreeBase(bool full, size_t capacity)
{
    full ? set_root() : clear_root();
    reserve(capacity);
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::reserve(size_t capacity)
{
    _nodes.reserve(capacity);
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::set_root()
{
    _nodes.clear();
    _nodes.emplace(1, ALL_CHILDREN_SET);
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::clear_root()
{
    _nodes.clear();
    _nodes.emplace(1, 0);
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::set(LocationCode location_code)
{
    if (location_code == 1) // Root - there is no parent
    {
        set_root();
        return;
    }

    // Find the parent code of the desired location; this is the Node that will need
    // to be created/modified.
    const LocationCode parent_location_code = LocationCodes::parent_code(location_code);
    //const int parent_depth = get_depth(parent_location_code);
    const int parent_depth = LocationCodes::depth(parent_location_code);

    // Iterate down from the root to the parent
    for (int depth = 0; depth < parent_depth; ++depth)
    {
        const LocationCode ancestor_location_code = location_code >> 3 * (parent_depth + 1 - depth);
        const int child_index = LocationCodes::final_child_index(location_code >> 3 * (parent_depth - depth));
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
    const auto result = _nodes.emplace(parent_location_code, 1 << (LocationCodes::final_child_index(location_code) + 8));

    if (result.second) // The parent node didn't exist
    {
        return;
    }

    // The parent node already existed
    const int child_index = LocationCodes::final_child_index(location_code);
    set_child_value(result.first->second, child_index);

    // If the node itself exists, erase it and all its children
    erase_node(location_code);

    // If we get to this point, we know that all ancestors already existed, 
    // otherwise we would have triggered the early return above.
    // There's a chance that one or more ancestors are now fully set, in which case
    // we must erase them and tweak their parents.
    LocationCode ancestor_location_code = parent_location_code;
    NodeType* node = &result.first->second;

    for (int depth = parent_depth; depth > 0; --depth)
    {
        if (*node == ALL_CHILDREN_SET)
        {
            // Erase and set the parent as set
            erase_node(ancestor_location_code);
            const int child_index = LocationCodes::final_child_index(ancestor_location_code);
            ancestor_location_code = LocationCodes::parent_code(ancestor_location_code);
            node = get_node_ptr(ancestor_location_code);
            set_child_value(*node, child_index);
        }
        else
        {
            break;
        }
    }
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::clear(LocationCode location_code)
{
}

template <typename LocationCode, typename MapType>
NodeType* OctreeBase<LocationCode, MapType>::get_node_ptr(LocationCode location_code)
{
    const auto it = _nodes.find(location_code);
    return (it == _nodes.end()) ? nullptr : &(it->second);
}

template <typename LocationCode, typename MapType>
OptionalNodeType OctreeBase<LocationCode, MapType>::get_node(LocationCode location_code) const
{
    const auto it = _nodes.find(location_code);
    return (it == _nodes.end()) ? std::nullopt : OptionalNodeType(it->second);
}

template <typename LocationCode, typename MapType>
NodeType OctreeBase<LocationCode, MapType>::get_node_unsafe(LocationCode location_code) const
{
    return _nodes.find(location_code)->second;
}

template <typename LocationCode, typename MapType>
float OctreeBase<LocationCode, MapType>::get_volume() const
{
    constexpr LocationCode biggest = 1 << (8 * sizeof(LocationCode) - 1);
    return (float)get_node_volume(1, biggest >> 3) / biggest;
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::erase_node(LocationCode location_code)
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
            erase_node(LocationCodes::child_code(location_code, i));
        }
    }

    _nodes.erase(it);
}

template <typename LocationCode, typename MapType>
LocationCode OctreeBase<LocationCode, MapType>::get_node_volume(LocationCode location_code, LocationCode child_volume) const
{
    const std::optional<NodeType> node = get_node(location_code);
    LocationCode result = 0;

    for (int i = 0; i < 8; ++i)
    {
        if (get_child_exists(node.value(), i))
        {
            result += get_node_volume(LocationCodes::child_code(location_code, i), child_volume >> 3);
        }
        else if (get_child_set_if_not_exists(node.value(), i))
        {
            result += child_volume;
        }
    }

    return result;
}

template <typename LocationCode, typename MapType>
std::ostream& operator<<(std::ostream& os, const typename OctreeBase<LocationCode, MapType>::NodeMapType& nodes)
{
    const auto show_node = [&os](int code_padding, LocationCode location_code, NodeType node)
    {
        std::vector<LocationCode> children, filled_values;

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
        for (LocationCode child_code : filled_values)
        {
            os << child_code << ' ';
        }
        os << "], OC: [";

        for (LocationCode child_code : children)
        {
            os << child_code << ' ';
        }
        os << "]\n";
    };

    std::vector<LocationCode> keys;
    keys.reserve(nodes.size());
    for (const auto& it : nodes)
    {
        keys.push_back(it.first);
    }
    std::sort(keys.begin(), keys.end());

    const int code_padding = (keys.empty()) ? 0 : std::to_string(keys[keys.size() - 1]).size();

    for (LocationCode location_code : keys)
    {
        show_node(code_padding, location_code, nodes.find(location_code)->second);
    }

    return os;
}

template <typename LocationCode, typename MapType>
std::ostream& operator<<(std::ostream& os, const OctreeBase<LocationCode, MapType>& tree)
{
    const typename OctreeBase<LocationCode, MapType>::NodeMapType& node_map = &tree.get_node_map();

    os << "Octree with volume " << tree.get_volume() << ":\n" << node_map;
    return os;
}

template <typename LocationCode, typename MapType>
std::string OctreeBase<LocationCode, MapType>::to_string() const
{
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::export_mesh(const char* filename, ExportFormat format) const
{
    if (format != ExportFormat::OBJ_FORMAT)
    {
        throw std::runtime_error("Invalid ExportFormat");
    }

    std::ofstream file(filename);
    export_obj(file);
}

template <typename LocationCode, typename MapType>
void OctreeBase<LocationCode, MapType>::export_obj(std::ostream& os) const
{
    // const int x_offset[8] = {0, 1, 0, 1, 0, 1, 0, 1};
    // const int y_offset[8] = {0, 0, 1, 1, 0, 0, 1, 1};
    // const int z_offset[8] = {0, 0, 0, 0, 1, 1, 1, 1};

    // std::unordered_map<uint32_t, uint32_t> vertex_code_to_index;

    // for (const auto& kv : _nodes)
    // {
    //     if ((kv.second & ALL_CHILDREN_SET) == 0)
    //     {
    //         // There are no children set; skip this node
    //         continue;
    //     }

    //     const int depth = get_depth(kv.first);
    //     const uint32_t corner_code = get_corner_code(kv.first);
    //     const uint32_t half_size = DEPTH_TO_HALF_SIZE[depth];

    //     for (int i = 0; i < 8; ++i)
    //     {
    //         if (!get_child_set(kv.second, i))
    //         {
    //             continue;
    //         }

    //         //corner.x + half_size * x_offset[i];
    //     }
    // }
}
