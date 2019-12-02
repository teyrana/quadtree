// The MIT License 
// (c) 2019 Daniel Williams

// NOTE: This is the template-class implementation -- 
//       It is not compiled until referenced, even though it contains the function implementations.

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::string;

#include <Eigen/Geometry>

#ifdef ENABLE_GDAL
#include "gdal.h"
#include "gdal_priv.h"
#endif

#include <nlohmann/json/json.hpp>

#include "geometry/layout.hpp"
#include "geometry/polygon.hpp"
#include "grid/grid.hpp"
#include "terrain.hpp"

using Eigen::Vector2d;

using terrain::geometry::Layout;
using terrain::geometry::Polygon;
using terrain::Terrain;

// used for reading and write json documents:
const static inline string allow_key("allow");
const static inline string block_key("block");
const static inline string layout_key("layout");
const static inline string grid_key("grid");
const static inline string tree_key("tree");

template<typename T>
Terrain<T>::Terrain(): Terrain(*(new T())) {}

template<typename T>
Terrain<T>::Terrain(T& _ref): 
    impl(_ref) 
{
#ifdef ENABLE_GDAL
    GDALAllRegister();
#endif
}

template<typename T>
Terrain<T>::~Terrain(){
#ifdef ENABLE_GDAL
    GDALDestroyDriverManager();
#endif
}

template<typename T>
cell_value_t Terrain<T>::classify(const Vector2d& p) const {
    return impl.classify(p);
}

template<typename T>
void Terrain<T>::debug() const {
    const Layout& layout = impl.get_layout();
    const double precision = layout.get_precision();
    const size_t dimension = layout.get_dimension();

    cerr << "====== Structure: ======\n";
    cerr << "##  layout:     " << layout.to_json().dump() << endl;
    // cerr << "##  height:     " << get_height() << endl;
    cerr << "##  precision:  " << precision << endl;
    cerr << "##  dimension:  " << dimension << endl;

    cerr << "           ======== ======== ======== ======== As Grid: ======== ======== ======== ========\n";
    // print header (x-axis-labels: 
    cerr << "             ";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "%5.2f ", x);
    } cerr << endl;
    // print top border
    cerr << "           +";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "------");
    } cerr << "---+\n";

    for(double y = (layout.get_y_max() - precision/2); y > layout.get_y_min(); y -= precision ){
        // print left header:
        fprintf(stderr, "     %5.2f | ", y);
        for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
            auto value = impl.classify({x,y});
            if( 0 < value ){
                fprintf(stderr, "   %2X,", static_cast<int>(value) );
            }else{
                cerr << "     ,";
            }
        }
        // print right header:
        fprintf(stderr, "  | %5.2f\n", y);
    }
    // print bottom border
    cerr << "           +";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "------");
    } cerr << "---+\n";
    // print footer: (x-axis-labels: 
    cerr << "             ";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "%5.2f ", x);
    } cerr << endl << endl;
}

template<typename T>
void inline Terrain<T>::fill(const cell_value_t value){
    impl.fill(value);
}

template<typename T>
void inline Terrain<T>::fill(const Polygon& poly, const cell_value_t fill_value){
    // adapted from:
    //  Public-domain code by Darel Rex Finley, 2007:  "Efficient Polygon Fill Algorithm With C Code Sample"
    //  Retrieved: (https://alienryderflex.com/polygon_fill/); 2019-09-07

    const Layout& layout = impl.get_layout();
    const double precision = layout.get_precision();
    const double precision_2 = layout.get_precision()/2;

    // Loop through the rows of the image.
    for( double y = layout.get_y_min() + precision_2; y < layout.get_y_max(); y += precision ){
        // generate a list of line-segment crossings from the polygon
        std::vector<double> crossings;
        for (int i=0; i < poly.size()-1; ++i) {
            const Vector2d& p1 = poly[i];
            const Vector2d& p2 = poly[i+1];

            const double y_max = std::max(p1[1], p2[1]);
            const double y_min = std::min(p1[1], p2[1]);
            // if y is in range:
            if( (y_min <= y) && (y < y_max) ){
                // construct x-coordinate that crosses this line:
                auto value = p1[0] + (y - p1[1]) * (p2[0]-p1[0])/(p2[1] - p1[1]);
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
            const double start_x = layout.constrain_x(crossings[crossing_index] + precision_2);
            const double end_x = layout.constrain_x(crossings[crossing_index+1] + precision_2);
            for( double x = start_x; x < end_x; x += precision){
                impl.store({x,y}, fill_value);
            }
        }
    }
}

template<typename T>
const Layout& Terrain<T>::get_layout() const {
    return impl.get_layout();
}

template<typename T>
const string& Terrain<T>::get_error() const {
    return error_message;
}

template<typename T>
inline double Terrain<T>::get_load_factor() const {
    // TODO: move implementation to this file
    // return impl.get_load_factor();

    return -1.0; // error. fixme!
}

template<typename T>
void inline Terrain<T>::reset(){
    impl.reset();
}

template<typename T>
void inline Terrain<T>::reset(const Layout& _layout){
    impl.reset(_layout);
}

template<typename T>
std::string Terrain<T>::summary() const {
    std::ostringstream buffer;
    buffer.imbue(std::locale(""));
    buffer << "====== Terrain Stats: ======\n";
    buffer << "##  layout:       " << impl.get_layout().to_string() << '\n';
    buffer << "##  dimension:    " << impl.get_layout().get_dimension() << endl;
    buffer << "##  size:         " << impl.get_layout().get_size() <<  " nodes  ===  " << impl.get_memory_usage()/1000 << " kilobytes\n";
    buffer << "##  compression:  " << impl.get_load_factor() << '\n';
    buffer << '\n';
    return buffer.str();
}
