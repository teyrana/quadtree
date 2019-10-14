#include <Eigen/Geometry>

// #include <nlohmann/json/json.hpp>

//#include "geometry/bounds.hpp"
#include "geometry/polygon.hpp"

#include "terrain.hpp"

using terrain::geometry::Polygon;

using nlohmann::json;


namespace terrain {

nlohmann::json generate_diamond( const double boundary_width,
                                 const double diamond_width,
                                 const double desired_precision)
{
    const double center = boundary_width/2;

    return {{"bounds",  {{"x", center}, {"y", center}, {"width", boundary_width}}},
            {"precision", desired_precision},
            {"allow", {{{center + diamond_width, center},
                        {center                , center + diamond_width},
                        {center - diamond_width, center},
                        {center                , center - diamond_width}}}}};
}

}; // namespace terrain
