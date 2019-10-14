// The MIT License 
// (c) 2019 Daniel Williams

#include <cstring>

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>

#include "geometry/grid_layout.hpp"

using std::string;
using Eigen::Vector2d;

namespace terrain::geometry {

const string x_key("x");
const string y_key("y");
const string precision_key("precision");
const string width_key("width");

const Vector2d GridLayout::anchor() const {
    return {get_x_min(), get_y_min()};
}

std::optional<GridLayout> GridLayout::make(nlohmann::json& doc){
    if(!doc.contains(x_key) || !doc[x_key].is_number()){
        return {};
    }else if(!doc.contains(y_key) || !doc[y_key].is_number()){
        return {};
    }else if(!doc.contains(precision_key) || !doc[precision_key].is_number()){
        return {};
    }else if(!doc.contains(width_key) || !doc[width_key].is_number()){
        return {};
    }

    const double precision = doc[precision_key].get<double>();
    const double x = doc[x_key].get<double>();
    const double y = doc[y_key].get<double>();
    const double width = doc[width_key].get<double>();

    return GridLayout(precision, x, y, width);

}

double GridLayout::constrain_x( double x) const {
    return std::max(get_x_min(), std::min(x, get_x_max()));
}

double GridLayout::constrain_y( double y) const {
    return std::max(get_y_min(), std::min(y, get_y_max()));
}

bool GridLayout::contains(const Vector2d& at) const {
    // outside x-bounds:
    if( (at[0] < x - half_width) || (at[0] > x + half_width) ){
        return false;
    }

    // outside y-bounds:
    if( (at[1] < y - half_width) || (at[1] > y + half_width) ){ 
        return false;
    }
    return true;
}

double GridLayout::get_x_max() const {
    return x + half_width;
}
double GridLayout::get_x_min() const {
    return x - half_width;
}
double GridLayout::get_y_max() const {
    return y + half_width;
}
double GridLayout::get_y_min() const {
    return y - half_width;
}

bool GridLayout::operator!=(const GridLayout& other) const {
    return ! (this->operator==(other));
}

bool GridLayout::operator==(const GridLayout& other) const {
    const double error =  abs(precision - other.precision) 
                        + abs(x - other.x)
                        + abs(y - other.y)
                        + abs(width - other.width);

    if( epsilon > error){
        return true;
    }

    return false;
}

std::string GridLayout::to_string() const {
    return "x: " + std::to_string(x)\
        +  ", y: " + std::to_string(y)\
        +  ", prec: " + std::to_string(precision)\
        +  ", width: " + std::to_string(width); 
}

} // namespace terrain::geometry
