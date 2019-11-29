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
#include "terrain.hpp"

using Eigen::Vector2d;

using terrain::geometry::Layout;
using terrain::geometry::Polygon;


template<typename terrain_t>
bool terrain::io::load_from_json_stream(terrain_t& t, std::istream& source){
    nlohmann::json doc = nlohmann::json::parse( source,  // source document
                                                nullptr,   // callback argument
                                                false);    // allow exceptions?

    if(doc.is_discarded()){
        t.error_message = "malformed json! ignore.\n";
        source.seekg(0);
        // cerr << source.rdbuf() << endl;
        return false;
    }else if(!doc.is_object()){
        t.error_message = "input should be a json _document_!!\n" + doc.dump(4) + '\n';
        return false; 
    }

    if(!doc.contains(layout_key)){
        t.error_message = "Expected '" + layout_key + "' field in json input document!\n";
        return false;
    }
    std::unique_ptr<Layout> new_layout = Layout::make_from_json(doc[layout_key]);
    if( ! new_layout ){
        t.error_message = "Failed to create a grid layout from the given json document!?\n";
        return false;
    }

    // data fields
    if( doc.contains(grid_key) ){
        t.impl.reset(*new_layout);
        return load_grid_from_json(t, doc[grid_key]);

    }else if( doc.contains(tree_key)){
        t.error_message = "!! Tree loading not implemented!\n";
        return false;
        
    }else if(doc.contains(allow_key)){
        t.impl.reset(*new_layout);
        return load_areas_from_json(t, doc[allow_key], doc[block_key]);
    }

    // #ifdef DEBUG
    // cerr << "!! Did not detect a data structure in this JSON document!:\n";
    // cerr << doc.dump(4) << endl;
    // #endif

    return false;
}

template<typename terrain_t>
bool terrain::io::load_grid_from_json(terrain_t& t, nlohmann::json grid ){
    const Layout& layout = t.get_layout();

    if(!grid.is_array() && !grid[0].is_array()){
        t.error_message = "terrain::io::load_grid expected a array-of-arrays! aborting!\n";
        return false;
    }

    if( grid.size() != layout.get_dimension() ){
        t.error_message = "terrain::io::load_grid expected a array of the same dimension as configured!!\n";
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

            t.impl.store({x,y}, cell.get<int>());

            ++column_index;
        }
        --row_index;
    }

    t.impl.prune();

    return true;
}

template<typename terrain_t>
bool terrain::io::load_areas_from_json(terrain_t& t, nlohmann::json allow_doc, nlohmann::json block_doc){
    auto allowed_polygons = make_polygons_from_json(allow_doc);
    auto blocked_polygons = make_polygons_from_json(block_doc);

    const cell_value_t allow_value = 0;
    const cell_value_t block_value = 0x99;

    t.fill(block_value);

    for( auto& poly : allowed_polygons ){
        t.fill(poly, allow_value);
    }

    for( auto& poly : blocked_polygons ){
        t.fill(poly, block_value);
    }

    t.impl.prune();

    return true;
}

inline std::vector<Polygon> terrain::io::make_polygons_from_json( nlohmann::json doc){
    std::vector<Polygon> result(static_cast<size_t>(doc.size()));
    if(0 < result.size()){
        for( size_t polygon_index = 0; polygon_index < doc.size(); ++polygon_index ){
            auto& poly_doc = doc[polygon_index];
            result[polygon_index] = {Polygon(poly_doc)};
        }
    }
    return result;
}

template<typename terrain_t>
bool terrain::io::load_shp_from_file(terrain_t& t, const string& filepath){
#ifdef ENABLE_GDAL
    
    // // create the source dataset (load into this source dataset)
    // GDALDriver* p_memory_driver = GetGDALDriverManager()->GetDriverByName("MEM");
    // GDALDataset *p_grid_dataset = p_memory_driver->Create("", image_width, image_width, 1, GDT_Byte, nullptr);
    // if( nullptr == p_grid_dataset ){
    //     cerr << "!! error allocating grid dataset ?!" << endl;
    //     return false;
    // }

    // auto p_gray_band = p_grid_dataset->GetRasterBand(1);
    // // unfortunately, this is not automatic
    // if( CE_Failure == p_gray_band->SetColorInterpretation(GCI_GrayIndex)){
    //     cerr << "?? Unsupported color format? --- probably a program error." << endl;
    //     GDALClose( p_grid_dataset );
    //     return false;
    // }

    // // allocate data buffers
    // const double sample_increment = layout.get_precision();
    // std::vector<cell_value_t> buffer( image_width * image_width );

    // int i = image_width-1;
    // for( double y = sample_increment/2; y < layout.get_dimension(); y += sample_increment, --i ){
    //     int j = 0;
    //     for( double x = sample_increment/2; x < layout.get_dimension(); x += sample_increment, ++j ){
    //         buffer[i*image_width + j] = impl.classify({x,y});
    //     }
    // }

    // if( CE_Failure == p_gray_band->RasterIO(GF_Write, 0, 0, image_width, image_width, buffer.data(), image_width, image_width, GDT_Byte, 0, 0)){
    //     GDALClose( p_grid_dataset );
    //     return false;
    // }

    // // Use the png driver to copy the source dataset
    // GDALDriver* p_png_driver = GetGDALDriverManager()->GetDriverByName("PNG");
    // GDALDataset *p_png_dataset = p_png_driver->CreateCopy(filepath.c_str(), p_grid_dataset, FALSE, nullptr, nullptr, nullptr);
    // if( nullptr != p_png_dataset ){
    //     GDALClose( p_png_dataset );
    // }
    // GDALClose( p_grid_dataset );

    // return true;
    
    cerr << "loading .shp files is not yet implemented! Could not load file: " << filepath << endl;
    return false;

#else
    cerr << "libpng is disabled!! Could not save.\n";
    return false;

#endif //#ifdef ENABLE_LIBPNG
}

