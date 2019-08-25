// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_SEGMENT_HPP_
#define _GEOMETRY_SEGMENT_HPP_

#include "geometry/point.hpp"

namespace geometry {
struct Segment {
public:
    // class is just POD + convenient methods
    Point start;
    Point end;

public:
    Segment();
    Segment(const Point& _start, const Point& _end);


    void clear();

};
} // namespace geometry
#endif
