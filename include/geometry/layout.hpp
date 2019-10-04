// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GEOMETRY_SQUARE_GRID_LAYOUT_HPP_
#define _GEOMETRY_SQUARE_GRID_LAYOUT_HPP_

#include <string>

#include "point.hpp"
#include "bounds.hpp"

namespace terrain::geometry {

///! \brief SquareLayout is used to encapsulate common logic about how to layout a square grid
class Layout {
public:
    constexpr Layout();
    constexpr Layout(const Bounds& _bounds, const double _precision);
    

    // constexpr Layout& operator=(Layout layout) { 

    // constexpr geometry::Point anchor() const;


    // // NYI
    // bool operator!=(const Bounds& other) const;
    // bool operator==(const Bounds& other) const;

    inline std::string str() const { return bounds.str() \
                                        + ", dim: " + std::to_string(dimension)\
                                        + ", prcn: " + std::to_string(precision) \
                                        + ", sz: " + std::to_string(size); }

// constants
public:
    // used for comparisons
    constexpr static double epsilon = 1e-6;
    // default values
    constexpr static Bounds default_bounds = {{0,0}, 1.0};
    constexpr static size_t default_dimension = 1.0;
    constexpr static double default_precision = 1.0;
    constexpr static size_t default_size = 1.0;

    // constexpr static Layout default_layout = {{{0,0}, 1}, 1};
    
private:
    ///! dimension * precision = width
    ///! \brief snaps this precision to match the next-power-of-2 dimension that covers the width
    constexpr static size_t snap_dimension(const double width, const double precision);

    ///! \brief
    constexpr static double snap_precision(const double width, const double dimension);

// properties
public:
    const geometry::Bounds bounds;
    const size_t dimension;
    const double precision;
    const size_t size;
};

} // namespace terrain::geometry

#include "layout.inl"

#endif // #ifdef _GEOMETRY_SQUARE_GRID_LAYOUT_HPP_
