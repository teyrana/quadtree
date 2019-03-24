// The MIT License 
// (c) 2019 Daniel Williams

#include "bounds.hpp"

#include <cmath>

using std::fmax;
using std::fmin;

using quadtree::Bounds;

Bounds::Bounds(double _min_x, double _min_y, double _max_x, double _max_y){
    this->max_x = _max_x;
    this->max_y = _max_y;
    this->min_x = _min_x;
    this->min_y = _min_y;
}

void Bounds::extend(double x, double y){
  min_x = fmin(x, min_x);
  max_x = fmax(x, max_x);
  
  min_y = fmin(y, min_y);
  max_y = fmax(y, max_y);
}

