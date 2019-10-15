// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _INTERPOLATE_HPP_
#define _INTERPOLATE_HPP_

#include <cstdint>
#include <iostream>
#include <string>

#include "cell_value.hpp"
#include "geometry/interpolate.hpp"

using terrain::cell_value_t;

namespace terrain::geometry {

/**
 * Performs the low-level interpolation between this node and another node, at the requested location
 *
 * @param {Point} the x,y coordinates to interpolate at.
 * @param {quadtree::Node} n2 the other node to interpolate
 * @return {cell_value_t} The resultant value
 */
cell_value_t interpolate_linear(const Eigen::Vector2d& at, const Node& n2) const;

/**
 * Performs bilinear-interpolation: 
 * http://en.wikipedia.org/wiki/Bilinear_Interpolation
 * 
 * @param {Point} the x,y coordinates to interpolate at.
 * @param {quadtree::Node} xn x-neighbor node to interpolate with
 * @param {quadtree::Node} dn diagonal-neighbor node to interpolate with
 * @param {quadtree::Node} yn y-neighbor node to interpolate with
 * @return {cell_value_t} The resultant value
 */
cell_value_t interpolate_bilinear(const Eigen::Vector2d& at, 
                                    const Node& xn,
                                    const Node& dn,
                                    const Node& yn) const;

} // namespace terrain::geometry

#endif // #ifndef _INTERPOLATE_HPP_
