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
#include <gnm.h>
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
        t.reset(*new_layout);
        return load_grid_from_json(t, doc[grid_key]);

    }else if( doc.contains(tree_key)){
        t.error_message = "!! Tree loading not implemented!\n";
        return false;

    }else if(doc.contains(allow_key)){
        t.reset(*new_layout);

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
            result[polygon_index] = Polygon(poly_doc);
        }
    }
    return result;
}

#ifdef ENABLE_GDAL
inline Polygon terrain::io::make_polygons_from_OGRLine( const OGRLinearRing& source ){
    const size_t point_count = static_cast<size_t>(source.getNumPoints());
    
    if(0 < point_count ){
        Polygon result(point_count);
        
        OGRPoint scratch;
        for( size_t point_index=0; point_index < point_count; ++point_index ){
            source.getPoint(point_index, &scratch);
            result[point_index] = {scratch.getX(), scratch.getY()};
        }

        result.complete();

        return result;
    }

    return {};
}
#endif // #ifdef ENABLE_GDAL

template<typename terrain_t>
bool terrain::io::load_shape_from_file(terrain_t& t, const string& filepath){
#ifdef ENABLE_GDAL
    auto * source_dataset = (GDALDataset*) GDALOpenEx( filepath.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( source_dataset == NULL ){
        cerr << "!> Open failed. " << filepath << " into memory...\n";
        return false;
    }
    // // DEBUG
    // cerr << "#> Loaded file: " << filepath << " into memory...\n";
    // // DEBUG
    // for( auto * each_layer: source_dataset->GetLayers() ){
    //     GNMGenericLayer * each_gnm_layer = (GNMGenericLayer*)each_layer;
    //     cerr << "___?> Layer.Name: " << each_gnm_layer->GetName() << endl;
    // }

    OGRLayer* shape_layer = source_dataset->GetLayer( 0 );
    // ... ->GetLayerByName( "navigation_area_100k" );
    if( nullptr == shape_layer ){
        fprintf( stderr, "    !! dataset doesn't contain any layer? ");
        goto CLEANUP_LOAD_SHAPEFILE;
    }

    shape_layer->ResetReading();
    OGRFeature *poly_feature;
    while( (poly_feature = shape_layer->GetNextFeature()) != NULL ){
        
        OGRGeometry * geom = poly_feature->GetGeometryRef();
        if( geom == NULL ){
            fprintf( stderr, "    !! layer -> feature: could not load geometry! \n");
            goto CLEANUP_LOAD_SHAPEFILE;
        }

        if( wkbPolygon != wkbFlatten(geom->getGeometryType()) ){
            fprintf( stderr, "    <! geometry is not a polygon! aborting!\n" );
            goto CLEANUP_LOAD_SHAPEFILE;
        }

        // Reference: OGRPolygon
        // https://gdal.org/api/ogrgeometry_cpp.html#ogrpolygon-class
        OGRPolygon* poly = geom->toPolygon();
        const Polygon& exterior = make_polygons_from_OGRLine( * poly->getExteriorRing());

        t.reset( exterior.make_layout(16.) );
        // cerr << "    .... loaded layout"  << t.get_layout().to_string() << endl;

        t.fill( block_value );

        //fprintf( stderr, "    .... loading exterior ring with %zu points.\n", exterior.size() );
        t.fill( exterior, allow_value );

        // if we have any interior rings, load them:
        if( 0 < poly->getNumInteriorRings()){
            if( wkbLineString != poly->getInteriorRing(0)->getGeometryType() ){
                fprintf( stderr, "    !! polygon does not contain wkbLineString/LINEARRING geometries!\n");
                fprintf( stderr, "        (instead found : %s (%d) )\n", poly->getInteriorRing(0)->getGeometryName(), poly->getInteriorRing(0)->getGeometryType() );
                goto CLEANUP_LOAD_SHAPEFILE;
            }

            // fprintf( stderr, "       .... loading %d interior rings: \n", poly->getNumInteriorRings() );
            bool skip = true;
            for( const OGRLinearRing* ring : poly ){
                if(skip){
                    // the zeroth entry is the exterior ring, which is treated specially
                    skip = false;
                    continue;
                }
                const Polygon& block_poly = make_polygons_from_OGRLine( *ring );
                t.fill(block_poly, block_value);

                // fputc('.', stderr);
            }
            // fputc( '\n', stderr);
        }

        OGRFeature::DestroyFeature(poly_feature);
    }
    t.impl.prune();

    CLEANUP_LOAD_SHAPEFILE:
        GDALClose( source_dataset );
        return true;

#else //#ifdef ENABLE_GDAL
    cerr << "GDAL functionality is disabled!! Could not save.\n";
    return false;

#endif //#ifdef ENABLE_GDAL
}

