// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <string>
using std::string;

#include <memory>
using std::unique_ptr;
using std::make_unique;

#include <iostream>
using std::cerr;
using std::endl;

#include <nlohmann/json/json.hpp>

#include "quadtree/tree.hpp"

using geometry::Bounds;
using geometry::Point;

using quadtree::QuadTree;
using quadtree::Node;

// used for reading and write json documents:
const static string bounds_key("bounds");
const static string tree_key("tree");
const static string grid_key("grid");

QuadTree::QuadTree(): QuadTree({0,0}, 1024) {}

QuadTree::QuadTree(Point _center, double _width):
    root(new Node({_center, _width}, 0))
{
    root->split();
}

QuadTree::~QuadTree(){
    this->reset();
}

bool QuadTree::contains(const double x, const double y) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x;
    const double cy = bounds.center.y;
    const double dim = bounds.half_width;

    if((x < (cx - dim)) || ((cx + dim) < x)){
        return false;
    }else if((y < (cy - dim)) || ((cy + dim) < y)){
        return false;
    }

    return true;
}

void QuadTree::draw(std::ostream& sink) const{
    auto& bounds = get_bounds(); 
    sink << "====== Tree: ======\n";
    sink << "@(" << bounds.center.x << ", " << bounds.center.y << ")   |" << bounds.half_width << "|\n";

    root->draw(sink, "", "RT");
    
    sink << endl;
}

const Bounds& QuadTree::get_bounds() const {
    return root->get_bounds();
}

node_value_t QuadTree::get_default_value(const Point& at) const {
    return 99;
}

void QuadTree::grow(const double precision){
    return root->split(precision);
}


node_value_t QuadTree::interp(const Point& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if point is outside the tree, entirely
    if( ! root->contains(at)){
        return get_default_value(at);
    }

    // if the point is near-to-center if the nearest node:
    if( near.nearby(at)){
        return near.get_value();
    }

    const Point& cn = near.get_center();
    const double dx = std::copysign(1.0, (at.x - cn.x)) * 2 * near.get_bounds().half_width;
    const double dy = std::copysign(1.0, (at.y - cn.y)) * 2 * near.get_bounds().half_width;
    const Node& n2 = root->search({cn.x + dx, cn.y     });
    const Node& n3 = root->search({cn.x + dx, cn.y + dy});
    const Node& n4 = root->search({cn.x     , cn.y + dy});

    const auto& interp = near.interpolate_bilinear(at, n2, n3, n4);

    return interp;
}

size_t QuadTree::height() const {
    return root->get_height();
}

void QuadTree::load(std::istream& source){
    nlohmann::json doc = nlohmann::json::parse( source,    // input
                                                nullptr,  // callback argument
                                                false);   // do not allow exceptions

    if(doc.is_discarded()){
        // in case of ANY error, discard this input
        return;
    }

    if(!(doc.is_object() && doc.contains("bounds"))){
        // 'bounds' is necessary for any well-formed tree serialization

        return;
    }else{
        auto& bounds = doc[bounds_key];
        if(!(bounds.contains("x") && bounds.contains("y") && bounds.contains("width"))){
            return;
        }

        // load bounds
        const Point center = {bounds["x"].get<double>(), bounds["y"].get<double>()};
        const double width = bounds["width"].get<double>();
        set({center,width});

        doc.erase(bounds_key);

        // load rest of document
        if(doc.contains(tree_key)){
            // detected treelike data
            root->load(doc[tree_key]);

        }else if(doc.contains(grid_key)){
            // detected gridlike data
            load_grid(doc[grid_key]);
        }
    }
}

void QuadTree::load(const std::vector<Point>& source){
    // pass
}

void QuadTree::load_grid(nlohmann::json& grid ){
    if(grid.is_array() && grid[0].is_array()){

        // pre-allocate the right tree shape & depth for the input grid
        const int dim = grid[0].size();
        const auto& bounds = root->get_bounds();
        const int precision = bounds.width() / dim;
        grow(precision);

        // poplutate the tree
        int yi = dim-1;
        for(auto& row : grid){
            double y = bounds.get_y_min() + (yi + 0.5)* precision;

            int xi = 0;
            for(auto& element : row){
                double x = bounds.get_x_min() + (xi + 0.5) * precision;
                Node& n= root->search({x,y});
                n.set_value(element.get<int>());
                ++xi;
            }
            --yi;
        }
        
        root->condense();
    }
}

node_value_t QuadTree::search(const double x, const double y) {
    const Point at(x,y);
    if(root->contains(at)){
        return root->search(at).get_value();
    }else{
        return get_default_value(at);
    }
}

void QuadTree::reset() {
    this->root.reset();
}

void QuadTree::set(const Bounds new_bounds) {
    if( new_bounds != root->get_bounds() ){
        root = std::make_unique<Node>(new_bounds, NAN);
    }
}

// void QuadTree::set(double x, double y, node_value_t new_value) {
//     if( ! contains(x,y)){
//         cerr << "Attempt to set location for a point (" << x << ", " << y << "), not contained by this tree\n";
//     }

//     // reminder: this method does not modify tree _shape_.  Just stores values in existing nodes.

// }

void QuadTree::write_json(std::ostream& sink) const {
    nlohmann::json doc;

    // output the bounds once, at the root-level, and all the bounds
    // for subsequent layers follow deterministicly
    const Bounds& bounds = root->get_bounds();
    doc[bounds_key] = {
        {"x", bounds.center.x},
        {"y", bounds.center.y},
        // tree is implemented & enforced as square
        {"width", 2 * bounds.half_width}
    };
    
    doc[tree_key] = root->to_json();

    // write result to output stream:
    sink << doc;
}

std::ostream& operator<<(std::ostream& os, const quadtree::QuadTree& tree)
{
    tree.write_json(os);
    return os;
}
