// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GRID_HPP_
#define _GRID_HPP_

#include <array>
#include <cmath>
#include <memory>
#include <cstdlib>
#include <string>
#include <iostream>

#include "geometry/bounds.hpp"
#include "geometry/point.hpp"
#include "geometry/polygon.hpp"
#include "node.hpp"
#include "node_value.hpp"

typedef uint8_t grid_value_t;

using geometry::Bounds;
using geometry::Point;
using geometry::Polygon;

namespace quadtree {

class Grid {
public:
    /**
     * Constructs a new quad tree, centered at 0,0 and 1024 units wide, square
     * 
     * Use this constructor if the tree will be loaded from a config file, and the initialization values do not matter.
     */
    Grid() = delete;
    
    /**
     * Constructs a new 2d square grid
     *
     * @param {_dim} number of cells along each dimensions of this grid
     * @param {_bounds} bounds which this grid will represent
     */
    Grid(double size, double spacing, const Point center);
    
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


    /**
     * Draws a simple debug representation of this tree to the given
     * output stream. 
     *
     * @param {std::ostream&} output stream to write data to
     */
    void draw(std::ostream& ) const;

    // sets grid to all zeros
    void fill(const grid_value_t _value);

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void fill(const Polygon& source, const grid_value_t fill_value);

    /** 
     *                   +---+---+     +---+---+
     * Index:            | 0 | 1 | ... |n-1| n |
     *                   +---+---+     +---+---+
     *                   |                     |
     * Coordinate:     (center - w/2)       (center + w/2)
     */
    grid_value_t get(size_t xi, size_t yi) const;

    /**
     * Sets the value of an (x, y) point within the quad-tree.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {V} value The value associated with the point.
     */
    void set(const double x, const double y, const grid_value_t new_value);

    /**
     * Gets the value of the point at (x, y) or null if the point is empty.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {grid_value_t} opt_default The default value to return if the node doesn't
     *                 exist.
     * @return {grid_value_t} The value of the node, if available; or the default value.
     */
    grid_value_t search(const Point& p);

    /**
     * Get the overall bounds of this tree
     *
     * @return Bounds object describing the tree's overall bounds.
     */
    const Bounds& get_bounds() const;

    grid_value_t get_default_value(const Point& at) const;

    /**
     * Loads a representation of a tree from the data source.  The the form source is assumed to
     * contain a serialization of a tree, as represented in valid json.  That is, in the same
     * format as QuadTree::serialize(...).
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void load_grid(std::istream& source);

    grid_value_t& operator()(size_t xi, size_t yi);

    /**
     * Removes a point from (x, y) if it exists.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {V} The value of the node that was removed, or null if the
     *         node doesn't exist.
     */
    bool remove(double x, double y);

    /**
     * Performs a deep reset of all tree data.
     */
    void reset();

    /**
     * Writes a json-serialization of the tree to the given out-stream
     *
     * @param {std::ostream&} destination for the serialization string
     */
    void serialize(std::ostream& sink) const;

    size_t size() const {return dimension*dimension;}

    bool to_png(const std::string filename) const;

    size_t width() const {return dimension;}

private:
    size_t grid_to_storage(size_t xi, size_t yi) const;
    size_t x_to_index(const double x) const;
    size_t y_to_index(const double y) const;

public:
    ///! the bounds that this grid covers
    const geometry::Bounds bounds;

    // number of cells in each dimension of the square grid
    const size_t dimension;

    ///! the spacing of each cell. === center-to-center distance. === cell-width.
    const double spacing;

    // raw array:  2D addressing is performed through the class methods
    const std::unique_ptr<grid_value_t[]> storage;

private:
    friend class GridTest_XYToIndex_Test;

};
} // namespace quadtree

#endif // _QUAD_TREE_HPP_
