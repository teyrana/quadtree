// The MIT License 
// (c) 2019 Daniel Williams

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "geometry/cell_value.hpp"
#include "quadtree/node.hpp"

using std::addressof;
using std::cerr;
using std::endl;
using std::make_unique;
using std::ostream;
using std::string;
using std::unique_ptr;

using terrain::geometry::Bounds;
using terrain::geometry::cell_value_t;
using terrain::quadtree::Node;

// Node::Node():
//     Node({{NAN, NAN}, NAN}, 0)
// {}

// Node::Node(const Eigen::Vector2d& _center, const double _width, const cell_value_t _value):
//     bounds(_center, _width), northeast(nullptr), northwest(nullptr), southwest(nullptr), southeast(nullptr), value(_value)
// {}

Node::Node(const Bounds& _bounds, const cell_value_t _value):
    bounds(_bounds), northeast(nullptr), northwest(nullptr), southwest(nullptr), southeast(nullptr), value(_value)
{}

// QuadTreeNode::QuadTreeNode(double minx, double miny, double maxx, double maxy){
//   if(!(node = quadtree_node_new())) return NULL;
//   if(!(node->bounds = quadtree_bounds_new())) return NULL;
//   quadtree_bounds_extend(node->bounds, maxx, maxy);
//   quadtree_bounds_extend(node->bounds, minx, miny);
// }

bool Node::contains(const Eigen::Vector2d& at) const {
    return bounds.contains(at);
}

void Node::draw(std::ostream& sink, const string& prefix, const string& as) const {
    sink << prefix << "[" << as << "]: ";
    if(is_leaf()){
        sink << static_cast<int>(get_value());
    }
    //sink << ' ' << bounds.str();
    sink << endl;
    
    if(!is_leaf()){
        auto next_prefix = prefix + "    ";
        northeast->draw(sink, next_prefix, "NE");
        northwest->draw(sink, next_prefix, "NW");
        southeast->draw(sink, next_prefix, "SE");
        southwest->draw(sink, next_prefix, "SW");
    }
}

void Node::fill(const cell_value_t fill_value){
    if(is_leaf()){
        set_value(fill_value);
    }else{
        northeast->fill(fill_value);
        northwest->fill(fill_value);
        southeast->fill(fill_value);
        southwest->fill(fill_value);
    }
}

const Bounds& Node::get_bounds() const {
    return bounds;
}

const Eigen::Vector2d& Node::get_center() const {
    return bounds.center;
}

size_t Node::get_count() const {
    if( is_leaf()){
        return 1;
    }else{
        size_t count = 0;
        count += northeast->get_count();
        count += northwest->get_count();
        count += southeast->get_count();
        count += southwest->get_count();
        return count + 1;
    }
}

size_t Node::get_height() const {
    if(is_leaf()){
        return 1;
    }else{
        const size_t ne_height = northeast->get_height();
        const size_t nw_height = northwest->get_height();
        const size_t se_height = southeast->get_height();
        const size_t sw_height = southwest->get_height();
        
        const size_t max_height = std::max(ne_height, std::max(nw_height, std::max(se_height, sw_height)));
        return max_height + 1;
    }
}

Node* Node::get_northeast() const {
    return northeast.get();
}

Node* Node::get_northwest() const {
    return northwest.get();
}

Node* Node::get_southeast() const {
    return southeast.get();
}

Node* Node::get_southwest() const {
    return southwest.get();
}
    
bool Node::is_leaf() const{
    return ! northeast;
}

cell_value_t& Node::get_value() {
    return this->value;
}

cell_value_t Node::get_value() const {
    return this->value;
}

