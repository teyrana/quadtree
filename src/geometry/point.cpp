// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>

#include "geometry/point.hpp"

using geometry::Point;

Point::Point() {
  this->x = NAN;
  this->y = NAN;
}

Point::Point(double _x, double _y) {
  this->x = _x;
  this->y = _y;
}

void Point::set(double _x, double _y) {
  this->x = _x;
  this->y = _y;
}

void Point::clear(){
  this->x = NAN;
  this->y = NAN;
}
