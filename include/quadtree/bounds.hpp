// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_BOUNDS_HPP_
#define _QUAD_TREE_BOUNDS_HPP_

#include <string>

#include "point.hpp"

namespace quadtree {
class Bounds {
public:
    Bounds();
    Bounds(const Point& center, const double height, double width);

    bool contains(double x, double y) const;
    
    void extend(double x, double y);

    // const Point& get_center() const;
    // double get_height() const;
    // double get_max_x() const;
    // double get_max_y() const;
    // double get_min_x() const;
    // double get_min_y() const;
    // double get_width() const;

public:
    Point center;
    // (defined in alphabetical order)
    double half_height;
    double half_width;

    // all other quantities are derived from the above


};
} // namespace quadtree
#endif
