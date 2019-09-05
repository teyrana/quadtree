// The MIT License 
// (c) 2019 Daniel Williams

#include <iostream>
#include <cmath>

#include "geometry/point.hpp"
#include "geometry/bounds.hpp"

using std::ostream;

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

bool Bounds::contains(const Point& at) const {
    // outside x-bounds:
    if( (at.x < center.x - half_width) || (at.x > center.x + half_width) ){
        return false;
    }

    // outside y-bounds:
    if( (at.y < center.y - half_height) || (at.y > center.y + half_height) ){ 
        return false;
    }
    
    return true;
}


void Bounds::extend(double x, double y){
    extend({x, y});
}

void Bounds::extend(const Point& p){
    if(isnan(center.x)){
        center.x = p.x;
        half_width = 0;
    }else if(p.x > center.x + half_width){
        center.x = (p.x + (center.x - half_width))/2;
        half_width = p.x - center.x; 
    }else if(p.x < center.x - half_width){
        center.x = (p.x + (center.x + half_width))/2;
        half_width = center.x - p.x;
    }

    if(isnan(center.y)){
        center.y = p.y;
        half_height = 0;
    }else if(p.y > center.y + half_height){
        center.y = (p.y + (center.y - half_height))/2;
        half_height = p.y - center.y;
    }else if(p.y < center.y - half_height){
        center.y = (p.y + (center.y + half_height))/2;
        half_height = center.y - p.y;
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

ostream& geometry::operator<<(ostream& sink, const Bounds& b){
    sink << " @" << b.center << " \u00B1" << b.half_height;
    return sink;
}
