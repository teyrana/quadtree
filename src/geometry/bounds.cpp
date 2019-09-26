// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "nlohmann/json/json.hpp"

#include "geometry/point.hpp"
#include "geometry/bounds.hpp"

using std::abs;
using std::cerr;
using std::endl;
using std::max;
using std::string;

using terrain::geometry::Point;
using terrain::geometry::Bounds;

Bounds::Bounds(nlohmann::json& doc){
    /* bool result = */ load(doc);
}

void Bounds::clear() {
    center.clear();
    half_width = NAN;
}

bool Bounds::contains(const Point& at) const {
    // outside x-bounds:
    if( (at.x < center.x - half_width) || (at.x > center.x + half_width) ){
        return false;
    }

    // outside y-bounds:
    if( (at.y < center.y - half_width) || (at.y > center.y + half_width) ){ 
        return false;
    }
    
    return true;
}

double Bounds::get_size() const {
    return half_width*2;
}

double Bounds::get_x_max() const {
    return center.x + half_width;
}

double Bounds::get_x_min() const {
    return center.x - half_width;
}

double Bounds::get_y_max() const {
    return center.y + half_width;
}

double Bounds::get_y_min() const {
    return center.y - half_width;
}

double Bounds::get_width() const {
    return half_width*2;
}

bool Bounds::load(nlohmann::json& doc) {
    const string x_key("x");
    const string y_key("y");
    const string width_key("width");

    if(!doc.contains(x_key) || !doc[x_key].is_number()){
        return false;
    }else if(!doc.contains(y_key) || !doc[y_key].is_number()){
        return false;
    }else if(!doc.contains(width_key) || !doc[width_key].is_number()){
        return false;
    }

    center.x = doc[x_key].get<double>();
    center.y = doc[y_key].get<double>();
    half_width = doc[width_key].get<double>() * 0.5;

    return true;
}

bool Bounds::operator!=(const Bounds& other) const {
    return ! this->operator==(other);
}

bool Bounds::operator==(const Bounds& other) const {
    return ( this->center.near(other.center)) &&
           (Bounds::epsilon > std::abs(this->half_width - other.half_width));
}

double Bounds::snapx( double x) const {
    return std::max(get_x_min(), std::min(x, get_x_max()));
}

double Bounds::snapy( double y) const {
    return std::max(get_y_min(), std::min(y, get_y_max()));
}

string Bounds::str() const {
    std::ostringstream sink; 
    sink << "@" << center.str() << " \u00B1" << half_width;
    return sink.str();
}

nlohmann::json Bounds::to_json() const {
    nlohmann::json buf; 
    buf["x"] = center.x;
    buf["y"] = center.y;
    buf["width"] = half_width * 2.0;
    return buf;
}

double Bounds::width() const {
    return 2*half_width;
}