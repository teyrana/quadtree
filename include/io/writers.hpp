/// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _TERRAIN_WRITERS_HPP_
#define _TERRAIN_WRITERS_HPP_

#include <cstdio>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/cell_value.hpp"
#include "geometry/layout.hpp"
#include "io/json.hpp"


namespace terrain::io {


///! \brief writes a json document to given output stream
template<typename terrain_t>
bool to_json(const terrain_t& t, std::ostream& document);

///! \brief outputs this terrain into the given json document
template<typename terrain_t>
bool to_json_grid(const terrain_t& t, nlohmann::json& doc);

///! \brief writes a png file to the given outstream
template<typename terrain_t>
bool to_png(const terrain_t& t, const std::string& filename);

template<typename terrain_t>
bool to_png(const terrain_t& t, FILE* dest);


}; // namespace terrain::io

#include "writers.inl"

#endif  // #ifdef _TERRAIN_WRITERS_HPP_
