#include <bitset>
#include <cstdint>
#include <immintrin.h>

// Common

template <typename T>
constexpr T LocationCodesBase<T>::high_bit(T location_code)
{
    return 1 << high_bit_index(location_code);
}

template <typename T>
constexpr uint8_t LocationCodesBase<T>::depth(T location_code)
{
    return high_bit_index(location_code) / 3;
}

template <typename T>
constexpr T LocationCodesBase<T>::location_bits(T location_code)
{
    return location_code & ~high_bit(location_code);
}

template <typename T>
constexpr T LocationCodesBase<T>::parent_code(T location_code)
{
    return location_code >> 3;
}

template <typename T>
constexpr T LocationCodesBase<T>::child_code(T location_code, uint8_t child_index)
{
    return (location_code << 3) | child_index;
}

template <typename T>
constexpr uint8_t LocationCodesBase<T>::final_child_index(T location_code)
{
    return location_code & 0x7;
}

template <typename T>
std::string LocationCodesBase<T>::to_binary(T location_code)
{
    return std::bitset<8*sizeof(T)>(location_code).to_string();
}

// uint32_t

template <>
constexpr uint8_t LocationCodesBase<uint32_t>::max_depth() { return 9; }

template <>
constexpr uint8_t LocationCodesBase<uint32_t>::high_bit_index(uint32_t location_code)
{
#if defined(__GNUC__)  
    return 31 - __builtin_clz(location_code);
#elif defined(_MSC_VER)
    unsigned long msb;
    _BitScanReverse(&msb, location_code);
    return msb;
#endif
}

template <>
Vertex LocationCodesBase<uint32_t>::lower_corner(uint32_t location_code)
{
    const uint32_t bits = location_bits(location_code);
    const int shift = max_depth() - depth(location_code);

    return Vertex(
        _pext_u32(bits, 0b001001001001001001001001001001) << shift,
        _pext_u32(bits, 0b010010010010010010010010010010) << shift,
        _pext_u32(bits, 0b100100100100100100100100100100) << shift
    );
}

template <>
uint32_t LocationCodesBase<uint32_t>::lower_corner_code(uint32_t location_code)
{
    const uint32_t bits = location_bits(location_code);
    const int shift = max_depth() - depth(location_code);

    return (
        (_pext_u32(bits, 0b001001001001001001001001001001) << 20) |
        (_pext_u32(bits, 0b010010010010010010010010010010) << 10) |
         _pext_u32(bits, 0b100100100100100100100100100100)
    ) << shift;
}

// uint64_t

template <>
constexpr uint8_t LocationCodesBase<uint64_t>::max_depth() { return 20; }

template <>
constexpr uint8_t LocationCodesBase<uint64_t>::high_bit_index(uint64_t location_code)
{
#if defined(__GNUC__)  
    return 63 - __builtin_clzl(location_code);
#elif defined(_MSC_VER)
    unsigned long msb;
    _BitScanReverse64(&msb, location_code);
    return msb;
#endif
}

template <>
Vertex LocationCodesBase<uint64_t>::lower_corner(uint64_t location_code)
{
    const uint64_t bits = location_bits(location_code);
    const int shift = max_depth() - depth(location_code);

    return Vertex(
        _pext_u64(
            bits, 
            0b001001001001001001001001001001001001001001001001001001001001
        ) << shift,
        _pext_u64(
            bits, 
            0b010010010010010010010010010010010010010010010010010010010010
        ) << shift,
        _pext_u64(
            bits, 
            0b100100100100100100100100100100100100100100100100100100100100
        ) << shift
    );
}

template <>
uint64_t LocationCodesBase<uint64_t>::lower_corner_code(uint64_t location_code)
{
    const uint64_t bits = location_bits(location_code);
    const int shift = max_depth() - depth(location_code);

    return (
        (_pext_u64(
            bits, 
            0b001001001001001001001001001001001001001001001001001001001001
        ) << 40) |
        (_pext_u64(
            bits, 
            0b010010010010010010010010010010010010010010010010010010010010
        ) << 20) |
        _pext_u64(
            bits, 
            0b100100100100100100100100100100100100100100100100100100100100
        )
    ) << shift;
}
