// The MIT License 
// (c) 2019 Daniel Williams

#include <memory>
#include <string>
#include <iostream>

#include <Eigen/Geometry>

#include "geometry/cell_value.hpp"
#include "geometry/sample.hpp"

using std::cerr;
using std::endl;
using std::make_unique;
using std::string;
using std::unique_ptr;

using Eigen::Vector2d;

using terrain::geometry::cell_value_t;
using terrain::geometry::Sample;



cell_value_t interpolate_linear(const Eigen::Vector2d& at, const Node& node2) const {
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


cell_value_t interpolate_bilinear(const Eigen::Vector2d& at, 
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
