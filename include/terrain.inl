// The MIT License 
// (c) 2019 Daniel Williams

// NOTE: This is the template-class implementation -- 
//       It is not compiled until referenced, even though it contains the function implementations.

#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::string;

#include <nlohmann/json/json.hpp>
using nlohmann::json;

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

// used for debug output:
const static inline string indent("    ");

template<typename T>
Terrain<T>::Terrain(T& _ref): 
    impl(_ref) 
{}

template<typename T>
void Terrain<T>::debug() const {
    impl.debug();
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
double Terrain<T>::get_precision() const {
    return impl.get_precision();
}

template<typename T>
bool Terrain<T>::load(std::istream& source){
    json doc = json::parse( source,  // source document
                            nullptr,   // callback argument
                            false);    // allow exceptions?
    if(doc.is_discarded()){
        // cerr << "malformed json! ignore.\n";
        return false;
    }else if(!doc.is_object()){
        cerr << "input should be a json _document_!!\n";
        cerr << doc.dump(4) << endl;
        return false; 
    }
    
    if(!doc.contains(bounds_key)){
        // cerr << "Expected '" << bounds_key << "' field in json input document!\n";
        return false;
    }else if(doc.contains(precision_key) && !doc[precision_key].is_number()){
        cerr << "If document contains a precision value, it should be _numeric_!!\n";
        return false;
    }

    // data fields
    if( doc.contains(grid_key) ){
        const Bounds new_bounds(doc[bounds_key]);
        const double new_precision = new_bounds.width() / doc[grid_key].size();
        impl.reset(new_bounds, new_precision);

        return impl.load_grid(doc[grid_key]);

    }else if( doc.contains(tree_key)){
        cerr << "!! Tree loading not implemented!" << endl;
        return false;
        
    }else if(doc.contains(allow_key)){
        if( ! doc.contains(precision_key) ){
            cerr << "Polygon data requires a precision key !!\n";
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
bool Terrain<T>::load_polygons(json allow_doc, json block_doc){
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

    return true;
}

template<typename T>
std::vector<Polygon> Terrain<T>::make_polygons(json doc){
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
bool Terrain<T>::write(std::ostream& sink){
    // explicitly create the json object
    json doc = json::object();
    
    doc[bounds_key] = impl.get_bounds().to_json();

    doc[precision_key] = impl.get_precision();

    doc[grid_key] = to_json_grid();

    sink << doc.dump(4) << endl;

    return true;
}

template<typename T>
json Terrain<T>::to_json_grid() const {
    const size_t dim = impl.get_dimension();
    const double precision = impl.get_precision();
    const double prec_2 = precision /2;
    const Bounds& bounds = impl.get_bounds();

    // explicitly create the json array
    json grid;

    for(size_t yi=0; yi < dim; ++yi){
        const double y = ((dim-yi-1)*precision + prec_2 + bounds.center.y - bounds.half_width);
        // cerr << "    @[" << yi << "] => (" << y << ")" << endl;
        if(grid[yi].is_null()){
            grid[yi] = json::array();
        }
    
        for(size_t xi=0; xi < dim; ++xi){
            const double x = (xi * precision + prec_2 + bounds.center.x - bounds.half_width);
            grid[yi][xi] = impl.search({x,y});
            // cerr << "        @[" << xi << ", " << yi << "] => (" << x << ", " << y << ") => " << static_cast<int>(value) << endl;
        }
    }

    return grid;
}
