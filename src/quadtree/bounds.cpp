// The MIT License 
// (c) 2019 Daniel Williams

#include "quadtree/point.hpp"
#include "quadtree/bounds.hpp"

#include <cmath>

using quadtree::Point;
using quadtree::Bounds;


Bounds::Bounds():
    half_height(NAN), half_width(NAN)
{}

Bounds::Bounds( const Point& _center, const double _height, const double _width):
    center(_center),  half_height(_height/2), half_width(_width/2)
{}

bool Bounds::contains(double x, double y) const {
    // outside x-bounds:
    if( (x >= center.x + half_width) || (x <= center.x - half_width) ){
        return false;
    }

    // outside y-bounds:
    if( (y >= center.y + half_height) || (y <= center.y - half_height) ){
        return false;
    }
    
    return true;
}

void Bounds::extend(double x, double y){
    if(x > center.x + half_width){
        half_width = x - center.x;
    }
    if(x < center.x - half_width){
        half_width = center.x - x;
    }
    if(y > center.y + half_height){
        half_height = y - center.y;
    }
    if(y < center.y + half_height){
        half_height = y - center.y;
    }
}

// const Point& Bounds::get_center() const {
//     return center;
// }

// double Bounds::get_width() const {
//     return half_height*2;
// }

// double Bounds::get_max_x() const {
//     return center.x + half_width;
// }

// double Bounds::get_max_y() const {
//     return center.y + half_height;
// }

// double Bounds::get_min_x() const {
//     return center.x - half_width;
// }

// double Bounds::get_min_y() const {
//     return center.y - half_height;
// }
