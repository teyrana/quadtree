// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _QUAD_TREE_NODE_HPP_
#define _QUAD_TREE_NODE_HPP_

#include <string>
using std::string;

#include <memory>
using std::unique_ptr;

#include "point.hpp"

namespace quadtree {

template <class V>
class QuadNode {
public:
    enum NodeType { EMPTY, LEAF, BRANCH };

public:

    QuadNode(double x, double y, double w, double h);
    ~QuadNode();

    NodeType get_type();

    double get_height() const;
    double get_width() const;
    V      get_value() const;
    double get_x() const;
    double get_y() const;
    
    
    bool is_leaf();
    bool is_empty();
    bool is_branch();

    void set_height(double h);
    void set_value(V new_value);
    void set_width(double w);
    void set_x(double x);
    void set_y(double x);

    string to_json();

//private:
public:
    NodeType nodetype = NodeType::EMPTY;
    
    //QuadNode<V>* parent;

    // quadtree_bounds_t *bounds;

    // center point
    Point center;

    double width;
    double height;

    // purpose unclear
    V value;

    // purpose unclear
    void *key;

    unique_ptr<QuadNode> northwest; //nw;
    unique_ptr<QuadNode> northeast; //ne;
    unique_ptr<QuadNode> southwest; //sw;
    unique_ptr<QuadNode> southeast; //se;

};

// experiment:
typedef QuadNode<double> XYQuodnode;

} // namespace quadtree
#endif // _QUAD_TREE_NODE_HPP_
