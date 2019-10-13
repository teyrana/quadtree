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

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "quadtree/tree.hpp"

using namespace terrain;
using geometry::Bounds;
using quadtree::Tree;
using quadtree::Node;


static cell_value_t scratch;

Tree::Tree(): 
    Tree(Layout::default_layout.bounds, layout->default_precision)
{}

Tree::Tree(const Bounds& _bounds, const double _precision):
    layout(new Layout(_bounds, _precision)), root(std::make_unique<Node>(_bounds, 0))
{}

Tree::~Tree(){
    root.release();
}

bool Tree::contains(const Eigen::Vector2d& p) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center[0];
    const double cy = bounds.center[1];
    const double dim = bounds.half_width;

    if((p[0] < (cx - dim)) || ((cx + dim) < p[0])){
        return false;
    }else if((p[1] < (cy - dim)) || ((cy + dim) < p[1])){
        return false;
    }

    return true;
}

void Tree::debug_tree() const {
    cerr << "====== Quad Tree: ======\n";
    cerr << "##  bounds:     " << get_bounds().str() << endl;
    cerr << "##  height:     " << get_height() << endl;
    cerr << "##  precision:  " << get_precision() << endl;

    root->draw(cerr, "    ", "RT");
    cerr << endl;
}

const Bounds& Tree::get_bounds() const {
    return layout->bounds;
}

size_t Tree::get_dimension() const {
    return layout->dimension;
}

size_t Tree::calculate_complete_tree(const size_t height){

    // see: https://en.wikipedia.org/wiki/M-ary_tree
    //      # properties of M-ary trees
    //
    //    m == branching_factor == 4  ///< for a quadtree, this is trivially 4
    //
    //       (h+1)
    //     m        - 1
    // N = ---------------
    //        m - 1
    // 
    return ( pow(4,height+1) - 1 )/3;
}

double Tree::get_load_factor() const {
    const size_t height = root->get_height();
    const size_t count = root->get_count();
    const size_t complete = calculate_complete_tree(height);
    return static_cast<double>(count) / static_cast<double>(complete);
}

size_t Tree::get_memory_usage() const {
    return size() * sizeof(Node);
}

double Tree::get_precision() const { 
    return layout->precision;
}

cell_value_t Tree::interp(const Eigen::Vector2d& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if Eigen::Vector2d is outside the tree, entirely
    if( ! contains(at)){
        return cell_default_value;
    }

    // if the Eigen::Vector2d is near-to-center if the nearest node:
    if( near.nearby(at)){
        return near.get_value();
    }

    const Eigen::Vector2d& cn = near.get_center();
    const double dx = std::copysign(1.0, (at[0] - cn[0])) * 2 * near.get_bounds().half_width;
    const double dy = std::copysign(1.0, (at[1] - cn[1])) * 2 * near.get_bounds().half_width;
    const Node& n2 = root->search({cn[0] + dx, cn[1]     });
    const Node& n3 = root->search({cn[0] + dx, cn[1] + dy});
    const Node& n4 = root->search({cn[0]     , cn[1] + dy});

    const auto& interp = near.interpolate_bilinear(at, n2, n3, n4);

    return interp;
}

void Tree::fill(const cell_value_t fill_value){
    root->fill(fill_value);
}

size_t Tree::get_height() const {
    return root->get_height() - 1;
}

const Layout& Tree::get_layout() const {
    return *layout.get();
}

bool Tree::load_tree(const nlohmann::json& doc){
    if(! doc.is_object()){
        cerr << "?? attempted to load unexpected format: no-object json document!\n";
        return false;
    }
    return root->load(doc);
}

void Tree::prune(){
    root->prune();
}

void Tree::reset(const Bounds new_bounds, const double new_precision){
    if( isnan(new_precision) ){
        root.release();
        return;
    }

    layout.reset(new geometry::Layout(new_bounds, new_precision));

    root = std::make_unique<Node>(layout->bounds, 0);
    root->split(layout->precision);
}

cell_value_t& Tree::search(const Eigen::Vector2d& p) {
    if(contains(p)){
        return root->search(p).get_value();
    }

    scratch = cell_error_value;
    return scratch;
}

cell_value_t Tree::search(const Eigen::Vector2d& p) const {
    if(contains(p)){
        return root->search(p).get_value();
    }
    return cell_default_value;
}

size_t Tree::size() const {
    return root->get_count();
}

json Tree::to_json_tree() const {
    return root->to_json();
}

size_t Tree::get_width() const {
    return layout->bounds.width();
}
