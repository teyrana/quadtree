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

Tree::Tree(): 
    Tree(Layout::default_layout.bounds, layout->default_precision)
{}

Tree::Tree(const Bounds& _bounds, const double _precision):
    layout(new Layout(_bounds, _precision)), root(std::make_unique<Node>(0))
{}

Tree::~Tree(){
    root.release();
}

bool Tree::contains(const Eigen::Vector2d& p) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x();
    const double cy = bounds.center.y();
    const double dim = bounds.half_width;

    if((p.x() < (cx - dim)) || ((cx + dim) < p.x())){
        return false;
    }else if((p.y() < (cy - dim)) || ((cy + dim) < p.y())){
        return false;
    }

    return true;
}

cell_value_t Tree::classify(const Eigen::Vector2d& p) const {
    if(contains(p)){
        double cur_x = layout->bounds.center.x();
        double cur_y = layout->bounds.center.y();
        double current_width = layout->bounds.width();
        Node* current_node = root.get();
        double next_width = layout->bounds.half_width;

        while( ! current_node->is_leaf() )
        {
            current_width = next_width;
            next_width *= 0.5;

            if(p.x() > cur_x){
                if( p.y() > cur_y){
                    cur_x += next_width;
                    cur_y += next_width;
                    current_node = current_node->get_northeast();
                }else{
                    cur_x += next_width;
                    cur_y -= next_width;
                    current_node = current_node->get_southeast();
                }
            }else{
                if( p.y() > cur_y){
                    cur_x -= next_width;
                    cur_y += next_width;
                    current_node = current_node->get_northwest();
                }else{
                    cur_x -= next_width;
                    cur_y -= next_width;
                    current_node = current_node->get_southwest();
                }
            }
        }
        return current_node->get_value();
    }

    return cell_error_value;
}

void Tree::debug_tree(const bool show_pointers) const {
    cerr << "====== Quad Tree: ======\n";
    cerr << "##  bounds:     " << get_bounds().str() << endl;
    cerr << "##  height:     " << get_height() << endl;
    cerr << "##  precision:  " << get_precision() << endl;

    root->draw(cerr, "    ", "RT", show_pointers);
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

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if Eigen::Vector2d is outside the tree, entirely
    if( ! contains(at)){
        return cell_default_value;
    }

    // const Node& near = root->search(at, get_bounds());
//     const Eigen::Vector2d& cn = near.get_center();
//     const double dx = std::copysign(1.0, (at.x() - cn.x())) * 2 * near.get_bounds().half_width;
//     const double dy = std::copysign(1.0, (at.y() - cn.y())) * 2 * near.get_bounds().half_width;
//     const Node& n2 = root->search({cn.x() + dx, cn.y()     }, get_bounds());
//     const Node& n3 = root->search({cn.x() + dx, cn.y() + dy}, get_bounds());
//     const Node& n4 = root->search({cn.x()     , cn.y() + dy}, get_bounds());

//     const auto& interp = near.interpolate_bilinear(at, n2, n3, n4);
    // return interp;

    return NAN;
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

    root = std::make_unique<Node>(0);
    root->split(layout->precision, layout->bounds.width());
}

Sample Tree::sample(const Eigen::Vector2d& p) const {
    if(contains(p)){
        Eigen::Vector2d current_location = layout->bounds.center;
        double current_width = layout->bounds.width();
        Node* current_node = root.get();
        double next_width = layout->bounds.half_width;

        while( ! current_node->is_leaf() )
        {
            current_width = next_width;
            next_width /= 2;

            if(p.x() > current_location.x()){
                if( p.y() > current_location.y()){
                    current_location += Eigen::Vector2d( next_width, next_width );
                    current_node = current_node->get_northeast();
                }else{
                    current_location += Eigen::Vector2d( next_width, -next_width );
                    current_node = current_node->get_southeast();
                }
            }else{
                if( p.y() > current_location.y()){
                    current_location += Eigen::Vector2d( -next_width,  next_width);
                    current_node = current_node->get_northwest();
                }else{
                    current_location += Eigen::Vector2d( -next_width, -next_width);
                    current_node = current_node->get_southwest();
                }
            }
        }
        return {current_location, current_node->get_value()};
    }

    return {{NAN, NAN}, cell_error_value};
}

bool Tree::store(const Eigen::Vector2d& p, const cell_value_t new_value) {
    if(contains(p)){
        root->search(p, get_bounds() ).get_value() = new_value;
        return true;
    }
    return false;
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
