// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <cstdio>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>

using std::string;
using std::cerr;
using std::endl;

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "quadtree/tree.hpp"

using namespace terrain;
using geometry::Bounds;
using geometry::Point;
using quadtree::Tree;
using quadtree::Node;


static cell_value_t scratch;

Tree::Tree(): 
    Tree(Tree::default_bounds, 4.)
{}

Tree::Tree(const Bounds& _bounds, const double _precision):
    bounds(_bounds), root(new Node(_bounds, 0)), precision(_precision)
{}

Tree::~Tree(){
    root.release();
}

bool Tree::contains(const Point& p) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x;
    const double cy = bounds.center.y;
    const double dim = bounds.half_width;

    if((p.x < (cx - dim)) || ((cx + dim) < p.x)){
        return false;
    }else if((p.y < (cy - dim)) || ((cy + dim) < p.y)){
        return false;
    }

    return true;
}

void Tree::cull(){
    root->cull();
}

void Tree::debug_tree() const {
    cerr << "====== Quad Tree: ======\n";
    cerr << "##  bounds:     " << get_bounds().str() << endl;
    cerr << "##  height:     " << get_height() << endl;
    cerr << "##  precision:  " << get_precision() << endl;

    root->draw(cerr, "    ", "RT");
    cerr << endl;
}

size_t Tree::dimension() const {
    return static_cast<size_t>(bounds.width() / precision);
}

const Bounds& Tree::get_bounds() const {
    return bounds;
}

size_t Tree::get_dimension() const {
    return static_cast<size_t>( bounds.width() / precision );
}

void Tree::grow(const double _precision){
    precision = _precision;
    return root->split(_precision);
}

cell_value_t Tree::interp(const Point& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if point is outside the tree, entirely
    if( ! contains(at)){
        return cell_default_value;
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

void Tree::fill(const cell_value_t fill_value){
    root->fill(fill_value);
}

size_t Tree::get_height() const {
    return root->get_height();
}

void Tree::load_polygon(const std::vector<Point>& source){
    // pass
}

bool Tree::load_grid(nlohmann::json& grid ){
    if(grid.is_array() && grid[0].is_array()){
        const size_t dim = grid.size();
        const size_t pow_of_2 = pow(2, ceil(log2(dim)));
        if(grid.size() != pow_of_2 ){
            return false;
        }
        
        // pre-allocate the right tree shape & depth for the input grid
        const double new_precision = bounds.width() / dim;
        grow(new_precision);

        // populate the tree
        int row_index = dim-1;
        for(auto& row : grid){
            double y = bounds.get_y_min() + (row_index + 0.5)* precision;

            int column_index = 0;
            // i.e. a cell is the element at [column_index, row_index] <=> [x,y]
            for(auto& cell : row){
                double x = bounds.get_x_min() + (column_index + 0.5) * precision;
                Node& n= root->search({x,y});
                n.set_value(cell.get<int>());
                ++column_index;
            }
            --row_index;
        }
        
        root->cull();
        return true;
    }
    return false;
}

bool Tree::load_tree(nlohmann::json& doc){
    if(! doc.is_object()){
        return false;
    }
    return root->load(doc);
}


cell_value_t& Tree::search(const Point& p) {
    if(contains(p)){
        return root->search(p).get_value();
    }

    scratch = cell_error_value;
    return scratch;
}

cell_value_t Tree::search(const Point& p) const {
    if(contains(p)){
        return root->search(p).get_value();
    }
    return cell_default_value;
}

void Tree::reset(const Bounds new_bounds, const double new_precision){
    if( isnan(new_precision) ){
        root.release();
        return;
    }

    this->bounds = new_bounds;
    this->precision = new_precision;


    root = std::make_unique<Node>(bounds,0);
    root->split(precision);
}

// void Tree::set(double x, double y, cell_value_t new_value) {
//     if( ! contains(x,y)){
//         cerr << "Attempt to set location for a point (" << x << ", " << y << "), not contained by this tree\n";
//     }

//     // reminder: this method does not modify tree _shape_.  Just stores values in existing nodes.

// }


json Tree::to_json_tree() const {
    return root->to_json();
}


// size_t Tree::x_to_index(double x) const {
//     if(x < bounds.get_x_min()){
//         return 0;
//     }else if(x > bounds.get_x_max()){
//         return dimension()-1;
//     }

//     return static_cast<size_t>((x - (bounds.center.x - bounds.half_width))/precision);
// }

// size_t Tree::y_to_index(double y) const {
//     if(y < bounds.get_y_min()){
//         return 0;
//     }else if(y > bounds.get_y_max()){
//         return dimension()-1;
//     }

//     return static_cast<size_t>((y - (bounds.center.y - bounds.half_width))/precision);
// }
