// The MIT License 
// (c) 2019 Daniel Williams

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>

using std::cerr;
using std::endl;
using std::hex;
using std::setfill;
using std::setw;
using std::string;
using std::unique_ptr;

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>

#include "geometry/layout.hpp"

#include "grid/grid.hpp"

using Eigen::Vector2d;

using terrain::geometry::Bounds;
using terrain::geometry::cell_value_t;
using terrain::geometry::Polygon;
using terrain::geometry::Layout;

using terrain::grid::Grid;

Grid::Grid(): 
    Grid(Layout::default_layout.bounds, Layout::default_precision)
{
    reset();
}

Grid::Grid(const Bounds& _bounds, double _precision):
    layout(new geometry::Layout(_bounds, _precision))
{
    reset();
}


const Vector2d Grid::anchor() const {
    const double width_2 = layout->bounds.half_width;
    return layout->bounds.center - Vector2d(width_2, width_2);
}

bool Grid::contains(const Vector2d& p) const {
    return layout->bounds.contains(p);
}

cell_value_t Grid::classify(const Vector2d& p) const {
    if(contains(p)){
        const size_t x_index = (p[0] - layout->bounds.center[0] + layout->bounds.half_width)/layout->precision;
        const size_t y_index = (p[1] - layout->bounds.center[1] + layout->bounds.half_width)/layout->precision;
        return get_cell(x_index, y_index);
    }

    return geometry::cell_default_value;
}

void Grid::fill(const cell_value_t value){
    memset(storage.data(), value, size());
}

const Bounds& Grid::get_bounds() const {
    return layout->bounds;
}

cell_value_t& Grid::get_cell(const size_t xi, const size_t yi) {
    return storage[xi + yi * get_dimension()];
}

cell_value_t Grid::get_cell(const size_t xi, const size_t yi) const {
    return storage[xi + yi * get_dimension()];
}

size_t Grid::get_dimension() const {
    return layout->dimension;
}

size_t Grid::get_memory_usage() const { 
    return layout->dimension * layout->dimension * sizeof(cell_value_t);
}

double Grid::get_precision() const {
    return layout->precision;
}

void Grid::reset() {
    storage.resize( layout->size );
}

void Grid::reset(const Bounds new_bounds, const double new_precision){
    layout.reset(new geometry::Layout(new_bounds, new_precision));
    
    reset();
}

size_t Grid::size() const {
    return storage.size();
}

bool Grid::store(const Vector2d& p, const cell_value_t new_value) {
    if(contains(p)){
        const size_t x_index = (p[0] - layout->bounds.center[0] + layout->bounds.half_width)/layout->precision;
        const size_t y_index = (p[1] - layout->bounds.center[1] + layout->bounds.half_width)/layout->precision;
        get_cell(x_index, y_index) = new_value;
        return true;
    }

    return false;
}

double Grid::width() const {
    return layout->bounds.width();
}

