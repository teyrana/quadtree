// The MIT License 
// (c) 2019 Daniel Williams

// NOTE: This is the template-class implementation -- 
//       It is not compiled until referenced, even though it contains the function implementations.

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::string;

#include <nlohmann/json/json.hpp>

#ifdef ENABLE_LIBPNG
#include <png.h>
#include <zlib.h>
#endif

#include "geometry/bounds.hpp"
#include "geometry/polygon.hpp"
#include "grid/grid.hpp"
#include "terrain.hpp"

using terrain::geometry::Bounds;
using terrain::geometry::Polygon;
using terrain::Terrain;

// used for reading and write json documents:
const static inline string allow_key("allow");
const static inline string block_key("block");
const static inline string bounds_key("bounds");
const static inline string grid_key("grid");
const static inline string precision_key("precision");
const static inline string tree_key("tree");

template<typename T>
Terrain<T>::Terrain(): impl(*(new T())) {}

template<typename T>
Terrain<T>::Terrain(T& _ref): 
    impl(_ref) 
{}

template<typename T>
void Terrain<T>::debug() const {
    const Bounds& bounds = impl.get_bounds();
    const double precision = impl.get_precision();
    const size_t dimension = impl.get_dimension();

    cerr << "====== Structure: ======\n";
    cerr << "##  bounds:     " << bounds.str() << endl;
    // cerr << "##  height:     " << get_height() << endl;
    cerr << "##  precision:  " << precision << endl;
    cerr << "##  dimension:  " << dimension << endl;

    cerr << "           ======== ======== ======== ======== As Grid: ======== ======== ======== ========\n";
    // print header (x-axis-labels: 
    cerr << "            ";
    for(double x = (bounds.get_x_min() + precision/2); x < bounds.get_x_max(); x += precision){
        fprintf(stderr, "%4.1f ", x);
    } cerr << endl;
    // print top border
    cerr << "          +";
    for(double x = (bounds.get_x_min() + precision/2); x < bounds.get_x_max(); x += precision){
        fprintf(stderr, "-----");
    } cerr << "---+\n";

    for(double y = (bounds.get_y_max() - precision/2); y > bounds.get_y_min(); y -= precision ){
        // print left header:
        fprintf(stderr, "     %4.1f | ", y);
        for(double x = (bounds.get_x_min() + precision/2); x < bounds.get_x_max(); x += precision){
            auto value = impl.search({x,y});
            if( 0 < value ){
                fprintf(stderr, "  %2X,", static_cast<int>(value) );
            }else{
                cerr << "    ,";
            }
        }
        // print left header:
        fprintf(stderr, "  | %4.1f\n", y);
    }
    // print bottom border
    cerr << "          +";
    for(double x = (bounds.get_x_min() + precision/2); x < bounds.get_x_max(); x += precision){
        fprintf(stderr, "-----");
    } cerr << "---+\n";
    // print footer: (x-axis-labels: 
    cerr << "            ";
    for(double x = (bounds.get_x_min() + precision/2); x < bounds.get_x_max(); x += precision){
        fprintf(stderr, "%4.1f ", x);
    } cerr << endl << endl;
}

template<typename T>
void inline Terrain<T>::fill(const Polygon& poly, const cell_value_t fill_value){
    // adapted from:
    //  Public-domain code by Darel Rex Finley, 2007:  "Efficient Polygon Fill Algorithm With C Code Sample"
    //  Retrieved: (https://alienryderflex.com/polygon_fill/); 2019-09-07

    const Bounds& bounds = impl.get_bounds();
    const double precision = impl.get_precision();

    // Loop through the rows of the image.
    for( double y = precision/2; y < bounds.width(); y += precision ){
        // generate a list of line-segment crossings from the polygon
        std::vector<double> crossings;
        for (int i=0; i < poly.size()-1; ++i) {
            const Point& p1 = poly[i];
            const Point& p2 = poly[i+1];

            const double y_max = std::max(p1.y, p2.y);
            const double y_min = std::min(p1.y, p2.y);
            // if y is in range:
            if( (y_min <= y) && (y < y_max) ){
                // construct x-coordinate that crosses this line:
                auto value = p1.x + (y - p1.y) * (p2.x-p1.x)/(p2.y - p1.y);
                crossings.emplace_back(value);
            }
        }

        // early exit
        if( 0 == crossings.size()){
            continue;
        }

        // Sort the crossings:
        std::sort(crossings.begin(), crossings.end());
        
        //  Fill the pixels between node pairs.
        for( int crossing_index = 0; crossing_index < crossings.size(); crossing_index += 2){
            const double start_x = bounds.snapx(crossings[crossing_index]);
            const double end_x = bounds.snapx(crossings[crossing_index+1]);
            for( double x = start_x; x < end_x; x += precision){
                impl.search({x,y}) = fill_value;
            }
        }
    }
}

