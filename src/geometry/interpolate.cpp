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


namespace terrain::geometry {

cell_value_t interpolate_linear(const Eigen::Vector2d& to, const Sample& s1, const Sample& s2){
    if(s1.at.isApprox(s2.at)){
        return s1.is;
    }

    // distances from query point to each interpolation point
    const double dist1 = (s1.at - to).norm();
    const double dist2 = (s2.at - to).norm();

    // If the point is farther than from a point than the distance between the two interpolation points, 
    //     return the value at one of the end-points.
    // This is not perfect, but it's a reasonable heuristic.
    // ... in particular, it will return odd values at large distances
    // ... arguably, this should return a NAN value instead -- for not-applicable
    const double dist12 = (s1.at - s2.at).norm();
    if(dist12 < dist1){
        return s2.is;
    }else if( dist12 < dist2){
        return s1.is;
    }

    const double combined_distance = dist1 + dist2;
    const double normdist1 = 1 - dist1 / combined_distance;
    const double normdist2 = 1 - dist2 / combined_distance;
    const double interp_value = (normdist1*s1.is + normdist2*s2.is);

    return round(interp_value);
}

cell_value_t interpolate_bilinear(  const Eigen::Vector2d& to, 
                                    const Sample& ne,
                                    const Sample& nw,
                                    const Sample& sw,
                                    const Sample& se)
{
    // cerr << "    ==>>  to:    @" << to[0] << ", " << to[1] << endl;
    // cerr << "        - NE:     " << ne.at[0] << ", " << ne.at[1] << " = " << (int)ne.is << endl;
    // cerr << "        - NW:     " << nw.at[0] << ", " << nw.at[1] << " = " << (int)nw.is << endl;
    // cerr << "        - SW:     " << sw.at[0] << ", " << sw.at[1] << " = " << (int)sw.is << endl;
    // cerr << "        - SE:     " << se.at[0] << ", " << se.at[1] << " = " << (int)se.is << endl;

    // not necessary ?
    // // test for degenerate cases:
    // if( &sx == &sd ){
    //     // top or bottom border
    //     return interpolate_linear(to, {to[0], xn.at[1]}, xn);
    // }else if( &sy  == &sd ){
    //     // left or right border
    //     return interpolate_linear({yn[0], to[1]}, yn);
    // }

    // calculate full bilinear interpolation:
    const Eigen::Vector2d upper_point = {to[0], (nw.at[1] + ne.at[1])/2};
    const Sample upper_sample = { upper_point,
                                  interpolate_linear(upper_point, nw, ne)};


    const Eigen::Vector2d lower_point = {to[0], (sw.at[1] + se.at[1])/2};
    const Sample lower_sample = { lower_point,
                                interpolate_linear(lower_point, sw, se) };

    // cerr << "        ::Upper:    " << upper_point[0] << ", " << upper_point[1] << " = " << (int)upper_sample.is << endl;
    // cerr << "        ::Lower:    " << lower_point[0] << ", " << lower_point[1] << " = " << (int)lower_sample.is << endl;

    return interpolate_linear(to, upper_sample, lower_sample);
}

} // namespace terrain::geometry
