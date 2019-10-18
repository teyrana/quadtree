// The MIT License 
// (c) 2019 Daniel Williams

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include <Eigen/Geometry>
using Eigen::Vector2d;

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

Node::Node(): Node(NAN) {}

Node::Node(const cell_value_t _value):
    northeast(nullptr), northwest(nullptr), southwest(nullptr), southeast(nullptr), value(_value)
{}

void Node::draw(std::ostream& sink, const string& prefix, const string& as, const bool show_pointers) const {

    sink << prefix << "[" << as << "]: ";
    if(is_leaf()){
        sink << static_cast<int>(get_value());
    }
    //sink << ' ' << bounds.str();
    if(show_pointers){
        sink << "   @:" << this << endl;
    }
    sink << endl;
    
    if(!is_leaf()){
        auto next_prefix = prefix + "    ";
        northeast->draw(sink, next_prefix, "NE", show_pointers);
        northwest->draw(sink, next_prefix, "NW", show_pointers);
        southwest->draw(sink, next_prefix, "SW", show_pointers);
        southeast->draw(sink, next_prefix, "SE", show_pointers);
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

bool Node::operator==(const Node& other) const {
    return static_cast<const void*>(this) == static_cast<const void*>(&other);
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

Node& Node::search(const Eigen::Vector2d& p, const geometry::Bounds bounds) {
    if(is_leaf()){
        return *this;
    }

    const Vector2d& center = bounds.center; 
    const double quarter_width = bounds.half_width/2;
    const double half_width = bounds.half_width;

    if(p[0] > bounds.center[0]){
        if( p[1] > bounds.center[1]){
            Bounds next_bounds(center + Vector2d(quarter_width, quarter_width), half_width);
            return this->northeast->search(p, next_bounds);
        }else{
            Bounds next_bounds(center + Vector2d(quarter_width, -quarter_width), half_width);
            return this->southeast->search(p, next_bounds);
        }
    }else{
        if( p[1] > bounds.center[1]){
            Bounds next_bounds(center + Vector2d(-quarter_width, quarter_width), half_width);
            return this->northwest->search(p, next_bounds);
        }else{
            Bounds next_bounds(center + Vector2d(-quarter_width, -quarter_width), half_width);
            return this->southwest->search(p, next_bounds);
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
    if(is_leaf()){
        value = NAN;

        this->northeast = make_unique<Node>(value);
        this->northwest = make_unique<Node>(value);
        this->southeast = make_unique<Node>(value);
        this->southwest = make_unique<Node>(value);
    }
}

void Node::split(const double precision, const double width){
    if(precision >= width){
        return;
    }
    
    if(is_leaf()){
        split();
    }

    const double half_width = width / 2;

    this->northeast->split(precision, half_width);
    this->northwest->split(precision, half_width);
    this->southeast->split(precision, half_width);
    this->southwest->split(precision, half_width);
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
