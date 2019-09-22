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

#include "quadtree/grid.hpp"

using geometry::Bounds;
using geometry::Polygon;

using quadtree::Grid;

Grid::Grid(double _size, double _spacing, const Point _center):
    bounds(_center, _size),
    dimension(static_cast<size_t>(_size/_spacing)),
    spacing(_spacing),
    storage(new grid_value_t[dimension*dimension])
{}

const Point Grid::anchor() const {
    return bounds.center.sub({bounds.half_width, bounds.half_width});
}

bool Grid::contains(const Point& p) const {
    return bounds.contains(p);
}

void Grid::draw(std::ostream& sink) const{
    auto& bounds = get_bounds(); 
    sink << "====== Tree: ======\n";
    sink << bounds << "\n";

    for( int yi = dimension-1; 0 <= yi ; --yi){
        sink << "   ";
    
        for( int xi = 0; xi < dimension; ++xi){
            const auto value = get(xi,yi);
            if(0 == value){
                sink << "   ,";
            }else{
                sink << " " << setfill('0') << setw(2) << hex << static_cast<int>(value) << ",";
            }
        }
        sink << '\n';
    }
    sink << '\n' << endl;
}

void Grid::fill(const grid_value_t _value){
    memset(storage.get(), _value, size());
}

const Bounds& Grid::get_bounds() const {
    return bounds;
}

grid_value_t Grid::get_default_value(const Point& at) const {
    return 99;
}

size_t Grid::grid_to_storage(size_t xi, size_t yi) const {
    return yi*dimension + xi;
}

void Grid::load_grid(std::istream& input){
    // TODO: expand this to accept a json file format

    size_t row_index = dimension - 1;
    std::vector<grid_value_t> buf;
    while(input.good()){
        if (',' == input.peek()){
            input.ignore(); // consume this ','
            
        }else if(' ' == input.peek()){
            // ignore this entire group of spaces
            input.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

        }else if('\n' == input.peek()){
            // trigger load buffer to grid
            input.ignore(); // consume this '\n'

            assert(dimension == buf.size());

            memcpy(storage.get()+row_index*dimension, buf.data(), buf.size());

            buf.clear();
            --row_index;

        }else{
            int v;
            input >> v;
            buf.push_back(static_cast<grid_value_t>(v));
        }
    }


    // if(!(doc.is_object() && doc.contains("bounds"))){
    //     // 'bounds' is necessary for any well-formed tree serialization
    //     return;
    // }else{
    //     auto& bounds = doc["bounds"];
    //     if(!(bounds.contains("x") && bounds.contains("y") && bounds.contains("width"))){
    //         return;
    //     }

    //     // load contents
    //     const Point pt = { bounds["x"].get<double>(), bounds["y"].get<double>()};
    //     const double width = bounds["width"].get<double>();
}

void Grid::fill(const Polygon& source, const grid_value_t fill_value){
    // adapted from:
    //  Public-domain code by Darel Rex Finley, 2007:  "Efficient Polygon Fill Algorithm With C Code Sample"
    //  Retrieved: (https://alienryderflex.com/polygon_fill/); 2019-09-07

    // Loop through the rows of the image.
    // for( int yi = 0; yi < size(); yi += dimension){
    for( int yi = dimension-1; 0 <= yi; --yi){
        const double y = bounds.center.y + yi*spacing - bounds.half_width;

        // generate a list of line-segment crossings from the polygon
        const std::vector<Point>& pts = source.get_points();
        std::vector<double> crossings;
        for (int xi=0; xi < pts.size() - 1; ) {
            const Point& p1 = pts[xi];
            const Point& p2 = pts[++xi];

            // if y is in range:
            const double y_max = std::max(p1.y, p2.y);
            const double y_min = std::min(p1.y, p2.y);
            if( (y_min <= y) && (y < y_max) ){
                // construct x-coordinate that crosses this line:
                crossings.emplace_back(p1.x + (y - p1.y) * (p2.x-p1.x)/(p2.y - p1.y));
            }
        }

        // early exit
        if( 0 == crossings.size()){
            continue;
        }

        // Sort the crossings:
        std::sort(crossings.begin(), crossings.end());

        //  Fill the pixels between node pairs.
        for( int xing = 0; xing < crossings.size(); xing += 2){
            const double startx = bounds.snapx(crossings[xing]);
            const double endx = bounds.snapx(crossings[xing+1]);

            const size_t start_xi = x_to_index(startx);
            const size_t dxi = x_to_index(endx) - start_xi;
            const size_t start_index = grid_to_storage(start_xi, yi);

            memset(storage.get() + start_index, fill_value, dxi);
        }
    }
}


grid_value_t Grid::get(const size_t xi, const size_t yi) const {
    return storage.get()[xi + yi*dimension];
}

grid_value_t& Grid::operator()(const size_t xi, const size_t yi){
    return storage.get()[xi + yi*dimension];
}


grid_value_t Grid::search(const Point& at) {
    if(contains(at)){
        const Point& delta = at.sub(anchor());
        const size_t x_index = static_cast<int>(delta.x / spacing);
        const size_t y_index = static_cast<int>(delta.y / spacing);
        return get(x_index, y_index);
    }

    return get_default_value(at);

}

void Grid::serialize(std::ostream& sink) const {
    nlohmann::json doc;

    // output the bounds once, at the root-level, and all the bounds
    // for subsequent layers follow deterministicly
    const Bounds& bounds = get_bounds();
    doc["bounds"] = {
        {"x", bounds.center.x},
        {"y", bounds.center.y},
        // tree is implemented & enforced as square
        {"width", 2 * bounds.half_width}
    };

    // doc["values"] = values;

    // write result to output stream:
    sink << doc;
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
        this->dimension, this->dimension,
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
    const png_uint_32 height = this->dimension;
    const png_uint_32 width = this->dimension;
    png_bytep image = this->storage.get();
    unique_ptr<png_bytep[]> row_pointers(new png_bytep[height]);
    for (uint32_t i=0; i < height; ++i){
        row_pointers[height - 1 - i] = image + i*width*sizeof(grid_value_t);
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
        return dimension-1;
    }

    return static_cast<size_t>((x - (bounds.center.x - bounds.half_width))/spacing);
}

size_t Grid::y_to_index(double y) const {
    if(y < bounds.get_y_min()){
        return 0;
    }else if(y > bounds.get_y_max()){
        return dimension-1;
    }

    return static_cast<size_t>((y - (bounds.center.y - bounds.half_width))/spacing);
}