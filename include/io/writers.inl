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


template<typename terrain_t>
bool terrain::io::to_json(const terrain_t& t, std::ostream& sink){
    // explicitly create the json object
    nlohmann::json doc = nlohmann::json::object();

    doc[layout_key] = t.get_layout().to_json();
    if( doc[layout_key].is_discarded() ){
        return false;
    }

    bool grid_success = to_json_grid(t, doc[grid_key]);
    if( ! grid_success ){
        return false;
    }

    sink << doc.dump() << endl;
    return true;
}

template<typename terrain_t>
bool terrain::io::to_json_grid(const terrain_t& t, nlohmann::json& grid ) {
    const Layout& layout = t.get_layout();

    const auto center = layout.get_center();
    const size_t dim = layout.get_dimension();
    const double width_2 = layout.get_half_width();
    const double precision = layout.get_precision();
    const double prec_2 = precision/2;

    for(size_t yi=0; yi < dim; ++yi){
        const double y = ((dim-yi-1)*precision + prec_2 + center.y() - width_2);
        // cerr << "    @[" << yi << "] => (" << y << ")" << endl;
        if(grid[yi].is_null()){
            grid[yi] = nlohmann::json::array();
        }
    
        for(size_t xi=0; xi < dim; ++xi){
            const double x = (xi * precision + prec_2 + center.x() - width_2);
            grid[yi][xi] = t.classify({x,y});
            // cerr << "        @[" << xi << ", " << yi << "] => (" << x << ", " << y << ") => " << static_cast<int>(value) << endl;
        }
    }

    return true;
}

template<typename terrain_t>
bool terrain::io::to_png(const terrain_t& t, const string& filepath){
#ifdef ENABLE_GDAL
    const Layout& layout = t.get_layout();
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
    const double precision = layout.get_precision();
    std::vector<cell_value_t> buffer( image_width * image_width );

    // // Loop through the rows of the image.
    int i = image_width-1;
    for( double y = layout.get_y_min() + precision/2; y < layout.get_y_max(); y += precision, --i ){
        int j = 0;
        for( double x = layout.get_x_min() + precision/2; x < layout.get_x_max(); x += precision, ++j ){
            buffer[i*image_width + j] = t.classify({x,y});
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

