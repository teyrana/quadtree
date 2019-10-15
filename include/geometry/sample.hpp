// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _SAMPLE_HPP_
#define _SAMPLE_HPP_

#include <cmath>

#include <Eigen/Geometry>

#include "cell_value.hpp"


namespace terrain::geometry {

struct Sample {
public:
    const Eigen::Vector2d at;
    const terrain::geometry::cell_value_t is;
};

} // namespace terrain

#endif // _SAMPLE_HPP_
