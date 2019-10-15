// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_HPP_
#define _QUAD_TREE_HPP_

#define QUAD_TREE_VERSION "0.0.1"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/bounds.hpp"
#include "geometry/cell_value.hpp"
#include "geometry/layout.hpp"

#include "quadtree/node.hpp"

using std::unique_ptr;

using namespace terrain::geometry;

namespace terrain::quadtree {

/**
 * Datastructure: A point Quad Tree for representing 2D data. Each
 * region has the same ratio as the bounds for the tree.
 * 
 * The implementation currently requires pre-determined bounds for data as it
 * can not rebalance itself to that degree.
 */
class Tree {
public:
    /**
     * Constructs a new quad tree, centered at 0,0 and 1024 units wide, square
     * 
     * Use this constructor if the tree will be loaded from a config file, and the initialization values do not matter.
     */
    Tree();
    
    /**
     * Constructs a new quad tree.
     *
     * @param x, y coordinates of tree's center point.
     * @param tree width.  Tree is square. (i.e. height === width)
     */
    Tree(const Bounds& _bounds, const double _precision);

    /**
     *  Releases all memory associated with this quad tree.
     */
    ~Tree();

    static size_t calculate_complete_tree(const size_t height);

    /**
     * Returns true if the point at (x, y) exists in the tree.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {bool} Whether the tree contains a point at (x, y).
     */
    bool contains(const Eigen::Vector2d& p) const;

    /**
     * Draws a simple debug representation of this tree to the given
     * output stream. 
     *
     */
    void debug_tree() const;

    /**
     * Gets the value of the point at (x, y).  If the point is not close to the center of a node, this function interpolates or extrapolates an appropriate value.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {cell_value_t} The resultant value
     */
    cell_value_t interp(const Eigen::Vector2d& at) const;

    ///! \brief sets all leaf nodes to the given value
    ///! \param fill_value - value to write
    void fill(const cell_value_t fill_value);

    /**
     * Get the overall bounds of this tree
     *
     * @return Bounds object describing the tree's overall bounds.
     */
    const Bounds& get_bounds() const;

    size_t get_dimension() const ;

    size_t get_height() const;

    const Layout& get_layout() const;

    double get_load_factor() const; 
    
    size_t get_memory_usage() const;

    double get_precision() const;

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void load_polygon(const std::vector<Eigen::Vector2d>& source);

    /**
     * Loads a representation of a tree from the data source.  The the form source is assumed to
     * contain a serialization of a tree, as represented in valid json.  That is, in the same
     * format as Tree::serialize(...).
     *
     * @param tree - input json object containing a tree structure
     */
    bool load_tree(const nlohmann::json& tree);

    void prune();

    ///! \brief resets _the tree_ to fully populate the bounds at the given precision
    ///! 
    ///! \param bounds - new bounds to describe
    ///! \param precision - describe the bounds to at least this precision
    void reset(const Bounds bounds, const double new_precision);
    
    /**
     * Gets the value of the point at (x, y) or null if the point is empty.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {cell_value_t} opt_default The default value to return if the node doesn't
     *                 exist.
     * @return {cell_value_t} The value of the node, if available; or the default value.
     */
    cell_value_t search(const Eigen::Vector2d& p) const;

    cell_value_t& search(const Eigen::Vector2d& p);
    size_t size() const;

    ///! \brief generates a json structure, describing the tree itself
    nlohmann::json to_json_tree() const;

    size_t get_width() const;

    bool write_png(const std::string filename) const;

private:
    static bool is_perimeter_cell(const Bounds& root_bounds, const Bounds& near_bounds);

private:
    ///! the data layout this tree represents
    std::unique_ptr<geometry::Layout> layout;

    unique_ptr<terrain::quadtree::Node> root;

private:
    friend class QuadTreeTest_ConstructDefault_Test;
    friend class QuadTreeTest_LoadValidTree_Test;
    friend class QuadTreeTest_CalculateLoadFactor_Test;
    friend class QuadTreeTest_LoadGridFromJSON_Test;
    friend class QuadTreeTest_WriteLoadCycle_Test;
    friend class QuadTreeTest_SearchExplicitTree_Test;
    friend class QuadTreeTest_InterpolateTree_Test;

};
} // namespace terrain::quadtree

#endif // _QUADTREE_QUAD_TREE_HPP_
