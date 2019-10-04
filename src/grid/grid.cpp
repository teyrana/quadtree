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

#include <nlohmann/json/json.hpp>

#include "geometry/layout.hpp"

#include "grid/grid.hpp"

using terrain::geometry::Bounds;
using terrain::geometry::Polygon;
using terrain::geometry::Layout;

using terrain::grid::Grid;

// a read-only value, used to absorb out-of-bounds writes
static cell_value_t scratch;


Grid::Grid(): 
    Grid(Layout::default_bounds, Layout::default_precision)
{
    reset();
}

Grid::Grid(const Bounds& _bounds, double _precision):
    layout(new geometry::Layout(_bounds, _precision))
{
    reset();
}


const Point Grid::anchor() const {
    const double width_2 = layout->bounds.half_width;
    return layout->bounds.center.sub({width_2, width_2});
}

bool Grid::contains(const Point& p) const {
    return layout->bounds.contains(p);
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

cell_value_t& Grid::search(const Point& p) {
    if(contains(p)){
        return get_cell(x_to_index(p.x), y_to_index(p.y));
    }

    scratch = cell_error_value;
    return scratch;
}

size_t Grid::size() const {
    return storage.size();
}


double Grid::width() const {
    return layout->bounds.width();
}

size_t Grid::x_to_index(double x) const {
    if(x < layout->bounds.get_x_min()){
        return 0;
    }else if(x > layout->bounds.get_x_max()){
        return layout->dimension-1;
    }

    return static_cast<size_t>((x - layout->bounds.center.x + layout->bounds.half_width)/layout->precision);
}

size_t Grid::y_to_index(double y) const {
    if(y < layout->bounds.get_y_min()){
        return 0;
    }else if(y > layout->bounds.get_y_max()){
        return layout->dimension-1;
    }

    return static_cast<size_t>((y - layout->bounds.center.y + layout->bounds.half_width)/layout->precision);
}