cell_value_t Node::interpolate_linear(const Eigen::Vector2d& at, const Node& node2) const {
    if(this == &node2){
        return this->get_value();
    }
    const auto& node1 = *this;

    // distances from query point to each interpolation point
    const double dist1 = (node1.bounds.center - at).norm();
    const double dist2 = (node2.bounds.center - at).norm();

    // this is not perfect, but it's a reasonable heuristic
    // ... in particular, it will return odd values at large distances
    // ... arguably, this should return a NAN value instead -- for not-applicable
    const double dist12 = (bounds.center - node2.bounds.center).norm();
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


cell_value_t Node::interpolate_bilinear(const Eigen::Vector2d& at, 
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
        return interpolate_linear({at[0], xn[1]}, xn);
    }else if( &yn  == &dn ){
        // left or right border
        return interpolate_linear({yn[0], at[1]}, yn);
    }

    // calculate full bilinear interpolation:
    const Eigen::Vector2d upper_point(at[0], xn[1]);
    const Node upper_node({upper_point, 
                               2*bounds.half_width},
                               this->interpolate_linear(upper_point, xn));

    const Eigen::Vector2d lower_point(at[0], yn[1]);
    const Node lower_node({lower_point, 
                               2*bounds.half_width},
                               yn.interpolate_linear(lower_point, dn));

    // cout << "         >>(U): " << upper_node << "    = " << upper_node.get_value() << endl;
    // cout << "         >>(L): " << lower_node << "    = " << lower_node.get_value() << endl;

    return upper_node.interpolate_linear(at, lower_node);
}

bool Node::load(const nlohmann::json& doc){
    reset();
    if(doc.is_object()){
        this->split();
        get_northeast()->load(doc["NE"]);
        get_northwest()->load(doc["NW"]);
        get_southeast()->load(doc["SE"]);
        get_southwest()->load(doc["SW"]);
        return true;
    }else{
        assert(is_leaf());
        this->set_value(doc.get<double>());
        return true;
    }
}

bool Node::nearby(const Eigen::Vector2d& p) const {
    return nearby(p, snap_center_distance);
}

bool Node::nearby(const Eigen::Vector2d& p, const double threshold) const {
    return (snap_center_distance > (p - bounds.center).norm());
}

double Node::operator[](const size_t index){
    return bounds.center[index];
}

const double Node::operator[](const size_t index) const {
    return bounds.center[index];
}

void Node::prune() {
    if( is_leaf() ){
        return;
    }

    northeast->prune();
    northwest->prune();
    southeast->prune();
    southwest->prune();

    if( get_northeast()->is_leaf() && 
        get_northwest()->is_leaf() && 
        get_southeast()->is_leaf() &&
        get_southwest()->is_leaf())
    {
        auto nev = get_northeast()->get_value();
        auto nwv = get_northwest()->get_value();
        auto sev = get_southeast()->get_value();
        auto swv = get_southwest()->get_value();

        if( (nev == nwv) && (nwv == sev) && (sev == swv )){
            reset();
            set_value(nev);
        }
    }
}

Node& Node::search(const Eigen::Vector2d& p) {
    if(is_leaf()){
        return *this;
    }

    if(p[0] > bounds.center[0]){
        if( p[1] > bounds.center[1]){
            return this->northeast->search(p);
        }else{
            return this->southeast->search(p);
        }
    }else{
        if( p[1] > bounds.center[1]){
            return this->northwest->search(p);
        }else{
            return this->southwest->search(p);
        }
    }
}

void Node::set_value(cell_value_t new_value){
    this->value = new_value;
}

void Node::reset(){
    if(northeast != nullptr) this->northeast.release();
    if(northwest != nullptr) this->northwest.release();
    if(southeast != nullptr) this->southeast.release();
    if(southwest != nullptr) this->southwest.release();
}

void Node::split(){
    const Eigen::Vector2d& ctr = bounds.center;
    const double& qw = bounds.half_width/2; ///< quarter-width

    value = NAN;

    this->northeast = make_unique<Node>(Bounds({ctr[0] + qw, ctr[1] + qw}, bounds.half_width), value);
    this->northwest = make_unique<Node>(Bounds({ctr[0] - qw, ctr[1] + qw}, bounds.half_width), value);
    this->southeast = make_unique<Node>(Bounds({ctr[0] + qw, ctr[1] - qw}, bounds.half_width), value);
    this->southwest = make_unique<Node>(Bounds({ctr[0] - qw, ctr[1] - qw}, bounds.half_width), value);
}

void Node::split(const double precision){
    if(2 * bounds.half_width <= precision){
        return;
    }
    
    if(is_leaf()){
        split();
    }

    this->northeast->split(precision);
    this->northwest->split(precision);
    this->southeast->split(precision);
    this->southwest->split(precision);
}

nlohmann::json Node::to_json() const {
    nlohmann::json doc;

    if(is_leaf()){
        doc = json({this->value}, false, json::value_t::number_integer)[0];
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

Node::~Node() {
    reset();
}
