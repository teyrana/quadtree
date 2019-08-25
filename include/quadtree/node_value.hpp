// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_NODE_VALUE_HPP_
#define _QUAD_TREE_NODE_VALUE_HPP_

#include <cmath>

namespace quadtree {

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

typedef uint32_t node_value_t;
    
} // namespace quadtree

#endif // _NODE_VALUE_HPP_