template<typename T>
const Bounds& Terrain<T>::get_bounds() const {
    return impl.get_bounds();
}

template<typename T>
size_t Terrain<T>::get_dimension() const {
    return impl.get_dimension();
}

template<typename T>
const string& Terrain<T>::get_error() const {
    return error_message;
}

template<typename T>
inline double Terrain<T>::get_load_factor() const {
    return impl.get_load_factor();
}

template<typename T>
double Terrain<T>::get_precision() const {
    return impl.get_precision();
}

template<typename T>
size_t Terrain<T>::get_size() const {
    return impl.size();
}

template<typename T>
bool Terrain<T>::json(std::ostream& sink){
    // explicitly create the json object
    nlohmann::json doc = nlohmann::json::object();
    
    doc[bounds_key] = impl.get_bounds().to_json();

    doc[precision_key] = impl.get_precision();

    doc[grid_key] = to_json_grid();

    sink << doc.dump(4) << endl;

    return true;
}

template<typename T>
bool Terrain<T>::load(std::istream& source){
    nlohmann::json doc = nlohmann::json::parse( source,  // source document
                                                nullptr,   // callback argument
                                                false);    // allow exceptions?

    if(doc.is_discarded()){
        error_message = "malformed json! ignore.\n";
	source.seekg(0);
	// cerr << source.rdbuf() << endl;
        return false;
    }else if(!doc.is_object()){
        error_message = "input should be a json _document_!!\n" + doc.dump(4) + '\n';
        return false; 
    }
    
    if(!doc.contains(bounds_key)){
        error_message = "Expected '" + bounds_key + "' field in json input document!\n";
        return false;
    }else if(doc.contains(precision_key) && !doc[precision_key].is_number()){
	error_message = "If document contains a precision value, it should be _numeric_!!\n";
        return false;
    }

    // data fields
    if( doc.contains(grid_key) ){
        const Bounds new_bounds(doc[bounds_key]);
        const double new_precision = new_bounds.width() / doc[grid_key].size();
        impl.reset(new_bounds, new_precision);

        return load_grid(doc[grid_key]);

    }else if( doc.contains(tree_key)){
        error_message = "!! Tree loading not implemented!\n";
        return false;
        
    }else if(doc.contains(allow_key)){
        if( ! doc.contains(precision_key) ){
            error_message = "Polygon data requires a precision key !!\n";
            return false;
        }

        const Bounds new_bounds(doc[bounds_key]);
        const double new_precision = doc[precision_key].get<double>();
        impl.reset(new_bounds, new_precision);

        return load_polygons(doc[allow_key], doc[block_key]);
    }

    // #ifdef DEBUG
    // cerr << "!! Did not detect a data structure in this JSON document!:\n";
    // cerr << doc.dump(4) << endl;
    // #endif
    return false;
}

template<typename T>
bool Terrain<T>::load_grid(nlohmann::json grid ){
    const Bounds& bounds = impl.get_bounds();
    const double precision = impl.get_precision();

    if(!grid.is_array() && !grid[0].is_array()){
        cerr << "Terrain::load_grid expected a array-of-arrays! aborting!\n";
        return false;
    }

    if( grid.size() != impl.get_dimension() ){
        cerr << "Terrain::load_grid expected a array of the same dimension as configured!!\n";
        return false;
    }


    // populate the tree
    int row_index = impl.get_dimension() - 1;
    for(auto& row : grid){
        double y = bounds.get_y_min() + (row_index + 0.5)* precision;

        int column_index = 0;
        // i.e. a cell is the element at [column_index, row_index] <=> [x,y]
        for(auto& cell : row){
            double x = bounds.get_x_min() + (column_index + 0.5) * precision;

            impl.search({x,y}) = cell.get<int>();

            ++column_index;
        }
        --row_index;
    }
        
    impl.prune();
    return true;
}

template<typename T>
bool Terrain<T>::load_polygons(nlohmann::json allow_doc, nlohmann::json block_doc){
    auto allowed_polygons = make_polygons(allow_doc);
    auto blocked_polygons = make_polygons(block_doc);

    const cell_value_t allow_value = 0;
    const cell_value_t block_value = 0x99;

    impl.fill(block_value);

    for( auto& poly : allowed_polygons ){
        fill(poly, allow_value);
    }

    for( auto& poly : blocked_polygons ){
        fill(poly, block_value);
    }

    impl.prune();

    return true;
}

