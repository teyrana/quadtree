// The MIT License 
// (c) 2019 Daniel Williams

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "quadtree/node.hpp"

using std::addressof;
using std::cout;
using std::endl;
using std::ostream;

using namespace geometry;
using namespace quadtree;

Node::Node():
    Node({NAN, NAN}, NAN, 0)
{}

Node::Node(const Point& _center, const double _width, const node_value_t _value):
    bounds(_center, _width), northeast(nullptr), northwest(nullptr), southwest(nullptr), southeast(nullptr), value(_value)
{}

// QuadTreeNode::QuadTreeNode(double minx, double miny, double maxx, double maxy){
//   if(!(node = quadtree_node_new())) return NULL;
//   if(!(node->bounds = quadtree_bounds_new())) return NULL;
//   quadtree_bounds_extend(node->bounds, maxx, maxy);
//   quadtree_bounds_extend(node->bounds, minx, miny);
// }

bool Node::contains(const Point& at) const {
    return bounds.contains(at);
}

const Bounds& Node::get_bounds() const {
    return bounds;
}

const Point& Node::get_center() const {
    return bounds.center;
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
    return northwest.get();
}

Node* Node::get_southeast(){
    return southeast.get();
}

Node* Node::get_southwest(){
    return southwest.get();
}
    
bool Node::is_leaf() const{
    return ! northeast;
}

node_value_t Node::interpolate_linear(const Point& at, const Node& node2) const {
    if(this == &node2){
        return this->get_value();
    }
    const auto& node1 = *this;

    // distances from query point to each interpolation point
    const double dist1 = at.distance(node1.bounds.center);
    const double dist2 = at.distance(node2.bounds.center);

    // this is not perfect, but it's a reasonable heuristic
    // ... in particular, it will return odd values at large distances
    // ... arguably, this should return a NAN value instead -- for not-applicable
    const double dist12 = this->bounds.center.distance(node2.bounds.center);
    if(dist12 < dist1){
        return node2.value;
    }else if( dist12 < dist2){
        return node1.value;
    }

    const double combined_distance = dist1 + dist2;
    const double normdist1 = 1 - dist1 / combined_distance;
    const double normdist2 = 1 - dist2 / combined_distance;
    const double interp_value = (normdist1*node1.value + normdist2*node2.value);

    return round(interp_value);
}


node_value_t Node::interpolate_bilinear(const Point& at, 
                                        const Node& xn,
                                        const Node& dn,
                                        const Node& yn) const 
{
    // cout << "    ==>>             @" << at << endl;
    // cout << "    ==>> this:       " << *this << "    = " << get_value() << endl;
    // cout << "    ==>> xn:         " << xn << "    = " << xn.get_value() << endl;
    // cout << "    ==>> dn:         " << dn << "    = " << dn.get_value() << endl;
    // cout << "    ==>> yn:         " << yn << "    = " << yn.get_value() << endl;

    // test for degenerate cases:
    if( &xn == &dn ){
        // top or bottom border
        return interpolate_linear({at.x, xn.y()}, xn);
    }else if( &yn  == &dn ){
        // left or right border
        return interpolate_linear({yn.x(), at.y}, yn);
    }

    // calculate full bilinear interpolation:
    const Point upper_point(at.x, xn.y());
    const Node upper_node(upper_point, 
                          2*bounds.half_width,
                          this->interpolate_linear(upper_point, xn));

    const Point lower_point(at.x, yn.y());
    const Node lower_node(lower_point, 
                          2*bounds.half_width,
                          yn.interpolate_linear(lower_point, dn));

    // cout << "         >>(U): " << upper_node << "    = " << upper_node.get_value() << endl;
    // cout << "         >>(L): " << lower_node << "    = " << lower_node.get_value() << endl;

    return upper_node.interpolate_linear(at, lower_node);
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

bool Node::nearby(const Point& p) const {
    return nearby(p, snap_center_distance);
}

bool Node::nearby(const Point& p, const double threshold) const {
    return (snap_center_distance > bounds.center.distance(p));
}

ostream& quadtree::operator<<(ostream& sink, const Node& n){
    sink << n.bounds;
    return sink;
}

const Node& Node::search(const Point& p) {
    if(is_leaf()){
        return *this;
    }

    if(p.x > bounds.center.x){
        if( p.y > bounds.center.y){
            return *this->northeast.get();
        }else{
            return *this->southeast.get();
        }
    }else{
        if( p.y > bounds.center.y){
            return *this->northwest.get();
        }else{
            return *this->southwest.get();
        }
    }
}

void Node::set_value(node_value_t new_value){
    this->value = new_value;
}

void Node::reset(){
    if(northeast != nullptr) this->northeast.release();
    if(northwest != nullptr) this->northwest.release();
    if(southeast != nullptr) this->southeast.release();
    if(southwest != nullptr) this->southwest.release();
}

void Node::split(){
    const Point& ctr = bounds.center;
    const double& qw = bounds.half_width/2;

    this->northeast.reset(new Node({ctr.x + qw, ctr.y + qw}, bounds.half_width, value));
    this->northwest.reset(new Node({ctr.x - qw, ctr.y + qw}, bounds.half_width, value));
    this->southeast.reset(new Node({ctr.x + qw, ctr.y - qw}, bounds.half_width, value));
    this->southwest.reset(new Node({ctr.x - qw, ctr.y - qw}, bounds.half_width, value));
}

nlohmann::json Node::to_json() const {
    nlohmann::json doc;

    if(is_leaf()){
	doc = json({1.0}, false, json::value_t::number_integer)[0];
        //this->value;
    }else{
        assert(northeast);
        assert(northwest);
        assert(southeast);
        assert(southwest);
        doc["NE"] = northeast->to_json();
        doc["NW"] = northwest->to_json();
        doc["SE"] = southeast->to_json();
        doc["SW"] = southwest->to_json();
    }
    return doc;
}

std::string Node::to_string() const {
    std::ostringstream buf;
    buf << this->to_json();
    return buf.str();
}

double Node::x() const {
    return bounds.center.x;
}

double Node::y() const {
    return bounds.center.y;
}

Node::~Node() {
    reset();
}
