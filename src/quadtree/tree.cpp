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
    Tree(layout->default_bounds, layout->default_precision)
{}

Tree::Tree(const Bounds& _bounds, const double _precision):
    layout(new Layout(_bounds, _precision)), root(std::make_unique<Node>(_bounds, 0))
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

size_t Tree::calculate_full_loading(const size_t height){
    size_t full = 1;
    for(size_t n = 1; n < height; ++n){
        full += std::pow(4,n);
    }
    return full;
}

double Tree::get_load_factor() const {
    const size_t height = root->get_height();
    const size_t count = root->get_count();
    const size_t full = calculate_full_loading(height);
    return static_cast<double>(count) / static_cast<double>(full);
}

double Tree::get_precision() const { 
    return layout->precision;
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

size_t Tree::size() const {
    return root->get_count();
}

json Tree::to_json_tree() const {
    return root->to_json();
}

size_t Tree::get_width() const {
    return layout->bounds.width();
}
