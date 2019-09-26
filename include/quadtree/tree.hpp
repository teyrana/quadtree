// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_HPP_
#define _QUAD_TREE_HPP_

#define QUAD_TREE_VERSION "0.0.1"

#include <cstdlib>
#include <string>
#include <iostream>

#include <cmath>
#include <memory>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/bounds.hpp"
#include "geometry/point.hpp"
#include "cell_value.hpp"
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
     * @param {Point} x, y coordinates of tree's center point.
     * @param {double} tree width.  Tree is square. (i.e. height === width)
     */
    Tree(const Bounds& _bounds, const double _precision);

    /**
     *  Releases all memory associated with this quad tree.
     */
    ~Tree();

        
    /**
     * Returns true if the point at (x, y) exists in the tree.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {bool} Whether the tree contains a point at (x, y).
     */
    bool contains(const geometry::Point& p) const;

    void cull();

    /**
     * Draws a simple debug representation of this tree to the given
     * output stream. 
     *
     * @param {std::ostream&} output stream to write data to
     */
    void debug() const; 

    size_t dimension() const;

    /**
     * Gets the value of the point at (x, y).  If the point is not close to the center of a node, this function interpolates or extrapolates an appropriate value.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {cell_value_t} The resultant value
     */
    cell_value_t interp(const Point& at) const;

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

    double get_precision() const { return precision;}

    size_t get_height() const;

    void grow(const double precision);

    /**
     * Loads a representation of a tree from the data source.  The the form source is assumed to
     * contain a serialization of a tree, as represented in valid json.  That is, in the same
     * format as Tree::serialize(...).
     *
     * @param {std::istream} input stream containing the serialization text
     */
    bool load_grid(nlohmann::json& grid);

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void load_polygon(const std::vector<Point>& source);

    bool load_tree(nlohmann::json& tree);

    // /**
    //  * Sets the value of an (x, y) point within the quad-tree.
    //  *
    //  * @param {double} x The x-coordinate.
    //  * @param {double} y The y-coordinate.
    //  * @param {V} value The value associated with the point.
    //  */
    // void set(const double x, const double y, const cell_value_t new_value);

    /**
     * Gets the value of the point at (x, y) or null if the point is empty.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {cell_value_t} opt_default The default value to return if the node doesn't
     *                 exist.
     * @return {cell_value_t} The value of the node, if available; or the default value.
     */
    cell_value_t search(const Point& p) const;

    cell_value_t& search(const Point& p);

    /**
     * Removes a point from (x, y) if it exists.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {V} The value of the node that was removed, or null if the
     *         node doesn't exist.
     */
    bool remove(double x, double y);

    ///! \brief resets _the tree_ to fully populate the bounds at the given precision
    ///! 
    ///! \param bounds - new bounds to describe
    ///! \param precision - describe the bounds to at least this precision
    void reset(const Bounds bounds, const double new_precision);
    
    ///! \brief generates a json structure, describing the tree itself
    nlohmann::json to_json_tree() const;

    size_t width() const {return bounds.width();}

    bool write_png(const std::string filename) const;

    // size_t x_to_index(double x) const;
    // size_t y_to_index(double y) const;


private:
    void debug_grid(std::ostream& sink) const;

    static bool is_perimeter_cell(const Bounds& root_bounds, const Bounds& near_bounds);
    
    // /**
    //  * Traverses the tree depth-first, with quadrants being traversed in clockwise
    //  * order (NE, SE, SW, NW).  The provided function will be called for each
    //  * leaf node that is encountered.
    //  * @param {Tree.Node} node The current node.
    //  * @param {function(Tree.Node)} fn The function to call
    //  *     for each leaf node. This function takes the node as an argument, and its
    //  *     return value is irrelevant.
    //  * @private
    //  */
    // void traverse(Node<V> node, Func<V> func);


    //  * Inserts a point into the tree, updating the tree's structure if necessary.
    //  * @param {.Tree.Node} parent The parent to insert the point
    //  *     into.
    //  * @param {Tree.Point} point The point to insert.
    //  * @return {bool} True if a new node was added to the tree; False if a node
    //  *     already existed with the correpsonding coordinates and had its value
    //  *     reset.
    //  * @private
    //  */
    // private bool insert(Node<V> parent, Point<V> point);

public:
    static constexpr Bounds default_bounds = {Point(0.,0.), 32};

private:
    Bounds bounds;
    unique_ptr<terrain::quadtree::Node> root;
    double precision;

private:
    friend class QuadTreeTest_ConstructDefault_Test;
    friend class QuadTreeTest_LoadValidTree_Test;
    friend class QuadTreeTest_LoadGridFromJSON_Test;
    friend class QuadTreeTest_WriteLoadCycle_Test;
    friend class QuadTreeTest_TestSearchExplicitTree_Test;
    friend class QuadTreeTest_TestInterpolateTree_Test;
    friend class QuadTreeTest_TestSearchImplicitTree_Test;

};
} // namespace terrain::quadtree

#endif // _QUADTREE_QUAD_TREE_HPP_
