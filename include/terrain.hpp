/// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

// attempt to work through Sean-Parent's talk:
//   https://channel9.msdn.com/Events/GoingNative/2013/Inheritance-Is-The-Base-Class-of-Evil
//   https://sean-parent.stlab.cc/presentations/2013-09-06-inheritance/value-semantics-unique.cpp
// This StackOverflow answer: 
//   https://stackoverflow.com/questions/318064/how-do-you-declare-an-interface-in-c/17299151#17299151

#include <string>
#include <vector>

#include <nlohmann/json/json_fwd.hpp>

#include "cell_value.hpp"

#include "geometry/bounds.hpp"
#include "geometry/polygon.hpp"

namespace terrain {
template<typename T>
class Terrain {
public:
    T& impl;
    Terrain(T& _ref);

    ///! \brief writes debug information to std err
    void debug() const;

    // sets grid to all zeros
    void inline fill(const cell_value_t _value);

    /**
     * Loads the vector of points as a CCW polygon.
     *
     * @param {std::istream} input stream containing the serialization text
     */
    void inline fill(const geometry::Polygon& source, const cell_value_t fill_value);

    ///! \brief retreives the boundary information
    const geometry::Bounds& get_bounds() const;

    ///! \brief describes the precision of this lookup structure === cell size
    double get_precision() const;

    ///! \brief loads a json document from the given input stream
    bool load(std::istream& source);

    ///! \brief writes a png file to the given outstream
    bool png(FILE* dest);

    ///! \brief writes a json document to given output stream
    bool json(std::ostream& document);


private:
    ///! \brief loads all the allowed and blocked areas
    ///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
    ///! @param block - a (json) list of blocked areas, as defined by polygons, as defined by a list of points.
    bool load_polygons(nlohmann::json allow, nlohmann::json block);

    ///! \brief loads list of polygons from json, into a structure
    ///!
    ///! @param allow - a (json) list of allowed areas, as defined by polygons, as defined by a list of points.
    ///! @return vector of polygons
    std::vector<geometry::Polygon> make_polygons(nlohmann::json list);

    nlohmann::json to_json_grid() const;
};

}; // namespace terrain

#endif