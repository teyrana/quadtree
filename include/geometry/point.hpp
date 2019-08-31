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
    
    constexpr static double epsilon = 1e-6;

public:
    Point();
    Point(double x, double y);

    // calculates angle between these vectors in radians
    double angle(const Point * other) const;

    void clear();
    
    /**
     * 
     * @return returns the z-component of a cross-product
     */
    double cross(Point const * other) const;

    /**
     * 
     * \brief calculates angles between three points, by exploiting:
    //
    //            $ a \dot b = ||a|| ||b|| cos(\theta) $
    //
    //
    //      p2  +----------+ p3
    //         /         /
    //        /       /
    //    A  /     / B
    //      /   /
    //     / /
    // p1 +
    //
    //     p1 -> p2 === segment A
    //     p1 -> p3 === segment C
    //
    // \warning assumes polygon is already right-handed.
    // \param p1 First point in the triple sequence
    // \param p2 First point in the triple sequence
    // \param p3 First point in the triple sequence
     */
    // WARNING: PURPOSE UNKNOWN! 
    // TODO: BROKEN
    static bool curves_right(const Point& p1, const Point& p2, const Point& p3);

    double distance(const Point& other) const;

    /**
     * @return returns the magnitude of the dot product
     */
    double dot(const Point * other) const ;

    double is_right_handed(const Point * other) const;

    bool is_zero() const;

    bool near(const Point& other) const;

    double norm() const;

    void normalize();

    void set(double x, double y);

    Point subtract(const Point * other) const;
    
};
} // namespace geometry
#endif
