// The MIT License 
// (c) 2019 Daniel Williams

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "quadtree/node.hpp"

using std::cout;
using std::endl;
using namespace quadtree;

Node::Node():
    northeast(nullptr), quadrants({nullptr, nullptr, nullptr})
{}

Node::Node(const Point& _center, const double _width):
    Node(_center, _width, _width)
{}

Node::Node(const Point& _center, const double _height, const double _width):
    bounds(_center, _height, _width), northeast(nullptr), quadrants({nullptr, nullptr, nullptr})
{}

// QuadTreeNode::QuadTreeNode(double minx, double miny, double maxx, double maxy){
//   if(!(node = quadtree_node_new())) return NULL;
//   if(!(node->bounds = quadtree_bounds_new())) return NULL;
//   quadtree_bounds_extend(node->bounds, maxx, maxy);
//   quadtree_bounds_extend(node->bounds, minx, miny);
// }

Node* Node::search(double x, double y) {
    if(is_leaf()){
        return this;
    }

    if(x > bounds.center.x){
        if( y > bounds.center.y){
            return this->northeast.get();
        }else{
            return this->quadrants.southeast.get();
        }
    }else{
        if( y > bounds.center.y){
            return this->quadrants.northwest.get();
        }else{
            return this->quadrants.southwest.get();
        }
    }

    return this;
}

const Bounds& Node::get_bounds() const {
    return this->bounds;
}

node_value_t Node::get_value() const {
    if(is_leaf()){
	return this->value;
    }

    return 0.;
}

Node* Node::get_northeast(){
    return northeast.get();
}

Node* Node::get_northwest(){
    return quadrants.northwest.get();
}

Node* Node::get_southeast(){
    return quadrants.southeast.get();
}

Node* Node::get_southwest(){
    return quadrants.southwest.get();
}
    
bool Node::is_leaf() const{
    return ! northeast;
}

void Node::load(nlohmann::json doc){
    reset();
    if(doc.is_object()){
        this->split();
        get_northeast()->load(doc["NE"]);
        get_northwest()->load(doc["NW"]);
        get_southeast()->load(doc["SE"]);
        get_southwest()->load(doc["SW"]);
    }else{
        assert(is_leaf());
        this->set_value(doc.get<double>());
        
    }
}

void Node::set_value(node_value_t new_value){
    this->value = new_value;
}

void Node::reset(){
    if(northeast != nullptr) this->northeast.release();
    if(quadrants.northwest != nullptr) this->quadrants.northwest.release();
    if(quadrants.southeast != nullptr) this->quadrants.southeast.release();
    if(quadrants.southwest != nullptr) this->quadrants.southwest.release();
}

void Node::split(){
    const Point& ctr = bounds.center;
    const double& qh = bounds.half_height/2;
    const double& qw = bounds.half_width/2;

    this->          northeast.reset(new Node({ctr.x + qw, ctr.y + qh}, bounds.half_height, bounds.half_width));
    this->quadrants.northwest.reset(new Node({ctr.x - qw, ctr.y + qh}, bounds.half_height, bounds.half_width));
    this->quadrants.southeast.reset(new Node({ctr.x + qw, ctr.y - qh}, bounds.half_height, bounds.half_width));
    this->quadrants.southwest.reset(new Node({ctr.x - qw, ctr.y - qh}, bounds.half_height, bounds.half_width));
}

nlohmann::json Node::to_json() const {
    nlohmann::json doc;

    if(is_leaf()){
	doc = json({1.0}, false, json::value_t::number_integer)[0];
        //this->value;
    }else{
        assert(northeast);
        assert(quadrants.northwest);
        assert(quadrants.southeast);
        assert(quadrants.southwest);
        doc["NE"] = northeast->to_json();
        doc["NW"] = quadrants.northwest->to_json();
        doc["SE"] = quadrants.southeast->to_json();
        doc["SW"] = quadrants.southwest->to_json();
    }
    return doc;
}

std::string Node::to_string() const {
    std::ostringstream buf;
    buf << this->to_json();
    return buf.str();
}

Node::~Node() {
    reset();
}
