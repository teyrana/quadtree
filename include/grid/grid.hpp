// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GRID_GRID_HPP_
#define _GRID_GRID_HPP_

#include <cmath>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>

#include "nlohmann/json/json.hpp"

#include "geometry/bounds.hpp"
#include "geometry/point.hpp"
#include "geometry/polygon.hpp"

#include "cell_value.hpp"

using terrain::geometry::Bounds;
using terrain::geometry::Point;
using terrain::geometry::Polygon;

using terrain::cell_value_t;

namespace terrain::grid {

class Grid {
public:
    /**
     * Constructs a new quad tree, centered at 0,0 and 1024 units wide, square
     * 
     * Use this constructor if the tree will be loaded from a config file, and the initialization values do not matter.
     */
    Grid();
    
    /**
     * Constructs a new 2d square grid
     *
     * @param {_dim} number of cells along each dimensions of this grid
     * @param {_bounds} bounds which this grid will represent
     */
    Grid(const Bounds& _bounds, double precision);

    /**
     *  Releases all memory associated with this quad tree.
     */
    ~Grid(){};

    const Point anchor() const;


    /**
     * Returns true if the point at (x, y) exists in the tree.
     *
     * @param {Point} The p to test
     * @return {bool} Whether the tree contains the given point
     */
    bool contains(const Point& p) const;

    ///! \brief Draws a simple debug representation of this grid to stderr
    void debug() const;

    size_t dimension() const;

    ///! \brief sets the entire grid to the given value
    ///! \param fill_value - fill value for entire grid
    void fill(const cell_value_t fill_value);

    ///! \brief Fills the _interior_ of the given polygon with the given value.
    ///! 
    ///! @param source - polygon defining the fill araea. Assumed to be closed, CCW, and non-intersecting
    ///! @param fill_value -fill value for area
    void fill(const Polygon& source, const cell_value_t fill_value);

    /** 
     *                   +---+---+     +---+---+
     * Index:            | 0 | 1 | ... |n-1| n |
     *                   +---+---+     +---+---+
     *                   |                     |
     * Coordinate:     (center - w/2)       (center + w/2)

    ///! \warning !! DOES NOT CHECK BOUNDS !!
     */
    cell_value_t& get_cell(const size_t xi, const size_t yi);
    cell_value_t get_cell(const size_t xi, const size_t yi) const ;
   
    /**
     * Get the overall bounds of this tree
     *
     * @return Bounds object describing the tree's overall bounds.
     */
    const Bounds& get_bounds() const;

    double get_precision() const;

    bool load_grid(nlohmann::json& doc);

    void reset();
    void reset(const Bounds bounds, const double new_precision);

    /**
     * Sets the value at an (x, y) point
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {V} value The value associated with the point.
     */
    void set(const double x, const double y, const cell_value_t new_value);

    ///! \brief Access the value at an (x, y) point
    ///!
    ///! \param point - the x,y coordinates to search at:
    ///! \return reference to the cell value
    cell_value_t& search(const Point& p);

    size_t size() const;

    bool to_png(const std::string filename) const;

    double width() const {return bounds.width();}

private:
    static double snap_precision(const double width, const double precision);
    size_t x_to_index(const double x) const;
    size_t y_to_index(const double y) const;

public:
    ///! the bounds that this grid covers
    Bounds bounds;

    constexpr static double epsilon = 1e-6;
    constexpr static Bounds default_bounds = {{0,0}, 32};

    ///! the spacing of each cell === center-to-center distance. === cell-width.
    double precision;

    // raw array:  2D addressing is performed through the class methods
    std::vector<cell_value_t> storage;

private:
    friend class GridTest_SnapPrecision_Test;
    friend class GridTest_XYToIndex_Test;

};

}; // namespace terrain::grid

#endif // _GRID_HPP_
