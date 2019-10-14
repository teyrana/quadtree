// The MIT License 
// (c) 2019 Daniel Williams

#include "grid_layout.hpp"
using terrain::geometry::GridLayout;

// these three functions need to be defined "in" the header, because
// they are constexpr / inline, and need to be _defined_ in every
// translation unit they #include into.  (see the #include of this
// file, at the bottom of the grid_layout file.

constexpr GridLayout::GridLayout() 
    : precision(1), x(0), y(0), width(1),        // primary fields
      dimension(1), half_width(0.5), size(1)     // derived fields
{}

constexpr GridLayout::GridLayout(const double _precision, const double _x, const double _y, const double _width)
    : precision(snap_precision(_precision,_width)), x(_x), y(_y), width(_width),     // primary fields
      dimension(width/precision), half_width(width/2), size(dimension*dimension)     // derived fields
{}

constexpr double GridLayout::snap_precision( const double precision, const double width){
    const double dimension_estimate = width / precision;

    // coerce the dimension to the next-higher power-of-2
    size_t power = 1;
    while( dimension_estimate > power){
        power = power << 1;
    }
    return width / power;
}

