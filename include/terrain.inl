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
    cerr << "               ";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "%5.1f ", x);
    } cerr << endl;
    // print top border
    cerr << "           +";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "------");
    } cerr << "---+\n";

    for(double y = (layout.get_y_max() - precision/2); y > layout.get_y_min(); y -= precision ){
        // print left header:
        fprintf(stderr, "     %5.1f | ", y);
        for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
            auto value = impl.classify({x,y});
            if( 0 < value ){
                fprintf(stderr, "   %2X,", static_cast<int>(value) );
            }else{
                cerr << "     ,";
            }
        }
        // print right header:
        fprintf(stderr, "  | %5.1f\n", y);
    }
    // print bottom border
    cerr << "           +";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "------");
    } cerr << "---+\n";
    // print footer: (x-axis-labels: 
    cerr << "               ";
    for(double x = (layout.get_x_min() + precision/2); x < layout.get_x_max(); x += precision){
        fprintf(stderr, "%5.1f ", x);
    } cerr << endl << endl;
}

template<typename T>
void inline Terrain<T>::fill(const Polygon& poly, const cell_value_t fill_value){
    // adapted from:
    //  Public-domain code by Darel Rex Finley, 2007:  "Efficient Polygon Fill Algorithm With C Code Sample"
    //  Retrieved: (https://alienryderflex.com/polygon_fill/); 2019-09-07

    const Layout& layout = impl.get_layout();
    const double precision = layout.get_precision();
    const double width = layout.get_width();
    
    // Loop through the rows of the image.
    for( double y = precision/2; y < width; y += precision ){
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
            const double start_x = layout.constrain_x(crossings[crossing_index]);
            const double end_x = layout.constrain_x(crossings[crossing_index+1]);
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
bool Terrain<T>::to_json(std::ostream& sink){
    // explicitly create the json object
    nlohmann::json doc = nlohmann::json::object();
    
    doc[layout_key] = impl.get_layout().to_json();

    doc[grid_key] = to_json_grid();

    sink << doc.dump() << endl;

    return true;
}

template<typename T>
bool Terrain<T>::load_from_json_stream(std::istream& source){
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

    if(!doc.contains(layout_key)){
        error_message = "Expected '" + layout_key + "' field in json input document!\n";
        return false;
    }
    std::unique_ptr<Layout> new_layout = Layout::make_from_json(doc[layout_key]);
    if( ! new_layout ){
        error_message = "Failed to create a grid layout from the given json document!?\n";
	return false;
    }

    // data fields
    if( doc.contains(grid_key) ){
        impl.reset(*new_layout);
        return load_grid_from_json(doc[grid_key]);

    }else if( doc.contains(tree_key)){
        error_message = "!! Tree loading not implemented!\n";
        return false;
        
    }else if(doc.contains(allow_key)){
        impl.reset(*new_layout);
        return load_areas_from_json(doc[allow_key], doc[block_key]);
    }

    // #ifdef DEBUG
    // cerr << "!! Did not detect a data structure in this JSON document!:\n";
    // cerr << doc.dump(4) << endl;
    // #endif

    return false;
}

template<typename T>
bool Terrain<T>::load_grid_from_json(nlohmann::json grid ){
    const Layout& layout = impl.get_layout();

    if(!grid.is_array() && !grid[0].is_array()){
        cerr << "Terrain::load_grid expected a array-of-arrays! aborting!\n";
        return false;
    }

    if( grid.size() != layout.get_dimension() ){
        cerr << "Terrain::load_grid expected a array of the same dimension as configured!!\n";
        return false;
    }

    // populate the tree
    int row_index = layout.get_dimension() - 1;
    for(auto& row : grid){
        double y = layout.get_y_min() + (row_index + 0.5)* layout.get_precision();

        int column_index = 0;
        // i.e. a cell is the element at [column_index, row_index] <=> [x,y]
        for(auto& cell : row){
            double x = layout.get_x_min() + (column_index + 0.5) * layout.get_precision();

            impl.store({x,y}, cell.get<int>());

            ++column_index;
        }
        --row_index;
    }

    impl.prune();

    return true;
}

template<typename T>
bool Terrain<T>::load_areas_from_json(nlohmann::json allow_doc, nlohmann::json block_doc){
    auto allowed_polygons = make_polygons_from_json(allow_doc);
    auto blocked_polygons = make_polygons_from_json(block_doc);

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
std::vector<Polygon> Terrain<T>::make_polygons_from_json(nlohmann::json doc){
    std::vector<Polygon> result(static_cast<size_t>(doc.size()));
    if(0 < result.size()){
        size_t polygon_index = 0;
        for( auto& poly_doc : doc){
            result[polygon_index] = {Polygon(poly_doc)};
        }
    }
    return result;
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

template<typename T>
nlohmann::json Terrain<T>::to_json_grid() const {
    const Layout& layout = impl.get_layout();

    const auto center = layout.get_center();
    const size_t dim = layout.get_dimension();
    const double width_2 = layout.get_half_width();
    const double precision = layout.get_precision();
    const double prec_2 = precision/2;

    // explicitly create the json array
    nlohmann::json grid;

    for(size_t yi=0; yi < dim; ++yi){
        const double y = ((dim-yi-1)*precision + prec_2 + center.y() - width_2);
        // cerr << "    @[" << yi << "] => (" << y << ")" << endl;
        if(grid[yi].is_null()){
            grid[yi] = nlohmann::json::array();
        }
    
        for(size_t xi=0; xi < dim; ++xi){
            const double x = (xi * precision + prec_2 + center.x() - width_2);
            grid[yi][xi] = impl.classify({x,y});
            // cerr << "        @[" << xi << ", " << yi << "] => (" << x << ", " << y << ") => " << static_cast<int>(value) << endl;
        }
    }

    return grid;
}

template<typename T>
bool Terrain<T>::to_png(FILE* dest){
    cerr << "libpng is disabled!! Could not save.\n";
    return false;
}

template<typename T>
bool Terrain<T>::to_png(const string& filepath){
#ifdef ENABLE_GDAL
    const Layout& layout = impl.get_layout();
    const size_t image_width = layout.get_dimension();

    // create the source dataset (load into this source dataset)
    GDALDriver* p_memory_driver = GetGDALDriverManager()->GetDriverByName("MEM");
    GDALDataset *p_grid_dataset = p_memory_driver->Create("", image_width, image_width, 1, GDT_Byte, nullptr);
    if( nullptr == p_grid_dataset ){
        cerr << "!! error allocating grid dataset ?!" << endl;
        return false;
    }

    auto p_gray_band = p_grid_dataset->GetRasterBand(1);
    // unfortunately, this is not automatic
    if( CE_Failure == p_gray_band->SetColorInterpretation(GCI_GrayIndex)){
        cerr << "?? Unsupported color format? --- probably a program error." << endl;
        GDALClose( p_grid_dataset );
        return false;
    }

    // allocate data buffers
    const double sample_increment = layout.get_precision();
    std::vector<cell_value_t> buffer( image_width * image_width );

    int i = image_width-1;
    for( double y = sample_increment/2; y < layout.get_dimension(); y += sample_increment, --i ){
        int j = 0;
        for( double x = sample_increment/2; x < layout.get_dimension(); x += sample_increment, ++j ){
            buffer[i*image_width + j] = impl.classify({x,y});
        }
    }

    if( CE_Failure == p_gray_band->RasterIO(GF_Write, 0, 0, image_width, image_width, buffer.data(), image_width, image_width, GDT_Byte, 0, 0)){
        GDALClose( p_grid_dataset );
        return false;
    }

    // Use the png driver to copy the source dataset
    GDALDriver* p_png_driver = GetGDALDriverManager()->GetDriverByName("PNG");
    GDALDataset *p_png_dataset = p_png_driver->CreateCopy(filepath.c_str(), p_grid_dataset, FALSE, nullptr, nullptr, nullptr);
    if( nullptr != p_png_dataset ){
        GDALClose( p_png_dataset );
    }
    GDALClose( p_grid_dataset );

    return true;

#else
    cerr << "libpng is disabled!! Could not save.\n";
    return false;

#endif //#ifdef ENABLE_LIBPNG
}

