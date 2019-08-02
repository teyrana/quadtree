// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_NODE_HPP_
#define _QUAD_TREE_NODE_HPP_

#include <cstdint>
#include <string>
#include <memory>

#include <nlohmann/json/json_fwd.hpp>

#include "bounds.hpp"
#include "point.hpp"
#include "node_value.hpp"

namespace quadtree {

enum NodeType { EMPTY, LEAF, BRANCH };
enum NodeQuadrant {NW, NE, SW, SE};

class Node {
public:
    Node();
    Node(const Point& _center, const double _width);
    Node(const Point& _center, const double _height, const double _width);
    //Node(double cx, double cy, double _new_height, double h);  //alternate function signature

    ~Node();

    const Bounds& get_bounds() const; 
    node_value_t get_value() const;
    Node* get_northeast();
    Node* get_northwest();
    Node* get_southeast();
    Node* get_southwest();
    void load(nlohmann::json doc);

    Node* search(double x, double y);

    void split();
    
    void reset();
    
    bool is_leaf() const;

    void set_value(quadtree::node_value_t new_value);

    nlohmann::json to_json() const;
    std::string to_string() const;
    
private:
    // Important: this *should* be const -- that prevents this node
    // from effectively growing or shrinking, within the context of
    // the tree
    const quadtree::Bounds bounds;

    // By design, any given node will only cantain (a) children or (b) a value.
    // => If the following uptr, `northeast` has a value, the union will contain pointers.
    // => if 'northeast' is empty / null => the union contains leaf-node-values
    std::unique_ptr<Node> northeast; //ne;
    union {
        // Option A:
        struct {
	    // defined in CCW order:  NE -> NW -> SW -> SE
            std::unique_ptr<Node> northwest; //nw;
            std::unique_ptr<Node> southwest; //sw;
            std::unique_ptr<Node> southeast; //se;
        } quadrants;

        // Option B:
        quadtree::node_value_t value;

    };

private:
    // does not work, currently
    friend class NodeTest_ConstructDefault_Test;
    friend class NodeTest_ConstructByCenterAndSize_Test;
    friend class NodeTest_SplitNode_Test;

};
} // namespace quadtree
#endif // _QUAD_TREE_NODE_HPP_
