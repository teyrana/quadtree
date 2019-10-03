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

#include "grid/grid.hpp"

using terrain::geometry::Bounds;
using terrain::geometry::Polygon;

using terrain::grid::Grid;

// a read-only value, used to absorb out-of-bounds writes
static cell_value_t scratch;


Grid::Grid(): Grid(default_bounds, default_bounds.width()/2) {  
    reset(); 
}

Grid::Grid(const Bounds& _bounds, double _precision):
    bounds(_bounds), precision(_precision)
{
    reset();
}

const Point Grid::anchor() const {
    return bounds.center.sub({bounds.half_width, bounds.half_width});
}

bool Grid::contains(const Point& p) const {
    return bounds.contains(p);
}

void Grid::debug() const{
    auto& bounds = get_bounds(); 
    cerr << "====== Grid: ======\n";
    cerr << bounds.str() << "\n";

    for( size_t yi = dimension()-1; yi < dimension() ; --yi){
        for( size_t xi = 0; xi < dimension(); ++xi){
            const auto value = get_cell(xi,yi);
            if(0 == value){
                cerr << "   ,";
            }else{
                cerr << " " << setfill('0') << setw(2) << hex << static_cast<int>(value) << ",";
            }
        }
        cerr << '\n';
    }
    cerr << endl;
}

size_t Grid::dimension() const {
    return bounds.width() / precision;
}

void Grid::fill(const cell_value_t value){
    memset(storage.data(), value, size());
}

cell_value_t& Grid::get_cell(const size_t xi, const size_t yi) {
    return storage[xi + yi*dimension()];
}

cell_value_t Grid::get_cell(const size_t xi, const size_t yi) const {
    return storage[xi + yi*dimension()];
}

const Bounds& Grid::get_bounds() const {
    return bounds;
}

double Grid::get_precision() const {
    return precision;
}

bool Grid::load_grid(nlohmann::json& doc){
    if(!doc.is_array() || !doc[0].is_array()){
        cerr << "Grid::load_grid expected a array-of-arrays! aborting!\n";
        return false;
    }

    if( doc.size() != dimension() ){
        cerr << "Grid::load_grid expected a array of the same dimension as configured!!\n";
        return false;
    }

    const size_t dim = dimension();
    size_t row_index = dim - 1;
    for( auto& row : doc ){
        size_t column_index = 0;
        for( auto& cell : row){
            get_cell(column_index, row_index) = cell.get<cell_value_t>();
            column_index++;
        }
        --row_index;
    }

    return true;
}
void Grid::reset() {
    precision = snap_precision( bounds.width(), precision);
    storage.resize( dimension()*dimension() );
}

void Grid::reset(const Bounds new_bounds, const double new_precision){
    this->bounds = new_bounds;
    this->precision = snap_precision( bounds.width(), new_precision );
    storage.resize( dimension()*dimension() );
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

double Grid::snap_precision(const double width, const double precision){
    if( epsilon < std::fmod(width, precision)){
      const double dimension_guess = width/precision;
      const size_t next_power_of_2 = pow(2, ceil(log2(dimension_guess)));
      return width / next_power_of_2;
    }

    return precision;
}

size_t Grid::x_to_index(double x) const {
    if(x < bounds.get_x_min()){
        return 0;
    }else if(x > bounds.get_x_max()){
        return dimension()-1;
    }

    return static_cast<size_t>((x - bounds.center.x + bounds.half_width)/precision);
}

size_t Grid::y_to_index(double y) const {
    if(y < bounds.get_y_min()){
        return 0;
    }else if(y > bounds.get_y_max()){
        return dimension()-1;
    }

    return static_cast<size_t>((y - bounds.center.y + bounds.half_width)/precision);
}
