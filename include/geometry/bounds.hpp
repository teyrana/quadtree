// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_BOUNDS_HPP_
#define _GEOMETRY_BOUNDS_HPP_

#include <string>
#include <iostream>

#include "point.hpp"

namespace geometry {
class Bounds {
public:
    Bounds();
    Bounds(const Point& center, const double width);

    void clear();

    bool contains(const Point& at) const;

    double get_size() const;
    double get_x_max() const;
    double get_x_min() const;
    double get_y_max() const;
    double get_y_min() const;
    double get_width() const;

public:
    Point center;
    // (defined in alphabetical order)
    double half_width;

    // all other quantities are derived from the above


};

std::ostream& operator<<(std::ostream& sink, const Bounds& b);

} // namespace geometry
#endif
