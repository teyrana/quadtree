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

using terrain::geometry::cell_value_t;
using terrain::geometry::Polygon;
using terrain::geometry::Layout;

using terrain::grid::Grid;

Grid::Grid(): 
    layout(Layout())
{
    reset();
}

Grid::Grid(const Layout& _layout):
    layout(_layout)
{
    reset();
}


bool Grid::contains(const Vector2d& p) const {
    return layout.contains(p);
}

cell_value_t Grid::classify(const Vector2d& p) const {
    if(contains(p)){
	const double width_2 = layout.get_half_width();
	const double precision = layout.get_precision();
        const size_t x_index = (p[0] - layout.get_x() + width_2)/precision;
        const size_t y_index = (p[1] - layout.get_y() + width_2)/precision;
        return get_cell(x_index, y_index);
    }

    return geometry::cell_default_value;
}

void Grid::fill(const cell_value_t value){
    memset(storage.data(), value, size());
}

cell_value_t& Grid::get_cell(const size_t xi, const size_t yi) {
    return storage[xi + yi * layout.get_dimension()];
}

cell_value_t Grid::get_cell(const size_t xi, const size_t yi) const {
    return storage[xi + yi * layout.get_dimension()];
}

size_t Grid::get_memory_usage() const { 
    return layout.get_size() * sizeof(cell_value_t);
}

void Grid::reset() {
    storage.resize( layout.get_size() );
}

void Grid::reset(const Layout& new_layout){
    layout = new_layout;
    
    reset();
}

size_t Grid::size() const {
    return storage.size();
}

bool Grid::store(const Vector2d& p, const cell_value_t new_value) {
    if(contains(p)){
	const double width_2 = layout.get_half_width();
	const double precision = layout.get_precision();
        const size_t x_index = (p[0] - layout.get_x() + width_2)/precision;
        const size_t y_index = (p[1] - layout.get_y() + width_2)/precision;
        get_cell(x_index, y_index) = new_value;
        return true;
    }

    return false;
}

