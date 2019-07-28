// The MIT License 
// (c) 2019 Daniel Williams

#include <sstream>
#include <string>

#include "quad_node.hpp"
using namespace quadtree;

QuadNode::QuadNode(double _center_x, double _center_y, double _width, double _height) {
    this->center.x = _center_x;
    this->center.y = _center_y;
    this->width = _width;
    this->height = _height;
}

// QuadNode::QuadNode(double minx, double miny, double maxx, double maxy){
//   if(!(node = quadtree_node_new())) return NULL;
//   if(!(node->bounds = quadtree_bounds_new())) return NULL;
//   quadtree_bounds_extend(node->bounds, maxx, maxy);
//   quadtree_bounds_extend(node->bounds, minx, miny);
// }

double QuadNode::get_x() const {
    return center.x;
}

double QuadNode::get_y() const {
    return center.y;
}

double QuadNode::get_width() const {
    return width;
}

double QuadNode::get_height() const {
    return height;
}

node_value_t QuadNode::get_value() const {
    return this->value;
}

QuadNode::NodeType QuadNode::get_type() const {
    return this->nodetype;
}

bool QuadNode::is_leaf(){
    return QuadNode::LEAF == this->get_type();
}

bool QuadNode::is_empty(){
    return QuadNode::EMPTY == this->get_type();  
}

bool QuadNode::is_branch(){
    return QuadNode::BRANCH == this->get_type();  
}

void QuadNode::set_height(double new_height) {
    this->height = new_height;
}

void QuadNode::set_value(node_value_t new_value){
    this->value = new_value;
}

void QuadNode::set_width(double new_width) {
    this->width = new_width;
}

void QuadNode::set_x(double x){
    this->center.x = x;
}

void QuadNode::set_y(double y){
    this->center.y = y;
}

string QuadNode::to_json(){
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

QuadNode::~QuadNode() {
  if(this->northeast != nullptr) this->northeast.release();
  if(this->northwest != nullptr) this->northwest.release();
  if(this->southeast != nullptr) this->southeast.release();
  if(this->southwest != nullptr) this->southwest.release();
}
