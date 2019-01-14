#ifndef _LOCATION_CODE_H_
#define _LOCATION_CODE_H_

struct Vertex
{
    Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    const float x;
    const float y;
    const float z;

    inline bool operator==(const Vertex& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

template <typename T>
struct LocationCodesBase
{
    static constexpr uint8_t max_depth();
    static constexpr uint8_t high_bit_index(T location_code);
    static constexpr uint8_t depth(T location_code);
    static constexpr T high_bit(T location_code);
    static constexpr T location_bits(T location_code);
    static constexpr T parent_code(T location_code);
    static constexpr T child_code(T location_code, uint8_t child_index);
    static constexpr uint8_t final_child_index(T location_code);
    static T lower_corner_code(T location_code);
    static Vertex lower_corner(T location_code);
    static std::string to_binary(T location_code);
};

// constexpr uint32_t DEPTH_TO_HALF_SIZE[10] = {
//     512,
//     256,
//     128,
//     64,
//     32,
//     16,
//     8,
//     4,
//     2,
//     1
// };

#include "location_code.inl.h"

#endif // _LOCATION_CODE_H_
