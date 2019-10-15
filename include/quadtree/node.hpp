// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUADTREE_NODE_HPP_
#define _QUADTREE_NODE_HPP_

#include <cstdint>
#include <iostream>
#include <string>
#include <memory>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/cell_value.hpp"
#include "geometry/bounds.hpp"

using terrain::geometry::cell_value_t;

namespace terrain::quadtree {

enum NodeType { EMPTY, LEAF, BRANCH };
enum NodeQuadrant {NW, NE, SW, SE};

class Node {
public:
    Node() = delete;
    // Node(const Eigen::Vector2d& _center, const double _height);
    Node(const geometry::Bounds& _bounds, const cell_value_t value);

    //Node(double cx, double cy, double _new_height, double h);  //alternate function signature

    ~Node();

    bool contains(const Eigen::Vector2d& at) const;

    void draw(std::ostream& sink, const std::string& prefix, const std::string& as) const ;

    void fill(const cell_value_t fill_value);

    const geometry::Bounds& get_bounds() const;
    const Eigen::Vector2d& get_center() const;
    size_t get_count() const;
    size_t get_height() const;
    Node* get_northeast() const;
    Node* get_northwest() const;
    Node* get_southeast() const;
    Node* get_southwest() const;

    cell_value_t& get_value();
    cell_value_t get_value() const;

    /**
     * Performs the low-level interpolation between this node and another node, at the requested location
     *
     * @param {Point} the x,y coordinates to interpolate at.
     * @param {quadtree::Node} n2 the other node to interpolate
     * @return {cell_value_t} The resultant value
     */
    cell_value_t interpolate_linear(const Eigen::Vector2d& at, const Node& n2) const;

    /** 
     * Performs bilinear-interpolation: 
     * http://en.wikipedia.org/wiki/Bilinear_Interpolation
     * 
     * @param {Point} the x,y coordinates to interpolate at.
     * @param {quadtree::Node} xn x-neighbor node to interpolate with
     * @param {quadtree::Node} dn diagonal-neighbor node to interpolate with
     * @param {quadtree::Node} yn y-neighbor node to interpolate with
     * @return {cell_value_t} The resultant value
     */
    cell_value_t interpolate_bilinear(const Eigen::Vector2d& at, 
                                      const Node& xn,
                                      const Node& dn,
                                      const Node& yn) const;

    bool load(const nlohmann::json& doc);

    constexpr static double snap_center_distance = 0.5;
    bool nearby(const Eigen::Vector2d& p) const;
    bool nearby(const Eigen::Vector2d& p, const double threshold) const;

    double operator[](const size_t index);
    const double operator[](const size_t index) const;

    ///! \brief coalesce groups of leaf nodes with identice values (for some value of "identical")
    void prune();

    Node& search(const Eigen::Vector2d& at);

    void split();

    void split(const double precision);

    void reset();

    bool is_leaf() const;

    void set_value(cell_value_t new_value);

    nlohmann::json to_json() const;
    std::string to_string() const;

private:
    // Important: this *should* be const -- that prevents this node
    // from effectively growing or shrinking, within the context of
    // the tree
    const geometry::Bounds bounds;

    // By design, any given node will only cantain (a) children or (b) a value.
    // => If the following uptr, `northeast` has a value, the union will contain pointers.
    // => if 'northeast' is empty / null => the union contains leaf-node-values
    // defined in CCW order:  NE -> NW -> SW -> SE
    std::unique_ptr<Node> northeast; //ne;
    std::unique_ptr<Node> northwest; //nw;
    std::unique_ptr<Node> southwest; //sw;
    std::unique_ptr<Node> southeast; //se;

    cell_value_t value;

private:
    friend std::ostream& operator<<(std::ostream& s, const Node& n);
private:
    friend class NodeTest_ConstructDefault_Test;
    friend class NodeTest_ConstructByCenterAndSize_Test;
    friend class NodeTest_SetGet_Test;
    friend class NodeTest_SplitNode_Test;
    friend class NodeTest_InterpolateValue_Test;
};

} // namespace terrain::quadtree

#endif // #ifndef _QUADTREE_NODE_HPP_
