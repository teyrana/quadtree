// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>

using std::string;
using std::cerr;
using std::endl;

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#ifdef ENABLE_LIBPNG
#include <png.h>
#include <zlib.h>
#endif

#include "quadtree/tree.hpp"

using namespace terrain;
using geometry::Bounds;
using geometry::Point;
using quadtree::Tree;
using quadtree::Node;


static cell_value_t scratch;

Tree::Tree(): 
    Tree(Tree::default_bounds, 4.)
{}

Tree::Tree(const Bounds& _bounds, const double _precision):
    bounds(_bounds), root(new Node(_bounds, 0)), precision(_precision)
{}

Tree::~Tree(){
    root.release();
}

bool Tree::contains(const Point& p) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x;
    const double cy = bounds.center.y;
    const double dim = bounds.half_width;

    if((p.x < (cx - dim)) || ((cx + dim) < p.x)){
        return false;
    }else if((p.y < (cy - dim)) || ((cy + dim) < p.y)){
        return false;
    }

    return true;
}

void Tree::cull(){
    root->cull();
}

void Tree::debug() const {
    const auto& height = get_height();

    cerr << "====== Quad Tree: ======\n";
    cerr << "##  bounds: " << get_bounds().str() << endl;
    cerr << "##  height: " << get_height() << endl;

    if(5 > height){
        cerr << "     ==== As Tree: ====\n";
        root->draw(cerr, "    ", "RT");
        cerr << endl;
    }

    debug_grid(cerr);
}

void Tree::debug_grid(std::ostream& sink) const {
    const static string indent("    ");

    sink << indent << "==== As Grid: ====\n";
    for(double y = (bounds.get_y_max() - precision/2); y > bounds.get_y_min(); y -= precision ){
        sink << indent << ' ';
        for(double x = (bounds.get_y_min() + precision/2); x < bounds.get_y_max(); x += precision){
            auto value = search({x,y});
            if( 0 < value ){
                sink << ' ' << std::setfill(' ') << std::setw(2) << std::hex << static_cast<int>(value) << ',';
            }else{
                sink << "   ,";
            }
        }
        sink << '\n';
    }
    sink << '\n';
}

size_t Tree::dimension() const {
    return static_cast<size_t>(bounds.width() / precision);
}

const Bounds& Tree::get_bounds() const {
    return bounds;
}

size_t Tree::get_dimension() const {
    return static_cast<size_t>( bounds.width() / precision );
}

void Tree::grow(const double _precision){
    precision = _precision;
    return root->split(_precision);
}


cell_value_t Tree::interp(const Point& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if point is outside the tree, entirely
    if( ! contains(at)){
        return cell_default_value;
    }

    // if the point is near-to-center if the nearest node:
    if( near.nearby(at)){
        return near.get_value();
    }

    const Point& cn = near.get_center();
    const double dx = std::copysign(1.0, (at.x - cn.x)) * 2 * near.get_bounds().half_width;
    const double dy = std::copysign(1.0, (at.y - cn.y)) * 2 * near.get_bounds().half_width;
    const Node& n2 = root->search({cn.x + dx, cn.y     });
    const Node& n3 = root->search({cn.x + dx, cn.y + dy});
    const Node& n4 = root->search({cn.x     , cn.y + dy});

    const auto& interp = near.interpolate_bilinear(at, n2, n3, n4);

    return interp;
}

void Tree::fill(const cell_value_t fill_value){
    root->fill(fill_value);
}

size_t Tree::get_height() const {
    return root->get_height();
}

void Tree::load_polygon(const std::vector<Point>& source){
    // pass
}

bool Tree::load_grid(nlohmann::json& grid ){
    if(grid.is_array() && grid[0].is_array()){
        const size_t dim = grid.size();
        const size_t pow_of_2 = pow(2, ceil(log2(dim)));
        if(grid.size() != pow_of_2 ){
            return false;
        }
        
        // pre-allocate the right tree shape & depth for the input grid
        const double new_precision = bounds.width() / dim;
        grow(new_precision);

        // populate the tree
        int row_index = dim-1;
        for(auto& row : grid){
            double y = bounds.get_y_min() + (row_index + 0.5)* precision;

            int column_index = 0;
            // i.e. a cell is the element at [column_index, row_index] <=> [x,y]
            for(auto& cell : row){
                double x = bounds.get_x_min() + (column_index + 0.5) * precision;
                Node& n= root->search({x,y});
                n.set_value(cell.get<int>());
                ++column_index;
            }
            --row_index;
        }
        
        root->cull();
        return true;
    }
    return false;
}

