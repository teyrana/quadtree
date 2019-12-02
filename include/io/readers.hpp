/// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _TERRAIN_READERS_HPP_
#define _TERRAIN_READERS_HPP_

#include <cstdio>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/cell_value.hpp"
#include "geometry/layout.hpp"
#include "geometry/polygon.hpp"
#include "terrain.hpp"

namespace terrain::io {


inline const cell_value_t allow_value = 0;
inline const cell_value_t block_value = 0x99;

///! \brief generates a sample json document, used for creating debug terrain
template<typename terrain_t>
nlohmann::json generate_diamond(terrain_t& terrain,
                                const double width,
                                const double desired_precision);

///! \brief loads a generic / unspecified json document from the given input stream
///! 
///! Note: this json document may contain either polygons or a grid, and will be automatically loaded correctly
template<typename terrain_t>
bool load_from_json_stream(terrain_t& terrain, std::istream& source);

///! \brief 
template<typename terrain_t>
bool load_grid_from_json(terrain_t& terrain, nlohmann::json grid );

///! \brief loads all the allowed and blocked areas
///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
///! @param block - a (json) list of blocked areas, as defined by polygons, as defined by a list of points.
template<typename terrain_t>
bool load_areas_from_json(terrain_t& terrain, nlohmann::json allow, nlohmann::json block);

///! \brief loads list of polygons from json, into a structure
///!
///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
///! @return vector of polygons
inline std::vector<geometry::Polygon> make_polygons_from_json(nlohmann::json list);

inline geometry::Polygon make_polygons_from_OGRLine( const OGRLinearRing& source );

///! \brief load a .shp file into this terrain.
template<typename terrain_t>
bool load_shape_from_file(terrain_t& terrain, const string& filepath);

}; // namespace terrain::io

#include "readers.inl"

#endif  // #ifdef _TERRAIN_READERS_HPP_
