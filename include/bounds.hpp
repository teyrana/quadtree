// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_BOUNDS_HPP_
#define _QUAD_TREE_BOUNDS_HPP_

namespace quadtree {
class Bounds {
public:
    Bounds(double min_x, double min_y, double max_x, double max_y);
    void extend(double x, double y);


protected:
    double max_x;
    double max_y;
    double min_x;
    double min_y;

};
} // namespace quadtree
#endif
