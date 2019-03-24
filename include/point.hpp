// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_POINT_HPP_
#define _QUAD_TREE_POINT_HPP_

namespace quadtree {
class Point {
public:
    // this is just POD, with some convenient methods:
    Point();
    Point(double x, double y);

    void set(double x, double y);
    void clear();

    double x;
    double y;

};
} // namespace quadtree
#endif
