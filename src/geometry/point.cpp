// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>

#include "geometry/point.hpp"

using std::acos;
using std::sqrt;

using geometry::Point;

Point::Point() {
    this->x = NAN;
    this->y = NAN;
}

Point::Point(double _x, double _y) {
    this->x = _x;
    this->y = _y;
}

double Point::angle(const Point * other) const {
    if( this->is_zero() && other->is_zero() ){
        return NAN;
    }

    // there MUST be a more efficient way to calculate this...
    return acos(((this->x * other->x) + (this->y * other->y)) /
                (sqrt((this->x * this->x) + (this->y * this->y)) *
                 sqrt((other->x * other->x) + (other->y * other->y))));
}


// double Point::headingRadians() const {
//    return -atan2(y,x) + M_PI_2;
// }

void Point::clear(){
    this->x = NAN;
    this->y = NAN;
}

double Point::cross(const Point * other) const {
    // the order-of-operations is already well defined, here:
    // the parentheses are purely for humans
    return (this->x * other->y) - (this->y * other->x);
}


//---------------------------------------------------------------
// Procedure: calculateAngle
//   Purpose: calculates angles between three points, by exploiting:
//
//            $ a \dot b = ||a|| ||b|| cos(\theta) $
//
//
//      p2  +----------+ p3
//         /         /
//        /       /
//    A  /     / B
//      /   /
//     / /
// p1 +
//
//     p1 -> p2 === segment A
//     p1 -> p3 === segment C
//
// WARNING: PURPOSE UNKNOWN! 
// TODO: BROKEN
bool Point::curves_right(const Point& p1, const Point& p2, const Point& p3) {
    const Point segA(p2.x - p1.x, p2.y - p1.y);
    const Point segB(p3.x - p1.x, p3.y - p1.y);

    const double cross_product = segA.is_right_handed(&segB);

    return (0 > cross_product );
}


double Point::distance(const Point& other) const {
    return sqrt(pow(this->x - other.x, 2) + pow(this->y - other.y, 2));
}

double Point::is_right_handed(const Point * other) const {
    if( (this->x * other->y) < (this->y * other->x)){
        return -1.0;
    }else{
        // default. (NANs and zeros)
        return 1.0;
    }
}

double Point::dot(const Point * other) const {
    // the order-of-operations is already well defined, here:
    // the parentheses are purely for humans
    return (this->x * other->x) + (this->y * other->y);
}

bool Point::is_zero() const {
    if((abs(this->x) < epsilon) && (abs(this->y) < epsilon)){
        return true;
    }

    return false;
}

bool Point::near(const Point& other) const {
    if(epsilon > (abs(this->x - other.x) + abs(this->y - other.y))){
        return true;
    }
    return false;
}

double Point::norm() const {
    // the parentheses are purely for humans:
    return sqrt((this->x * this->x) + (this->y * this->y));
}

void Point::normalize() {
    const double norm = this->norm();
    if( 0 < norm ){
        x /= norm;
        y /= norm;
    }
}

void Point::set(double _x, double _y) {
    this->x = _x;
    this->y = _y;
}

Point Point::subtract(const Point * other) const {
    return {this->x - other->x, this->y - other->y};
}