template<typename T>
std::vector<Polygon> Terrain<T>::make_polygons(nlohmann::json doc){
    std::vector<Polygon> result(static_cast<size_t>(doc.size()));
    if(0 < result.size()){
        size_t polygon_index = 0;
        for( auto& poly_doc : doc){
            result[polygon_index] = {poly_doc};
        }
    }
    return result;
}

template<typename T>
bool Terrain<T>::png(const string& filename){
#ifdef ENABLE_LIBPNG
    FILE* dest = fopen(filename.c_str(), "wb");
    if(nullptr == dest){
        cerr << "could not open destination .png file ("<<filename<<") for reading." << endl;
        return false;
    }
    return png(dest);
#else
    cerr << "libpng is disabled!! Could not save."
    return false;
#endif //#ifdef ENABLE_LIBPNG
}

template<typename T>
bool Terrain<T>::png(FILE* dest){
#ifdef ENABLE_LIBPNG
    // from: < http://www.libpng.org/pub/png/libpng-manual.txt >
    // from: < https://dev.w3.org/Amaya/libpng/example.c >
    // (search for 'write_png' function near the end)
    if(nullptr == dest){
        return false;
    }

    png_structp png_ptr;
    png_infop  info_ptr;
    //png_colorp palette;

    // allocate and initialize libpng structures:
    {
        // the nulls at the end indicate we are using 'standard' error handling:
        //     i.e. 'setjmp' and 'png_jmpbuf'
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
        if (!png_ptr){
            cerr << "[Tree::to_png] png_create_write_struct failed" << endl;
            fclose(dest);
            return false;
        }
        
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr){
            cerr << "[Tree::to_png] png_create_info_struct failed" << endl;
            fclose(dest);
            png_destroy_write_struct(&png_ptr,  NULL);
            return false;
        }

        // associated this structure with our file pointer:
        png_init_io(png_ptr, dest);
            if (setjmp(png_jmpbuf(png_ptr))){
            cerr << "[Tree::to_png] Error during init_io" << endl;
            fclose(dest);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            return false;
        }
    }


    // write header:
    // Output is 8-bit depth, grayscale, alpha-less format.
    const png_uint_32 image_width = impl.get_dimension();
                               // = static_cast<size_t>(get_bounds().width() / precision);
    png_set_IHDR(png_ptr, info_ptr,
                 image_width, image_width,
                 8, PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[Grid:to_png] Error while writing header info" << endl;
        return false;
    }

    // allocate data buffers
    std::vector<png_byte> buffer( image_width * image_width );
    std::vector<png_bytep> row_pointers( image_width );

    const Bounds& bounds = impl.get_bounds();
    const double pixel_increment = static_cast<double>(bounds.width() / image_width);
    for( int yj = image_width-1; 0 <= yj ; --yj){
        double y = bounds.get_y_min() + (yj + 0.5)* pixel_increment;
        for( int xi = 0; xi < image_width; ++xi){
            double x = bounds.get_x_min() + (xi + 0.5)* pixel_increment;
            buffer[yj*image_width + xi] = impl.search({x,y});
        }
        row_pointers[image_width - 1 - yj] = &(buffer[yj*image_width]);
    }

    // write out the entire image data in one call
    png_write_image(png_ptr, row_pointers.data());

    // error during write
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[Tree::to_png] Error during end of write" << endl;
        fclose(dest);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }
    
    png_write_end(png_ptr, info_ptr);

    // clean up libpng structs
    if (png_ptr && info_ptr){
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    fclose(dest); // close file
    return true;
#else
    cerr << "libpng is disabled!! Could not save."
    return false;
#endif //#ifdef ENABLE_LIBPNG
}

template<typename T>
cell_value_t Terrain<T>::search(const Point& p) const {
    return impl.search(p);
}

template<typename T>
nlohmann::json Terrain<T>::to_json_grid() const {
    const size_t dim = impl.get_dimension();
    const double precision = impl.get_precision();
    const double prec_2 = precision /2;
    const Bounds& bounds = impl.get_bounds();

    // explicitly create the json array
    nlohmann::json grid;

    for(size_t yi=0; yi < dim; ++yi){
        const double y = ((dim-yi-1)*precision + prec_2 + bounds.center.y - bounds.half_width);
        // cerr << "    @[" << yi << "] => (" << y << ")" << endl;
        if(grid[yi].is_null()){
            grid[yi] = nlohmann::json::array();
        }
    
        for(size_t xi=0; xi < dim; ++xi){
            const double x = (xi * precision + prec_2 + bounds.center.x - bounds.half_width);
            grid[yi][xi] = impl.search({x,y});
            // cerr << "        @[" << xi << ", " << yi << "] => (" << x << ", " << y << ") => " << static_cast<int>(value) << endl;
        }
    }

    return grid;
}
