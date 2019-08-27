// The MIT License 
// (c) 2019 Daniel Williams

#include "geometry/point.hpp"
#include "geometry/bounds.hpp"

#include <cmath>

using geometry::Point;
using geometry::Bounds;


Bounds::Bounds():
    half_height(NAN), half_width(NAN)
{}

Bounds::Bounds( const Point& _center, const double _height, const double _width):
    center(_center),  half_height(_height/2), half_width(_width/2)
{}

void Bounds::clear() {
    center.clear();
    half_height = NAN;
    half_width = NAN;
}

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

void Bounds::extend(const Point p){
    extend(p.x, p.y);
}

void Bounds::extend(double x, double y){
    if(isnan(center.x)){
        center.x = x;
        half_width = 0;
    }else if(x > center.x + half_width){
        center.x = (x + (center.x - half_width))/2;
        half_width = x - center.x; 
    }else if(x < center.x - half_width){
        center.x = (x + (center.x + half_width))/2;
        half_width = center.x - x;
    }

    if(isnan(center.y)){
        center.y = y;
        half_height = 0;
    }else if(y > center.y + half_height){
        center.y = (y + (center.y - half_height))/2;
        half_height = y - center.y;
    }else if(y < center.y - half_height){
        center.y = (y + (center.y + half_height))/2;
        half_height = center.y - y;
    }
}

double Bounds::get_height() const {
    return half_height*2;
}

double Bounds::get_x_max() const {
    return center.x + half_width;
}

double Bounds::get_x_min() const {
    return center.x - half_width;
}

double Bounds::get_y_max() const {
    return center.y + half_height;
}

double Bounds::get_y_min() const {
    return center.y - half_height;
}

double Bounds::get_width() const {
    return half_width*2;
}
