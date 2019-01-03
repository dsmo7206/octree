#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <vector>

#include "octree.h"
#include "catch.hpp"

NodeType make_node(const std::vector<int>& children_set, const std::vector<int>& children_exist)
{
    NodeType result = 0;

    for (int child : children_set)
    {
        result |= 1 << (child + 8);
    }
    for (int child : children_exist)
    {
        result |= 1 << child;
    }

    return result;
}

uint32_t make_locator(int depth, int x_index, int y_index, int z_index)
{
    return (
        (1 << 3 * depth) |
        _pdep_u32(x_index, 0b100100100100100100100100100100) |
        _pdep_u32(y_index, 0b010010010010010010010010010010) |
        _pdep_u32(z_index, 0b001001001001001001001001001001)
    );
};

class Timer
{
public:

    Timer(const char* name) : 
        _name(name),
        _start_time(std::chrono::high_resolution_clock::now())
    {}

    ~Timer()
    {
        const auto duration = (std::chrono::high_resolution_clock::now() - _start_time).count();
        std::cout << _name << " took " << duration << " ns" << std::endl;
    }

private:
    
    const char* _name;
    const std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};

/////////////////////////////////////////////////////

TEST_CASE("Empty octree")
{
    Octree octree(false);

    const NodeMapType expected = {
        {0b1, make_node({}, {})},
    };

    REQUIRE(octree.get_node_map() == expected);
    REQUIRE(octree.get_volume() == 0);
}

TEST_CASE("Full octree")
{
    Octree octree(true);
    
    const NodeMapType expected = {
        {0b1, make_node({0, 1, 2, 3, 4, 5, 6, 7}, {})},
    };

    REQUIRE(octree.get_node_map() == expected);
    REQUIRE(octree.get_volume() == 1);
}

TEST_CASE("Set root indirectly")
{
    Octree octree(false);
    octree.set_root();
    
    const NodeMapType expected = {
        {0b1, make_node({0, 1, 2, 3, 4, 5, 6, 7}, {})},
    };

    REQUIRE(octree.get_node_map() == expected);
    REQUIRE(octree.get_volume() == 1);
}

TEST_CASE("Set root directly")
{
    Octree octree(false);
    octree.set(0b1);
    
    const NodeMapType expected = {
        {0b1, make_node({0, 1, 2, 3, 4, 5, 6, 7}, {})},
    };

    REQUIRE(octree.get_node_map() == expected);
    REQUIRE(octree.get_volume() == 1);
}

TEST_CASE("Set deep node")
{
    Octree octree(false);
    octree.set(0b1000111000111101010111);
    
    const NodeMapType expected = {
        {0b1,                   make_node( {}, {0})},
        {0b1000,                make_node( {}, {7})},
        {0b1000111,             make_node( {}, {0})},
        {0b1000111000,          make_node( {}, {7})},
        {0b1000111000111,       make_node( {}, {5})},
        {0b1000111000111101,    make_node( {}, {2})},
        {0b1000111000111101010, make_node({7},  {})},
    };
    
    REQUIRE(octree.get_node_map() == expected);
}

TEST_CASE("Set shallow node then deep node")
{
    Octree octree(false);
    octree.set(0b1000111);
    octree.set(0b1000111000111101010111);
    
    const NodeMapType expected = {
        {0b1,    make_node( {}, {0})},
        {0b1000, make_node({7},  {})},
    };
    
    REQUIRE(octree.get_node_map() == expected);
}

TEST_CASE("Set deep node then shallow node")
{
    Octree octree(false);
    octree.set(0b1000111000111101010111);
    octree.set(0b1000111);
    
    const NodeMapType expected = {
        {0b1,    make_node( {}, {0})},
        {0b1000, make_node({7},  {})},
    };
    
    REQUIRE(octree.get_node_map() == expected);
}

TEST_CASE("Set two deep nodes")
{
    Octree octree(false);
    octree.set(0b1000111000111101010111);
    octree.set(0b1000111111111101010111);
    
    const NodeMapType expected = {
        {0b1,                   make_node( {},    {0})},
        {0b1000,                make_node( {},    {7})},
        {0b1000111,             make_node( {}, {0, 7})},
        // The 0 branch
        {0b1000111000,          make_node( {},    {7})},
        {0b1000111000111,       make_node( {},    {5})},
        {0b1000111000111101,    make_node( {},    {2})},
        {0b1000111000111101010, make_node({7},     {})},
        // The 7 branch
        {0b1000111111,          make_node( {},    {7})},
        {0b1000111111111,       make_node( {},    {5})},
        {0b1000111111111101,    make_node( {},    {2})},
        {0b1000111111111101010, make_node({7},     {})},
    };
    
    REQUIRE(octree.get_node_map() == expected);
}

TEST_CASE("Basic set unwinding")
{
    // Set all the 8 children of a given (fairly shallow) node.
    // The parent node should itself be destroyed, and its parent
    // should have it marked as set.
    Octree octree(false);
    octree.set(0b1000000);
    octree.set(0b1000001);
    octree.set(0b1000010);
    octree.set(0b1000011);
    octree.set(0b1000100);
    octree.set(0b1000101);
    octree.set(0b1000110);
    octree.set(0b1000111);

    const NodeMapType expected = {
        {0b1, make_node({0}, {})},
    };
    
    REQUIRE(octree.get_node_map() == expected);
}

TEST_CASE("Sphere test")
{
    constexpr int depth = 7;
    constexpr int res = 1 << depth;

    // Make a res^3 octree representing a sphere
    Octree octree(false);
    octree.reserve(1 << (3 * depth));
    //octree.reserve(100000);

    std::vector<float> centers;
    for (int i = 0; i < res; ++i)
    {
        centers.push_back((float)i / res + 0.5f/res - 0.5f);
        //std::cout << i << ": " << centers.back() << std::endl;
    }

    {
        Timer timer("Building octree");

        for (int x_index = 0; x_index < res; ++x_index)
        {
            for (int y_index = 0; y_index < res; ++y_index)
            {;
                for (int z_index = 0; z_index < res; ++z_index)
                {
                    // if (centers[x_index]*centers[x_index] + centers[y_index]*centers[y_index] + centers[z_index]*centers[z_index] >= 0.25f)
                    //     continue;

                    if (x_index != res - 1 || y_index != res - 1 || z_index != res - 1)

                        octree.set(make_locator(depth, x_index, y_index, z_index));
                }
            }
        }
    }

    float volume;
    {
        Timer timer("Getting volume");
        volume = octree.get_volume();
    }
    std::cout << "volume = " << volume << std::endl;

    uint32_t num_nodes;
    {
        Timer timer("Getting num nodes");
        num_nodes = octree.get_num_nodes();
    }
    std::cout << "Num nodes = " << num_nodes << std::endl;
    std::cout << octree << std::endl;
}