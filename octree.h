#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "location_code.h"

enum class ExportFormat { OBJ_FORMAT };

using NodeType = uint16_t; // 8 bits child values then 8 bits child exists
using OptionalNodeType = std::optional<NodeType>;

template <typename LocationCode, typename MapWrapper>
class OctreeBase
{
public:

    using NodeMapType = typename MapWrapper::template TypeDecl<LocationCode, NodeType>::Type;
    using LocationCodes = LocationCodesBase<LocationCode>;

    OctreeBase(bool full=false, size_t capacity=0);

    void reserve(size_t capacity);

    void set_root();
    void clear_root();

    void set(LocationCode location_code);
    void clear(LocationCode location_code);

    float get_volume() const;

    NodeType* get_node_ptr(LocationCode location_code);
    OptionalNodeType get_node(LocationCode location_code) const;

    const NodeMapType& get_node_map() const { return _nodes; }

    static constexpr size_t get_max_depth()  { return LocationCodes::max_depth(); }
    int get_num_nodes() const { return 1 + _nodes.size(); }

    //friend std::ostream& operator<<(std::ostream&, const OctreeBase&);

    std::string to_string() const;

    void export_mesh(const char* filename, ExportFormat format) const;

private:

    void export_obj(std::ostream& os) const;

    NodeType get_node_unsafe(LocationCode location_code) const;
    void erase_node(LocationCode location_code);
    LocationCode get_node_volume(LocationCode location_code, LocationCode child_volume) const;

    NodeMapType _nodes;
};

template <typename LocationCode, typename MapType>
std::ostream& operator<<(std::ostream&, const OctreeBase<LocationCode, MapType>&);

template <typename LocationCode, typename MapType>
std::ostream& operator<<(std::ostream&, const typename OctreeBase<LocationCode, MapType>::NodeMapType&);

////////////

struct UnorderedMapWrapper
{
    template <typename Key, typename Value>
    struct TypeDecl
    {
        using Type = std::unordered_map<Key, Value>;
    };
};

using Octree32 = OctreeBase<uint32_t, UnorderedMapWrapper>;
using Octree64 = OctreeBase<uint64_t, UnorderedMapWrapper>;

#include "octree.inl.h"

#endif
