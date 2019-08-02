// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_POINT_HPP_
#define _QUAD_TREE_POINT_HPP_

namespace quadtree {
struct Point {
public:
    // class is just POD + convenient methods
    double x;
    double y;

public:
    Point();
    Point(double x, double y);

    void set(double x, double y);
    void clear();

};
} // namespace quadtree
#endif
