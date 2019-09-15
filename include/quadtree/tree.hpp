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
#include "node.hpp"
#include "node_value.hpp"

using std::unique_ptr;

using quadtree::node_value_t;
using geometry::Bounds;
using geometry::Point;

namespace quadtree {

/**
 * Datastructure: A point Quad Tree for representing 2D data. Each
 * region has the same ratio as the bounds for the tree.
 * 
 * The implementation currently requires pre-determined bounds for data as it
 * can not rebalance itself to that degree.
 */
class QuadTree {
public:
    /**
     * Constructs a new quad tree, centered at 0,0 and 1024 units wide, square
     * 
     * Use this constructor if the tree will be loaded from a config file, and the initialization values do not matter.
     */
    QuadTree();
    
    /**
     * Constructs a new quad tree.
     *
     * @param {Point} x, y coordinates of tree's center point.
     * @param {double} tree width.  Tree is square. (i.e. height === width)
     */
    QuadTree(Point center, double width);
    
    /**
     *  Releases all memory associated with this quad tree.
     */
    ~QuadTree();

        
    /**
     * Returns true if the point at (x, y) exists in the tree.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {bool} Whether the tree contains a point at (x, y).
     */
    bool contains(const double x, const double y) const;

    /**
     * Draws a simple debug representation of this tree to the given
     * output stream. 
     *
     * @param {std::ostream&} output stream to write data to
     */
    void draw(std::ostream& sink) const;
    

    /**
     * Gets the value of the point at (x, y).  If the point is not close to the center of a node, this function interpolates or extrapolates an appropriate value.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @return {node_value_t} The resultant value
     */
    node_value_t interp(const Point& at) const;

    void grow(const double precision);

    size_t height() const;

    /**
     * Loads a representation of a tree from the data source.  The the form source is assumed to
     * contain a serialization of a tree, as represented in valid json.  That is, in the same
     * format as QuadTree::serialize(...).
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void load(std::istream& source);

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void load(const std::vector<Point>& source);

    void load_grid(nlohmann::json& grid);

    void set(const Bounds bounds);

    // /**
    //  * Sets the value of an (x, y) point within the quad-tree.
    //  *
    //  * @param {double} x The x-coordinate.
    //  * @param {double} y The y-coordinate.
    //  * @param {V} value The value associated with the point.
    //  */
    // void set(const double x, const double y, const node_value_t new_value);

    /**
     * Gets the value of the point at (x, y) or null if the point is empty.
     *
     * @param {double} x The x-coordinate.
     * @param {double} y The y-coordinate.
     * @param {node_value_t} opt_default The default value to return if the node doesn't
     *                 exist.
     * @return {node_value_t} The value of the node, if available; or the default value.
     */
    node_value_t search(const double x, const double y);

    /**
     * Get the overall bounds of this tree
     *
     * @return Bounds object describing the tree's overall bounds.
     */
    const Bounds& get_bounds() const;

    node_value_t get_default_value(const Point& at) const;

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
    void write_json(std::ostream& sink) const;

private:
    static bool is_perimeter_cell(const Bounds& root_bounds, const Bounds& near_bounds);
    
    // /**
    //  * Traverses the tree depth-first, with quadrants being traversed in clockwise
    //  * order (NE, SE, SW, NW).  The provided function will be called for each
    //  * leaf node that is encountered.
    //  * @param {QuadTree.Node} node The current node.
    //  * @param {function(QuadTree.Node)} fn The function to call
    //  *     for each leaf node. This function takes the node as an argument, and its
    //  *     return value is irrelevant.
    //  * @private
    //  */
    // void traverse(Node<V> node, Func<V> func);


    //  * Inserts a point into the tree, updating the tree's structure if necessary.
    //  * @param {.QuadTree.Node} parent The parent to insert the point
    //  *     into.
    //  * @param {QuadTree.Point} point The point to insert.
    //  * @return {bool} True if a new node was added to the tree; False if a node
    //  *     already existed with the correpsonding coordinates and had its value
    //  *     reset.
    //  * @private
    //  */
    // private bool insert(Node<V> parent, Point<V> point);

private:
    unique_ptr<Node> root;

private:
    friend class TreeTest_LoadValidTree_Test;
    friend class TreeTest_LoadValidGrid_Test;
    friend class TreeTest_WriteLoadCycle_Test;
    friend class TreeTest_TestSearchExplicitTree_Test;
    friend class TreeTest_TestInterpolateTree_Test;
    friend class TreeTest_TestSearchImplicitTree_Test;

};
} // namespace quadtree

std::ostream& operator<<(std::ostream& os, const quadtree::QuadTree& tree);

#endif // _QUAD_TREE_HPP_
