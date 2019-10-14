// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GRID_LAYOUT_HPP_
#define _GRID_LAYOUT_HPP_

#include <optional>
#include <string>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

using Eigen::Vector2d;

namespace terrain::geometry {

///! \brief SquareLayout is used to encapsulate common logic about how to layout a square grid
class GridLayout {
public:

    constexpr GridLayout();
    // constexpr GridLayout()
    //     : precision(1), x(0), y(0), width(1) {}

    constexpr GridLayout(const double _precision, const double _x, const double _y, const double _width);

    const Vector2d anchor() const;

    inline Vector2d center() const { return {x,y}; }

    bool contains(const Eigen::Vector2d& at) const;

    bool operator!=(const GridLayout& other) const;
    bool operator==(const GridLayout& other) const;

    // definitely _not_ constexpr ;)
    GridLayout(nlohmann::json& doc);

    void clear();

    double get_x_max() const;
    double get_x_min() const;
    double get_y_max() const;
    double get_y_min() const;

    ///! \brief factory method for creating from a json document
    static std::optional<GridLayout> make(nlohmann::json& doc);

    double constrain_x( double x) const;
    double constrain_y( double y) const;

    std::string to_string() const;

    nlohmann::json to_json() const;

// constants
public:
    // used for comparisons
    constexpr static double epsilon = 1e-6;

    // default values
    // constexpr static double default_center_x = 0;
    // constexpr static double default_center_y = 0;
    // constexpr static double default_width = 1.0;
    // constexpr static double default_precision = 1.0;

    // constexpr static size_t default_dimension = 1.0;
    // constexpr static size_t default_size = 1.0;

    // const static Layout default_layout;

private:
    ///! \brief snaps this precision to match the next-power-of-2 dimension that covers the width
    ///! dimension * precision = width
    constexpr double snap_precision(const double precision, const double width);

public: // primary variables
    const double precision;

    // Center Coordinates:
    // =====
    // these are stored as the raw doubles, (instead of Eigen::Vector2d) because ...
    //    (1) the layout can compress the layout,
    //    (2) allow the use of constexpr constructors
    const double x;
    const double y;

    const double width;

public: // secondary / cached variables
    const size_t dimension;
    const double half_width;
    const size_t size;
};

#include "grid_layout.inl"

} // namespace terrain::geometry

#endif // #ifdef _GRID_LAYOUT_HPP_
