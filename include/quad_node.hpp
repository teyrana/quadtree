// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_NODE_HPP_
#define _QUAD_TREE_NODE_HPP_

#include <cstdint>
#include <string>
using std::string;

#include <memory>
using std::unique_ptr;

#include "point.hpp"

namespace quadtree {

typedef uint32_t node_value_t;
    
class QuadNode {
public:
    enum NodeType { EMPTY, LEAF, BRANCH };

public:

    QuadNode(double x, double y, double w, double h);
    ~QuadNode();


    double get_height() const;
    NodeType get_type() const;
    node_value_t get_value() const;
    double get_width() const;
    double get_x() const;
    double get_y() const;
    
    
    bool is_leaf();
    bool is_empty();
    bool is_branch();

    void set_height(double h);
    void set_value(node_value_t new_value);
    void set_width(double w);
    void set_x(double x);
    void set_y(double x);

    string to_json();

//private:
public:
    NodeType nodetype = NodeType::EMPTY;
    
    //QuadNode * parent;

    // quadtree_bounds_t *bounds;

    // center point
    Point center;

    double width;
    double height;

    // purpose unclear
    node_value_t value;

    // purpose unclear
    void *key;

    unique_ptr<QuadNode> northwest; //nw;
    unique_ptr<QuadNode> northeast; //ne;
    unique_ptr<QuadNode> southwest; //sw;
    unique_ptr<QuadNode> southeast; //se;

};
} // namespace quadtree
#endif // _QUAD_TREE_NODE_HPP_
