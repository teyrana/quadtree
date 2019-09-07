// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_POLYGON_HPP_
#define _GEOMETRY_POLYGON_HPP_

#include <initializer_list>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "geometry/bounds.hpp"
#include "geometry/point.hpp"

using geometry::Bounds;
using geometry::Point;

typedef std::vector<Point>::const_iterator cpiter;

namespace geometry {
class Polygon {
public:
    Polygon();
    Polygon(std::string new_name);
    Polygon(std::string _name, std::initializer_list<Point> init);



    // clears the internal point vector
    void clear();

    bool contains(double x, double y) const;
    
    // Retrieves the precomputed center of the polygon:
    // Currently, this is a naive, unweighted average of the polygon points.
    // \return 2D x,y point of the polygon's center
    const Bounds& get_bounds() const;


    // Retrieves the precomputed center of the polygon:
    // Currently, this is a naive, unweighted average of the polygon points.
    // \return was the load successful? 
    // \sidef
    bool load(const bool as_inclusive, std::vector<Point> source);

    const std::vector<Point>& get_points() const { return points;}
    
    //std::vector<Point>& segments();
    
    size_t size() const;
    
    // dumps the contains points to stderr
    // \param title - text to print in the output header
    // \param pts - set of points to dump
    void write_yaml(std::ostream& sink, std::string indent="") const; 

protected:
    // if necessary, adds an extra point to the end of the polygon-point vector to wrap it back to the first point.
    // this is not strictly operationally necessary, but it simplifies most of the algorithms that need to iterate over the points.
    void enclose_polygon();

    // \brief isRightHanded()
    // calculates if the polygon is defined in a right-handed // CCW direction.
    // runs in O(n).
    //
    // See Also: https://en.wikipedia.org/wiki/Shoelace_formula
    //
    // \returns true: polgyon is right-handed.  false: left-handed
    bool is_right_handed() const;

    void set_default();

    void update_bounds();

protected: // Configuration parameters
    std::string name;             ///< Arbitrary string describing this polygon. For humans.
    std::vector<Point> points;    ///< Main data store for this class.  Contains the vertices of the polygon
    bool inclusive;               ///< flags  if this polygon excludes area or includes area
    Bounds bounds;                ///< x,y bounds at the (approximate) center of the polygon

private:
    friend class PolygonTest_DefaultConfiguration_Test;
    friend class PolygonTest_LoadList_Test;
    friend class PolygonTest_LoadInitializerList_Test;
    
    // friend class PolygonTests_TestHandedness_Test;
    // friend class PolygonTests_InBoundingBoxByX_Test;
    // friend class PolygonTests_InConvexBoundariesByX_Test;
    // friend class PolygonTests_InConcaveBoundariesByX_Test;
    // friend class PolygonTests_InConcaveBoundariesByY_Test;
};

} // namespace geometry
#endif
