// The MIT License 
// (c) 2019 Daniel Williams

#include "layout.hpp"
using terrain::geometry::Layout;

// these three functions need to be defined "in" the header, because
// they are constexpr / inline, and need to be _defined_ in every
// translation unit they #include into.  (see the #include of this
// file, at the bottom of the grid_layout file.

constexpr Layout::Layout() 
    : precision(1.), width(1.), x(0.), y(0.),  // primary fields
      dimension(1), half_width(0.5), padding(64), size(1)   // derived fields
{}

constexpr Layout::Layout(const double _precision, const double _x, const double _y, const double _width)
    : precision(snap_precision(_precision,_width)), width(_width), x(_x), y(_y),     // primary fields
      dimension(width/precision), half_width(width/2), padding(calculate_padding(dimension)), size(dimension*dimension)     // derived fields
{
    assert( dimension <= maximum_supported_dimension );
}

constexpr uint8_t Layout::calculate_padding( const double dimension ){
    size_t power = 1;
    size_t padding = 64;
    // this loop basically calculates log2(dimension), but in constexpr 
    while( dimension > power){
        power <<= 1;
        padding -= 2;
    }

    return padding;
}

constexpr double Layout::snap_precision( const double precision, const double width){
    const double dimension_estimate = width / precision;

    // calculate the next-higher power-of-2, in constexpr form
    size_t power = 1;
    while( dimension_estimate > power){
        power = power << 1;
    }

    return width / power;
}

constexpr uint64_t Layout::interleave( const uint32_t input) const {
    uint64_t word = input;
    word = (word ^ (word<<16)) & 0x0000ffff0000ffff;
    word = (word ^ (word<<8))  & 0x00ff00ff00ff00ff;
    word = (word ^ (word<<4))  & 0x0f0f0f0f0f0f0f0f;
    word = (word ^ (word<<2))  & 0x3333333333333333; 
    word = (word ^ (word<<1))  & 0x5555555555555555; 
    return word;
}

constexpr index_t Layout::rhash( const double x_p, const double y_p) const {
    const uint32_t i = (x_p - x + half_width)/precision;
    const uint32_t j = (y_p - y + half_width)/precision;
    return rhash(i,j);
}

constexpr index_t Layout::rhash( const uint32_t i, const uint32_t j) const {
    return i + j*dimension;
}

constexpr index_t Layout::zhash( const double x_p, const double y_p) const {
    const uint32_t i = (x_p - x + half_width)/precision;
    const uint32_t j = (y_p - y + half_width)/precision;
    return zhash(i,j);
}

constexpr index_t Layout::zhash( const uint32_t i, const uint32_t j) const {
    return (interleave(i) + (interleave(j) << 1)) << padding;
}