bool Tree::load_tree(nlohmann::json& doc){
    if(! doc.is_object()){
        return false;
    }
    return root->load(doc);
}


cell_value_t& Tree::search(const Point& p) {
    if(contains(p)){
        return root->search(p).get_value();
    }

    scratch = cell_error_value;
    return scratch;
}

cell_value_t Tree::search(const Point& p) const {
    if(contains(p)){
        return root->search(p).get_value();
    }
    return cell_default_value;
}

void Tree::reset(const Bounds new_bounds, const double new_precision){
    if( isnan(new_precision) ){
        root.release();
        return;
    }

    this->bounds = new_bounds;
    this->precision = new_precision;


    root = std::make_unique<Node>(bounds,0);
    root->split(precision);
}

// void Tree::set(double x, double y, cell_value_t new_value) {
//     if( ! contains(x,y)){
//         cerr << "Attempt to set location for a point (" << x << ", " << y << "), not contained by this tree\n";
//     }

//     // reminder: this method does not modify tree _shape_.  Just stores values in existing nodes.

// }


json Tree::to_json_tree() const {
    return root->to_json();
}

bool Tree::write_png(const std::string filename) const {
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

    // allocate and initialize libpng structures:
    {
        // the nulls at the end indicate we are using 'standard' error handling:
        //     i.e. 'setjmp' and 'png_jmpbuf'
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
        if (!png_ptr){
            cerr << "[Tree::to_png] png_create_write_struct failed" << endl;
            fclose(fp);
            return false;
        }
        
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr){
            cerr << "[Tree::to_png] png_create_info_struct failed" << endl;
            fclose(fp);
            png_destroy_write_struct(&png_ptr,  NULL);
            return false;
        }

        // associated this structure with our file pointer:
        png_init_io(png_ptr, fp);
            if (setjmp(png_jmpbuf(png_ptr))){
            cerr << "[Tree::to_png] Error during init_io" << endl;
            fclose(fp);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            return false;
        }
    }

    // write header:
    // Output is 8-bit depth, grayscale, alpha-less format.
    const png_uint_32 image_width = static_cast<size_t>(get_bounds().width() / precision);
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

    unique_ptr<png_byte[]> buffer(new png_byte[image_width*image_width]);
    unique_ptr<png_bytep[]> row_pointers(new png_bytep[image_width]);
    const Bounds& bounds = get_bounds();
    const double increment = static_cast<double>(bounds.width() / image_width);
    for( int yj = image_width-1; 0 <= yj ; --yj){
        double y = bounds.get_y_min() + (yj + 0.5)* increment;
        for( int xi = 0; xi < image_width; ++xi){
            double x = bounds.get_x_min() + (xi + 0.5)* increment;
            buffer[yj*image_width + xi] = search({x,y});
        }
        row_pointers[image_width - 1 - yj] = buffer.get() + yj*image_width*sizeof(png_byte);
    }

    // write out the entire image data in one call
    png_write_image(png_ptr, row_pointers.get());

    // error during write
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[Tree::to_png] Error during end of write" << endl;
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

// size_t Tree::x_to_index(double x) const {
//     if(x < bounds.get_x_min()){
//         return 0;
//     }else if(x > bounds.get_x_max()){
//         return dimension()-1;
//     }

//     return static_cast<size_t>((x - (bounds.center.x - bounds.half_width))/precision);
// }

// size_t Tree::y_to_index(double y) const {
//     if(y < bounds.get_y_min()){
//         return 0;
//     }else if(y > bounds.get_y_max()){
//         return dimension()-1;
//     }

//     return static_cast<size_t>((y - (bounds.center.y - bounds.half_width))/precision);
// }
