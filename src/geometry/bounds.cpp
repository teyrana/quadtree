// The MIT License 
// (c) 2019 Daniel Williams

#include <iostream>
#include <cmath>

#include "geometry/point.hpp"
#include "geometry/bounds.hpp"

using std::ostream;
using std::abs;
using std::cerr;
using std::endl;
using std::max;

using geometry::Point;
using geometry::Bounds;


Bounds::Bounds():
    half_width(NAN)
{}

Bounds::Bounds( const Point& _center, const double _width):
    center(_center),  half_width(_width/2)
{}

void Bounds::clear() {
    center.clear();
    half_width = NAN;
}

bool Bounds::contains(const Point& at) const {
    // outside x-bounds:
    if( (at.x < center.x - half_width) || (at.x > center.x + half_width) ){
        return false;
    }

    // outside y-bounds:
    if( (at.y < center.y - half_width) || (at.y > center.y + half_width) ){ 
        return false;
    }
    
    return true;
}

double Bounds::get_size() const {
    return half_width*2;
}

double Bounds::get_x_max() const {
    return center.x + half_width;
}

double Bounds::get_x_min() const {
    return center.x - half_width;
}

double Bounds::get_y_max() const {
    return center.y + half_width;
}

double Bounds::get_y_min() const {
    return center.y - half_width;
}

double Bounds::get_width() const {
    return half_width*2;
}

bool Bounds::operator!=(const Bounds& other) const {
    return ! this->operator==(other);
}

bool Bounds::operator==(const Bounds& other) const {
    return ( this->center.near(other.center)) &&
           (Bounds::epsilon > std::abs(this->half_width - other.half_width));
}

double Bounds::snapx( double x) const {
    return std::max(get_x_min(), std::min(x, get_x_max()));
}

double Bounds::snapy( double y) const {
    return std::max(get_y_min(), std::min(y, get_y_max()));
}

double Bounds::width() const {
    return 2*half_width;
}

ostream& geometry::operator<<(ostream& sink, const Bounds& b){
    sink << " @" << b.center << " \u00B1" << b.half_width;
    return sink;
}
