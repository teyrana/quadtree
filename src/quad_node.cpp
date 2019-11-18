// The MIT License 
// (c) 2019 Daniel Williams

#include <sstream>
#include <string>

#include "quad_node.hpp"

using quadtree::QuadNode;

template<class V>
QuadNode<V>::QuadNode(double _center_x, double _center_y, double _width, double _height) {
    this->center.x = _center_x;
    this->center.y = _center_y;
    this->width = _width;
    this->height = _height;
}

// QuadNode<V>::QuadNode(double minx, double miny, double maxx, double maxy){
//   if(!(node = quadtree_node_new())) return NULL;
//   if(!(node->bounds = quadtree_bounds_new())) return NULL;
//   quadtree_bounds_extend(node->bounds, maxx, maxy);
//   quadtree_bounds_extend(node->bounds, minx, miny);
// }

template<class V>
double QuadNode<V>::get_x() const {
    return center.x;
}

template<class V>
double QuadNode<V>::get_y() const {
    return center.y;
}

template<class V>
double QuadNode<V>::get_width() const {
    return width;
}

template<class V>
double QuadNode<V>::get_height() const {
    return height;
}

template<class V>
V QuadNode<V>::get_value() const {
    return this->value;
}

template<class V>
bool QuadNode<V>::is_leaf(){
    return QuadNode<V>::LEAF == this->getType();  
}

template<class V>
bool QuadNode<V>::is_empty(){
    return QuadNode<V>::EMPTY == this->getType();  
}

template<class V>
bool QuadNode<V>::is_branch(){
    return QuadNode<V>::BRANCH == this->getType();  
}


template<class V>
void QuadNode<V>::set_height(double new_height) {
    this->height = new_height;
}

template<class V>
void QuadNode<V>::set_value(V new_value){
    this->value = new_value;
}

template<class V>
void QuadNode<V>::set_width(double new_width) {
    this->width = new_width;
}

template<class V>
void QuadNode<V>::set_x(double x){
    this->center.x = x;
}

template<class V>
void QuadNode<V>::set_y(double y){
    this->center.y = y;
}



template<class V>
string QuadNode<V>::to_json(){
    std::ostringstream buf;
    buf << "{";
    // if(this->northeast != nullptr) this->northeast.release();
    // if(this->northwest != nullptr) this->northwest.release();
    // if(this->southeast != nullptr) this->southeast.release();
    // if(this->southwest != nullptr) this->southwest.release();
    // << this->value 
    buf << "}";
    return buf.str();
}

template<class V>
QuadNode<V>::~QuadNode() {
  if(this->northeast != nullptr) this->northeast.release();
  if(this->northwest != nullptr) this->northwest.release();
  if(this->southeast != nullptr) this->southeast.release();
  if(this->southwest != nullptr) this->southwest.release();
}
