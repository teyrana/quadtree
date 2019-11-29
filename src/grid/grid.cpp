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
        return storage[layout.rhash(p.x(), p.y())];
    }

    return geometry::cell_default_value;
}

void Grid::fill(const cell_value_t value){
    memset(storage.data(), value, size());
}

cell_value_t& Grid::get_cell(const size_t xi, const size_t yi) {
    return storage[layout.rhash(static_cast<uint32_t>(xi), static_cast<uint32_t>(yi))];
}

cell_value_t Grid::get_cell(const size_t xi, const size_t yi) const {
    return storage[layout.rhash(static_cast<uint32_t>(xi), static_cast<uint32_t>(yi))];
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
        storage[layout.rhash(p.x(), p.y())] = new_value;
        return true;
    }

    return false;
}

