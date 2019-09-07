#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <iostream>

#include "geometry/polygon.hpp"

using std::cerr;
using std::max;
using std::min;
using std::endl;
using std::ostream;
using std::string;
using std::setw;

using geometry::Point;
using geometry::Polygon;

//---------------------------------------------------------------
// Constructor
//
Polygon::Polygon():
    Polygon("<unnamed>")
{}

Polygon::Polygon(string _name):
    name(_name),
    inclusive(true)
{
    set_default();
}

bool Polygon::load(const bool as_inclusive, std::vector<Point> source){
    // if the new polygon contains insufficient points, abort and clear.
    if(4 > source.size()){
        return false;
    }

    this->inclusive = as_inclusive;
    points = std::move(source);
    update_bounds();

    // cerr << "====== ====== ====== " << endl;
    // write_yaml(cerr, "    ");

    enclose_polygon();
    if(! is_right_handed()){
        std::reverse(std::begin(points), std::end(points));
    }

    // cerr << "====== ====== ====== " << endl;
    // write_yaml(cerr, "    ");

    return true;
}

void Polygon::clear(){
    points.clear();
    bounds.clear();
}

void Polygon::enclose_polygon(){
    // ensure that polygon loops back
    const auto& first_point = points[0];
    const auto& last_point = points[points.size()-1];

    if( ! first_point.near(last_point)){
        points.emplace_back(first_point);
    }
}

const Bounds& Polygon::get_bounds() const {
    return bounds;
}

bool Polygon::is_right_handed() const {
    double sum = 0;

    // NOTE THE RANGE!:
    //   this needs to iterate over all doubles
    for( uint i = 0; i < (points.size()-1); ++i ){
        auto& p1 = points[i];
        auto& p2 = points[i+1];
        sum += (p1.x*p2.y) - (p1.y*p2.x);
    }

    // // The shoelace formula includes a divide-by-2 step; ... but we don't
    // // care about the magnitude, just the sign.  So skip this step.
    // sum /= 2;

    if( 0 > sum ){
        // left-handed / clockwise points
        return false;
    }else{
        // right-handed / counter-clockwise points
        return true;
    }
}

ostream& operator<<(ostream& os, const Polygon& poly)
{
    poly.write_yaml(os);
    return os;
}

#include <iostream>     // std::cout
#include <algorithm>    // std::reverse
#include <vector>       // std::vector

void Polygon::set_default(){
    points.clear();
    points.emplace_back( 0, 0);
    points.emplace_back( 1, 0);
    points.emplace_back( 1, 1);
    points.emplace_back( 0, 1);
    update_bounds();
}

void Polygon::update_bounds() {
    bounds.clear();

    Point max(FLT_MIN, FLT_MIN);
    Point min(FLT_MAX, FLT_MAX);
    for( uint i = 0; i < points.size(); ++i ){
        auto& p = points[i];
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
    }

    bounds.center = geometry::average(min,max);
    bounds.half_width = std::max(max.x - min.x, max.y - min.y)/2;
}


void Polygon::write_yaml(std::ostream& sink, string indent) const {
    sink << indent << "name: " << name << '\n';
    sink << indent << "inclusive: " << inclusive << '\n';

    sink << indent << "points: \n";
    for( uint i = 0; i < points.size(); ++i ){
        auto& p = points[i];
        sink << indent << "    - " << p.x << ", " << p.y << '\n';
    }

    sink << indent << "bounds: \n";
    sink << indent << "    center: [" << bounds.center.x << ", " << bounds.center.y << "]\n";
    sink << indent << "    x: [" << bounds.get_x_min() << ", " << bounds.get_x_max() << "]\n";
    sink << indent << "    y: [" << bounds.get_y_min() << ", " << bounds.get_y_max() << "]\n";

}
