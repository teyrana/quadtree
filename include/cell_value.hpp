// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _CELL_VALUE_HPP_
#define _CELL_VALUE_HPP_

#include <cmath>

namespace terrain {

// // sized to fit in:  4 x sizeof(void*) on this platform, 
// // compact POD.
// struct NodeValue {
// public:
//     // distance from the edge
//     double distance;
//     // normal from the edge.  Pointing towards the interior, in a RHL direction
//     double direction;

// public:
//     // NodeValue(): distance(NAN), direction(NAN);
//     // ~NodeValue();
// };

typedef uint8_t cell_value_t;

constexpr cell_value_t cell_error_value   = 0xAB;
constexpr cell_value_t cell_default_value = 0x99;
    
} // namespace terrain

#endif // _CELL_VALUE_HPP_
