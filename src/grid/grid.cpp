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

#ifdef ENABLE_LIBPNG
#include <png.h>
#include <zlib.h>
#endif

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

bool Grid::to_png(const std::string filename) const {
#ifdef ENABLE_LIBPNG
    // from: < http://www.libpng.org/pub/png/libpng-manual.txt >
    // from: < https://dev.w3.org/Amaya/libpng/example.c >
    // (search for 'write_png' function near the end)
    
    FILE* fp = fopen(filename.c_str(), "wb");
    if(nullptr == fp){
        cerr << "could not open destination png file for reading." << endl;
        return false;
    }

    png_structp png_ptr;
    png_infop  info_ptr;
    //png_colorp palette;

    // Allocate and initialize libpng structures:
    {
        // the nulls at the end indicate we are using 'standard' error handling:
        //     i.e. 'setjmp' and 'png_jmpbuf'
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
        if (!png_ptr){
            cerr << "[Grid::to_png] png_create_write_struct failed" << endl;
            fclose(fp);
            return false;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr){
            cerr << "[Grid::to_png] png_create_info_struct failed" << endl;
            fclose(fp);
            png_destroy_write_struct(&png_ptr,  NULL);
            return false;
        }

        // Associated this structure with our file pointer:
        png_init_io(png_ptr, fp);
        if (setjmp(png_jmpbuf(png_ptr))){
            cerr << "[Grid::to_png] Error during init_io" << endl;
            fclose(fp);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            return false;
       }
    }

    // Write Header:
    // Output is 8-bit depth, grayscale, alpha-less format.
    png_set_IHDR(png_ptr, info_ptr,
        this->dimension(), this->dimension(),
        8, PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[Grid:to_png] Error while writing header info" << endl;
        return false;
    }
    
    // The easiest way to write the image (you may have a different memory
    // layout, however, so choose what fits your needs best).  You need to
    // use the first method if you aren't handling interlacing yourself.
    const png_uint_32 height = this->dimension();
    const png_uint_32 width = this->dimension();
    png_bytep image = const_cast<png_bytep>(storage.data());
    unique_ptr<png_bytep[]> row_pointers(new png_bytep[height]);
    for (uint32_t i=0; i < height; ++i){
        row_pointers[height - 1 - i] = image + i*width*sizeof(cell_value_t);
    }

    // write out the entire image data in one call
    png_write_image(png_ptr, row_pointers.get());
    // error during write
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[Grid::to_png] Error during end of write" << endl;
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }
    
    png_write_end(png_ptr, info_ptr);

    // clean up libpng structs
    if (png_ptr && info_ptr)
        png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp); // close file

    return true;
#else
    cerr << "libpng is disabled!! Could not save."
    return false;
#endif //#ifdef ENABLE_LIBPNG
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
