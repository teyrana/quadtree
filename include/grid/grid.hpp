// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GRID_GRID_HPP_
#define _GRID_GRID_HPP_

#include <cmath>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include "nlohmann/json/json.hpp"

#include "geometry/cell_value.hpp"
#include "geometry/polygon.hpp"
#include "geometry/layout.hpp"

using terrain::geometry::Layout;
using terrain::geometry::cell_value_t;
using terrain::geometry::Polygon;


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
    Grid(const Layout& _layout);

    /**
     *  Releases all memory associated with this quad tree.
     */
    ~Grid(){};

    /**
     * Returns true if the point at (x, y) exists in the tree.
     *
     * @param The p to test
     * @return Whether the tree contains the given point
     */
    bool contains(const Eigen::Vector2d& p) const;

    ///! \brief Draws a simple debug representation of this grid to stderr
    void debug() const;

    ///! \brief sets the entire grid to the given value
    ///! \param fill_value - fill value for entire grid
    void fill(const cell_value_t fill_value);

    ///! \brief Fills the _interior_ of the given polygon with the given value.
    ///! 
    ///! @param source - polygon defining the fill araea. Assumed to be closed, CCW, and non-intersecting
    ///! @param fill_value -fill value for area
    void fill(const Polygon& source, const cell_value_t fill_value);

    /**
     * Get the overall bounds of this tree
     *
     * @return Bounds object describing the tree's overall bounds.
     */
    inline const Layout& get_layout() const { return layout; }

    ///! \brief simply returns the value or reference to the internal data
    ///! \warning !! DOES NOT CHECK BOUNDS !!
    cell_value_t& get_cell(const size_t xi, const size_t yi);
    cell_value_t get_cell(const size_t xi, const size_t yi) const ;
   
    constexpr double get_load_factor() const { return 1.0; }

    size_t get_memory_usage() const;

    ///! the spacing of each cell === center-to-center distance. === cell-width.
    double get_precision() const;

    inline void prune() {};

    void reset();
    void reset(const Layout& _layout);

    ///! \brief Retrieve the value at an (x, y) Eigen::Vector2d
    ///!
    ///! \param Eigen::Vector2d - the x,y coordinates to search at
    ///! \return the cell value
    cell_value_t classify(const Eigen::Vector2d& p) const;

    ///! \brief the _total_ number of cells in this grid === (width * height)
    size_t size() const;

    ///! \brief Access the value at an (x, y) Eigen::Vector2d
    ///!
    ///! \param Eigen::Vector2d - the x,y coordinates to search at:
    ///! \return reference to the cell value
    bool store(const Eigen::Vector2d& p, const cell_value_t new_value);

    bool to_png(const std::string filename) const;

public:
    ///! the data layout this grid represents
    Layout layout;

    // raw array:  2D addressing is performed through the class methods
    std::vector<cell_value_t> storage;

private:
    friend class GridTest_SnapPrecision_Test;
    friend class GridTest_XYToIndex_Test;

};

}; // namespace terrain::grid

#endif // _GRID_HPP_
