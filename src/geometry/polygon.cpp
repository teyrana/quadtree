#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <vector>

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>

#include "geometry/polygon.hpp"

using std::cerr;
using std::max;
using std::min;
using std::endl;
using std::ostream;
using std::string;

using Eigen::Vector2d;

using terrain::geometry::Polygon;

//---------------------------------------------------------------
// Constructor
//
Polygon::Polygon(){
    set_default();
}

Polygon::Polygon(const size_t initial_capacity):
    points(initial_capacity)
{
    set_default();
}

Polygon::Polygon(nlohmann::json doc){
    load(doc);
}

Polygon::Polygon(std::vector<Vector2d>& init){
    load(init);
}

Polygon::Polygon(std::initializer_list<Vector2d> init_list)
{
    std::vector<Vector2d> pts = init_list;

    load(pts);
}

void Polygon::clear(){
    points.clear();
    bounds.clear();
}

void Polygon::complete(){
    update_bounds();

    // cerr << "====== ====== ====== " << endl;
    // write_yaml(cerr, "    ");

    enclose_polygon();
    if(! is_right_handed()){
        std::reverse(std::begin(points), std::end(points));
    }
}

void Polygon::emplace(const double x, const double y){
    points.emplace_back(x,y);
}

void Polygon::enclose_polygon(){
    // ensure that polygon loops back
    const auto& first_Vector2d = points[0];
    const auto& last_Vector2d = points[points.size()-1];

    if( ! first_Vector2d.isApprox(last_Vector2d)){
        points.emplace_back(first_Vector2d);
    }
}


const Bounds& Polygon::get_bounds() const {
    return bounds;
}

bool Polygon::is_right_handed() const {
    double sum = 0;

    // NOTE THE RANGE!:
    //   this needs to iterate over all doubles
    for( uint i = 0; i < (points.size()-1); ++i ){
        auto& p1 = points[i];
        auto& p2 = points[i+1];
        sum += (p1[0] * p2[1]) - (p1[1]*p2[0]);
    }

    // // The shoelace formula includes a divide-by-2 step; ... but we don't
    // // care about the magnitude, just the sign.  So skip this step.
    // sum /= 2;

    if( 0 > sum ){
        // left-handed / clockwise points
        return false;
    }else{
        // right-handed / counter-clockwise points
        return true;
    }
}

bool Polygon::load(std::vector<Vector2d> source){
    // if the new polygon contains insufficient points, abort and clear.
    if(4 > source.size()){
        return false;
    }

    points = std::move(source);
    
    complete();

    return true;
}

bool Polygon::load(nlohmann::json doc){
    if(doc.is_array() && doc[0].is_array() && (4 <= doc.size()) ){
        clear();
        points.resize(doc.size());

        size_t pair_index = 0;
        for( auto& pair : doc){
            Vector2d p(pair[0].get<double>(), pair[1].get<double>());
            points[pair_index] = p;
            ++pair_index;
        }

        complete(); 

        return true;
    }

    return false;
}

Vector2d& Polygon::operator[](const size_t index){
    return points[index];
}

const Vector2d& Polygon::operator[](const size_t index) const {
    return points[index];
}

void Polygon::push_back(const Vector2d p){
    points.push_back(p);
}

size_t Polygon::size() const {
    return points.size();
}

void Polygon::set_default(){
    points.clear();
    points.emplace_back( 0, 0);
    points.emplace_back( 1, 0);
    points.emplace_back( 1, 1);
    points.emplace_back( 0, 1);
    update_bounds();
}

void Polygon::update_bounds() {
    bounds.clear();

    Vector2d max(FLT_MIN, FLT_MIN);
    Vector2d min(FLT_MAX, FLT_MAX);
    
    for( uint i = 0; i < points.size(); ++i ){
        auto& p = points[i];
        max[0] = std::max(max[0], p[0]);
        max[1] = std::max(max[1], p[1]);
        min[0] = std::min(min[0], p[0]);
        min[1] = std::min(min[1], p[1]);
    }

    // the center is simply the midpoint between the extreme bounds:
    bounds.center = (min + max) / 2 ;

    auto r1 = max - min;
    auto r2 = r1.maxCoeff();

    bounds.half_width = r2 / 2;
}


void Polygon::write_yaml(std::ostream& sink, string indent) const {
    sink << indent << "points: \n";
    for( uint i = 0; i < points.size(); ++i ){
        auto& p = points[i];
        sink << indent << "    - " << p[0] << ", " << p[1] << '\n';
    }

    sink << indent << "bounds: \n";
    sink << indent << "    center: [" << bounds.center[0] << ", " << bounds.center[1] << "]\n";
    sink << indent << "    x: [" << bounds.get_x_min() << ", " << bounds.get_x_max() << "]\n";
    sink << indent << "    y: [" << bounds.get_y_min() << ", " << bounds.get_y_max() << "]\n";

}
