// The MIT License 
// (c) 2019 Daniel Williams

#ifndef _GRID_LAYOUT_HPP_
#define _GRID_LAYOUT_HPP_

#include <memory>
#include <string>

#include <Eigen/Geometry>

#include <nlohmann/json/json_fwd.hpp>

using Eigen::Vector2d;

namespace terrain::geometry {

///! \brief Layout is used to encapsulate common logic about how to layout a square grid
class Layout {
public:

    constexpr Layout();
    // constexpr Layout()
    //     : precision(1), x(0), y(0), width(1) {}

    constexpr Layout(const double _precision, const double _x, const double _y, const double _width);

    bool contains(const Eigen::Vector2d& at) const;

    bool operator!=(const Layout& other) const;
    bool operator==(const Layout& other) const;

    // definitely _not_ constexpr ;)
    Layout(nlohmann::json& doc);

    void clear();
    
    const Vector2d get_anchor() const;
    inline Vector2d get_center() const { return {x,y}; }
    inline size_t get_dimension() const { return dimension; }
    inline double get_half_width() const { return half_width; }
    inline double get_precision() const { return precision; }
    inline size_t get_size() const { return size; }
    double get_x() const { return x; }
    double get_x_max() const;
    double get_x_min() const;
    double get_y() const { return y; }
    double get_y_max() const;
    double get_y_min() const;
    inline size_t get_width() const { return width; }


    ///! \brief factory method for creating from a json document
    static std::unique_ptr<Layout> make_from_json(nlohmann::json& doc);

    double constrain_x( double x) const;
    double constrain_y( double y) const;

    nlohmann::json to_json() const;

    std::string to_string() const;

// constants
public:
    // used for comparisons
    constexpr static double epsilon = 1e-6;

private:
    ///! \brief snaps this precision to match the next-power-of-2 dimension that covers the width
    ///! dimension * precision = width
    constexpr double snap_precision(const double precision, const double width);

private:  // primary variables
    double precision;

    double width;

    // Center Coordinates:
    // =====
    // these are stored as the raw doubles, (instead of Eigen::Vector2d) because ...
    //    (1) the layout can compress the layout,
    //    (2) allow the use of constexpr constructors
    double x;
    double y;

private: // secondary / cached variables
    size_t dimension;
    double half_width;
    size_t size;
};

#include "layout.inl"

} // namespace terrain::geometry

#endif // #ifdef _GRID_LAYOUT_HPP_
