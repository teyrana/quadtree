// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_BOUNDS_HPP_
#define _GEOMETRY_BOUNDS_HPP_

#include <iostream>
#include <string>

#include "nlohmann/json/json_fwd.hpp"

#include "point.hpp"

namespace terrain::geometry {
class Bounds {
public:
    constexpr Bounds():
        center(NAN,NAN),half_width(NAN) {}
    constexpr Bounds(const Point& _center, const double _width):
        center(_center), half_width(_width/2) {}

    // definitely _not_ constexpr ;)
    Bounds(nlohmann::json& doc);

    void clear();

    bool contains(const Point& at) const;

    bool operator!=(const Bounds& other) const;
    bool operator==(const Bounds& other) const;

    double get_size() const;
    double get_x_max() const;
    double get_x_min() const;
    double get_y_max() const;
    double get_y_min() const;
    double get_width() const;

    bool load(nlohmann::json& doc);

    double snapx( double x) const;
    double snapy( double y) const;

    std::string str() const;

    nlohmann::json to_json() const;

    constexpr double width() const { return half_width*2; }
    
public:
    // used for comparisons
    constexpr static double epsilon = 1e-6;

    Point center;
    // (defined in alphabetical order)
    double half_width;

};

} // namespace terrain::geometry

#endif // #ifdef _GEOMETRY_BOUNDS_HPP_
