// The MIT License 
// (c) 2019 Daniel Williams

#include <cmath>
#include <string>
using std::string;

#include <memory>
using std::unique_ptr;
using std::make_unique;

#include <iostream>
using std::cerr;
using std::endl;

#include <nlohmann/json/json.hpp>

#ifdef ENABLE_LIBPNG
#include <png.h>
#include <zlib.h>
#endif

#include "quadtree/tree.hpp"

using geometry::Bounds;
using geometry::Point;

using quadtree::QuadTree;
using quadtree::Node;

// used for reading and write json documents:
const static string bounds_key("bounds");
const static string tree_key("tree");
const static string grid_key("grid");

QuadTree::QuadTree(): QuadTree(Bounds({0,0}, 1024), 1.) {}

QuadTree::QuadTree(Bounds _bounds, double _precision):
    root(new Node(_bounds, 0)),
    precision(_precision)
{
    root->split();
}

QuadTree::~QuadTree(){
    this->reset();
}

bool QuadTree::contains(const double x, const double y) const {
    const auto& bounds = get_bounds();

    const double cx = bounds.center.x;
    const double cy = bounds.center.y;
    const double dim = bounds.half_width;

    if((x < (cx - dim)) || ((cx + dim) < x)){
        return false;
    }else if((y < (cy - dim)) || ((cy + dim) < y)){
        return false;
    }

    return true;
}

const Bounds& QuadTree::get_bounds() const {
    return root->get_bounds();
}

node_value_t QuadTree::get_default_value(const Point& at) const {
    return 99;
}

void QuadTree::grow(const double _precision){
    precision = _precision;
    return root->split(_precision);
}


node_value_t QuadTree::interp(const Point& at) const {
    const Node& near = root->search(at);

    // cout << "@@" << at << "    near: " << near.get_bounds() << " = " << near.get_value() << endl;

    // if point is outside the tree, entirely
    if( ! root->contains(at)){
        return get_default_value(at);
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

size_t QuadTree::height() const {
    return root->get_height();
}

void QuadTree::load(std::istream& source){
    nlohmann::json doc = nlohmann::json::parse( source,    // input
                                                nullptr,  // callback argument
                                                false);   // do not allow exceptions

    if(doc.is_discarded()){
        // in case of ANY error, discard this input
        return;
    }

    if(!(doc.is_object() && doc.contains("bounds"))){
        // 'bounds' is necessary for any well-formed tree serialization

        return;
    }else{
        auto& bounds = doc[bounds_key];
        if(!(bounds.contains("x") && bounds.contains("y") && bounds.contains("width"))){
            return;
        }

        // load bounds
        const Point center = {bounds["x"].get<double>(), bounds["y"].get<double>()};
        const double width = bounds["width"].get<double>();
        set({center,width});

        doc.erase(bounds_key);

        // load rest of document
        if(doc.contains(tree_key)){
            // detected treelike data
            root->load(doc[tree_key]);

        }else if(doc.contains(grid_key)){
            // detected gridlike data
            load_grid(doc[grid_key]);
        }
    }
}

void QuadTree::load(const std::vector<Point>& source){
    // pass
}

void QuadTree::load_grid(nlohmann::json& grid ){
    if(grid.is_array() && grid[0].is_array()){

        // pre-allocate the right tree shape & depth for the input grid
        const int dim = grid[0].size();
        const auto& bounds = root->get_bounds();
        const int precision = bounds.width() / dim;
        grow(precision);

        // poplutate the tree
        int yj = dim-1;
        for(auto& row : grid){
            double y = bounds.get_y_min() + (yj + 0.5)* precision;

            int xi = 0;
            for(auto& element : row){
                double x = bounds.get_x_min() + (xi + 0.5) * precision;
                Node& n= root->search({x,y});
                n.set_value(element.get<int>());
                ++xi;
            }
            --yj;
        }
        
        root->cull();
    }
}


std::ostream& operator<<(std::ostream& os, const quadtree::QuadTree& tree)
{
    tree.write_json(os);
    return os;
}

node_value_t QuadTree::search(const double x, const double y) const {
    const Point at(x,y);
    if(root->contains(at)){
        return root->search(at).get_value();
    }else{
        return get_default_value(at);
    }
}

void QuadTree::reset() {
    this->root.reset();
}

void QuadTree::set(const Bounds new_bounds) {
    if( new_bounds != root->get_bounds() ){
        root = std::make_unique<Node>(new_bounds, NAN);
    }
}

// void QuadTree::set(double x, double y, node_value_t new_value) {
//     if( ! contains(x,y)){
//         cerr << "Attempt to set location for a point (" << x << ", " << y << "), not contained by this tree\n";
//     }

//     // reminder: this method does not modify tree _shape_.  Just stores values in existing nodes.

// }

bool QuadTree::write_tree(std::ostream& sink) const {
    auto& bounds = get_bounds(); 
    sink << "====== Tree: ======\n";
    sink << "##  " << bounds << endl;

    root->draw(sink, "", "RT");
    
    sink << endl;
    return true;
}

bool QuadTree::write_json(std::ostream& sink) const {
    nlohmann::json doc;

    // output the bounds once, at the root-level, and all the bounds
    // for subsequent layers follow deterministicly
    const Bounds& bounds = root->get_bounds();
    doc[bounds_key] = {
        {"x", bounds.center.x},
        {"y", bounds.center.y},
        // tree is implemented & enforced as square
        {"width", 2 * bounds.half_width}
    };
    
    doc[tree_key] = root->to_json();

    // write result to output stream:
    sink << doc;
    return true;
}


bool QuadTree::write_png(const std::string filename) const {
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
            cerr << "[QuadTree::to_png] png_create_write_struct failed" << endl;
            fclose(fp);
            return false;
        }
        
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr){
            cerr << "[QuadTree::to_png] png_create_info_struct failed" << endl;
            fclose(fp);
            png_destroy_write_struct(&png_ptr,  NULL);
            return false;
        }

        // associated this structure with our file pointer:
        png_init_io(png_ptr, fp);
            if (setjmp(png_jmpbuf(png_ptr))){
            cerr << "[QuadTree::to_png] Error during init_io" << endl;
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
            buffer[yj*image_width + xi] = search(x,y);
        }
        row_pointers[image_width - 1 - yj] = buffer.get() + yj*image_width*sizeof(png_byte);
    }

    // write out the entire image data in one call
    png_write_image(png_ptr, row_pointers.get());

    // error during write
    if (setjmp(png_jmpbuf(png_ptr))){
        cerr << "[QuadTree::to_png] Error during end of write" << endl;
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
