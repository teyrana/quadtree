// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>

#include "geometry/point.hpp"
#include "geometry/segment.hpp"

using geometry::Point;
using geometry::Segment;


Segment::Segment() {}

Segment::Segment(const Point& _start, const Point& _end):
  start(_start), end(_end)
{}

void Segment::clear(){
  this->start.clear();
  this->end.clear();
}
