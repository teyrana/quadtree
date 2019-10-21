/// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

// attempt to work through Sean-Parent's talk:
//   https://channel9.msdn.com/Events/GoingNative/2013/Inheritance-Is-The-Base-Class-of-Evil
//   https://sean-parent.stlab.cc/presentations/2013-09-06-inheritance/value-semantics-unique.cpp
// This StackOverflow answer: 
//   https://stackoverflow.com/questions/318064/how-do-you-declare-an-interface-in-c/17299151#17299151

#include <cstdio>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

#include "geometry/cell_value.hpp"
#include "geometry/layout.hpp"
#include "geometry/polygon.hpp"


namespace terrain {

///! \brief generates a sample json document, used for creating debug terrain
nlohmann::json generate_diamond(const double width,
                                const double desired_precision);

template<typename T>
class Terrain {
public:
    T& impl;
    std::string error_message;

    Terrain();

    Terrain(T& _ref);

    geometry::cell_value_t classify(const Eigen::Vector2d& p) const;

    ///! \brief writes debug information to std err
    void debug() const;

    // sets grid to all zeros
    void inline fill(const geometry::cell_value_t _value);

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void inline fill(const geometry::Polygon& source, const geometry::cell_value_t fill_value);

    ///! \brief loads a json document from the given input stream
    bool load_from_json_stream(std::istream& source);

    ///! \brief counts the number of cells *actually* tracked
    size_t get_count() const;

    ///! \brief describes the number of divisions along each of the x, y dimensions
    size_t get_dimension() const;

    const std::string& get_error() const;

    ///! \brief retreives the boundary information
    const geometry::Layout& get_layout() const;

    ///! \brief describes how much of the data structure is actively
    ///! being used
    double get_load_factor() const;

    ///! \brief gets the total count of cells in this data structure
    size_t get_size() const;

    std::string summary() const;

    ///! \brief writes a json document to given output stream
    bool to_json(std::ostream& document);

    ///! \brief writes a png file to the given outstream
    bool to_png(const std::string& filename);
    
private:
    ///! \brief 
    bool load_grid_from_json(nlohmann::json grid );

    ///! \brief loads all the allowed and blocked areas
    ///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
    ///! @param block - a (json) list of blocked areas, as defined by polygons, as defined by a list of points.
    bool load_areas_from_json(nlohmann::json allow, nlohmann::json block);

    ///! \brief loads list of polygons from json, into a structure
    ///!
    ///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
    ///! @return vector of polygons
    std::vector<geometry::Polygon> make_polygons_from_json(nlohmann::json list);

    nlohmann::json to_json_grid() const;

    bool to_png(FILE* dest);

}; // class Terrain<T>

}; // namespace terrain

#include "terrain.inl"

#endif  // #ifdef _TERRAIN_HPP_
