#include <Eigen/Geometry>

// #include <nlohmann/json/json.hpp>

//#include "geometry/bounds.hpp"
#include "geometry/polygon.hpp"

#include "terrain.hpp"

using terrain::geometry::Polygon;

using nlohmann::json;


namespace terrain {

nlohmann::json generate_diamond( const double width,
                                 const double desired_precision)
{
    const double width_2 = width /2;
    return {{"bounds",  {{"x", width_2}, {"y", width_2}, {"width", width}}},
            {"precision", desired_precision},
            {"allow", {{{  width, width_2},
                        {width_2,   width},
                        {      0, width_2},
                        {width_2,       0}}}}};
}

}; // namespace terrain
