// The MIT License 
// (c) 2019 Daniel Williams

#include "bounds.hpp"

namespace terrain::geometry {

constexpr Layout::Layout(): 
    bounds(default_bounds), 
    dimension(default_dimension), 
    precision(default_precision),
    size(default_size)
{}

constexpr Layout::Layout(const Bounds& _bounds, const double _precision): 
    bounds(_bounds), 
    dimension(snap_dimension(bounds.width(), _precision)), 
    precision(snap_precision(bounds.width(), dimension)),
    size(dimension*dimension)
{}

// constexpr Point Layout::anchor() const {
//     return bounds.center.sub({bounds.half_width, bounds.half_width});
// }

///! dimension * precision = width
///! \brief snaps this precision to match the next-power-of-2 dimension that covers the width
constexpr size_t Layout::snap_dimension(const double width, const double precision){
    const double dimension_estimate = width / precision;

    // coerce the dimension to the next-higher power-of-2
    size_t power = 1;
    while( dimension_estimate > power){
        power = power << 1;
    }
    return power;
}

///! dimension * precision = width
constexpr double Layout::snap_precision(const double width, const double dimension){
    return width / dimension;
}

} // namespace terrain::geometry