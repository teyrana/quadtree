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

using terrain::geometry::cell_value_t;

namespace terrain::quadtree {


class Node {
    enum Quadrant {NW, NE, SW, SE};
public:
    Node();
    Node(const cell_value_t value);

    ~Node();

    void draw(std::ostream& sink, const std::string& prefix, const std::string& as, const bool show_pointers) const;

    void fill(const cell_value_t fill_value);

    size_t get_count() const;
    size_t get_height() const;

    Node* get(Node::Quadrant quad) const;
    Node* get_northeast() const;
    Node* get_northwest() const;
    Node* get_southeast() const;
    Node* get_southwest() const;

    cell_value_t& get_value();
    cell_value_t get_value() const;

    bool load(const nlohmann::json& doc);

    bool operator==(const Node& other) const;

    ///! \brief coalesce groups of leaf nodes with identice values (for some value of "identical")
    void prune();

    void split(const double precision, const double width);

    void reset();

    bool is_leaf() const;

    void set_value(cell_value_t new_value);

    nlohmann::json to_json() const;

    std::string to_string() const;

private:
    void split();

private:
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
    friend class NodeTest_ConstructDefault_Test;
    friend class NodeTest_ConstructWithValue_Test;
    friend class NodeTest_SetGet_Test;
    friend class NodeTest_SplitNodeImperative_Test;

};

} // namespace terrain::quadtree

#endif // #ifndef _QUADTREE_NODE_HPP_
