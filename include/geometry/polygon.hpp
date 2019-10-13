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

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/bounds.hpp"

using terrain::geometry::Bounds;

namespace terrain::geometry {

class Polygon {
public:
    Polygon();
    Polygon(size_t initial_capacity);
    Polygon(nlohmann::json doc);
    Polygon(std::vector<Eigen::Vector2d>& init);
    Polygon(std::initializer_list<Eigen::Vector2d> init);

    // clears the internal point vector
    void clear();

    void emplace(const Eigen::Vector2d p);

    // Retrieves the precomputed center of the polygon:
    // Currently, this is a naive, unweighted average of the polygon points.
    // \return 2D x,y point of the polygon's center
    const Bounds& get_bounds() const;


    // Retrieves the precomputed center of the polygon:
    // Currently, this is a naive, unweighted average of the polygon points.
    // \return was the load successful? 
    // \sidef
    bool load(std::vector<Eigen::Vector2d> source);

    bool load(nlohmann::json doc);

    Eigen::Vector2d& operator[](const size_t index);

    const Eigen::Vector2d& operator[](const size_t index) const;

    size_t size() const;
    
    // dumps the contains points to stderr
    // \param title - text to print in the output header
    // \param pts - set of points to dump
    void write_yaml(std::ostream& sink, std::string indent="") const; 

protected:
    void complete();

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
    std::vector<Eigen::Vector2d> points;    ///< Main data store for this class.  Contains the vertices of the polygon
    Bounds bounds;                ///< x,y bounds at the (approximate) center of the polygon

private:
    friend class PolygonTest_DefaultConfiguration_Test;
    friend class PolygonTest_LoadList_5Point_Test;
    friend class PolygonTest_LoadList_DiamondRhombus_Test;
    
    // friend class PolygonTests_TestHandedness_Test;
    // friend class PolygonTests_InBoundingBoxByX_Test;
    // friend class PolygonTests_InConvexBoundariesByX_Test;
    // friend class PolygonTests_InConcaveBoundariesByX_Test;
    // friend class PolygonTests_InConcaveBoundariesByY_Test;
};

} // namespace terrain::geometry

#endif  // #endif _GEOMETRY_POLYGON_HPP_
