// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_POINT_HPP_
#define _GEOMETRY_POINT_HPP_

namespace geometry {
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
