// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_SEGMENT_HPP_
#define _GEOMETRY_SEGMENT_HPP_

#include "geometry/point.hpp"

namespace terrain::geometry {

struct Segment {
public:
    // - class is just POD
    // - allowed area is to the right, when going from start -> end
    Point start;
    Point end;

    // 
    Point center;

    // https://www.sciencedirect.com/science/article/pii/104996609190018K
    // Abstract
    // An algorithm is presented for converting a boundary representation for an image to its region quadtree representation.
    // Our algorithm is designed for operation on the linear quadtree representation, although it can easily be modified for 
    // the traditional pointer-based quadtree representation. The algorithm is a two phase process that first creates linear 
    // quadtree node records for each of the border pixels. This list of pixels is then sorted by locational code. The second 
    // processing phase fills in the nodes interior to the polygons by simulating a traversal of the corresponding pointer-based 
    // quadtree. Three previous algorithms have described similar conversion routines requiring time complexity of O(n · B) for 
    // at least one of the two phases, where B is the number of boundary pixels and n is the depth of the final tree for a 2n×2n 
    // image. A fourth algorithm, developed by Webber, can perform the border construction of this conversion in time O(n + B) 
    // with the restriction that the polygon must be positioned at constrained locations in the image space. Our algorithm 
    // requires time O(n + B) for the second phase, which is optimal. The first phase can be performed using the algorithm of 
    // Webber for total conversion time of O(n + B) with constrained location, in time O(B log B) using a simple sort to order 
    // the border pixels with no restriction in polygon location, or by a Jordan sequence sorting algorithm in time O(B) also 
    // with no restriction in polygon location.



    // https://stackoverflow.com/questions/36454103/storing-a-billion-polygons-in-quad-tree
public:
    Segment();
    Segment(const Point& _start, const Point& _end);

    void clear();

};
} // namespace terrain::geometry

#endif  // #endif _GEOMETRY_SEGMENT_HPP_
