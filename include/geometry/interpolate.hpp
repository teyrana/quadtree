// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _INTERPOLATE_HPP_
#define _INTERPOLATE_HPP_

#include <Eigen/Geometry>

#include "geometry/cell_value.hpp"
#include "geometry/sample.hpp"

namespace terrain::geometry {

/**
 * Performs the low-level interpolation between this node and another node, at the requested location
 *
 * @param {Point} the x,y coordinates to interpolate at.
 * @param {quadtree::Node} n2 the other node to interpolate
 * @return {cell_value_t} The resultant value
 */
cell_value_t interpolate_linear( const Eigen::Vector2d& at, const Sample& s1, const Sample& s2);

/**
 * Performs bilinear-interpolation: 
 * http://en.wikipedia.org/wiki/Bilinear_Interpolation
 * 
 * Warning: Undefined behavior, if interpolation happens outside of the rectangle defined by the 4 samples
 * 
 * @param the x,y coordinates to interpolate at.
 * @param ne quadrant sample point
 * @param nw quadrant sample point
 * @param sw quadrant sample point
 * @param se quadrant sample point
 * @return resultant value
 */
cell_value_t interpolate_bilinear( const Eigen::Vector2d& at, const Sample& ne, const Sample& nw, const Sample& sw, const Sample& se);

} // namespace terrain::geometry

#endif // #ifndef _INTERPOLATE_HPP_
