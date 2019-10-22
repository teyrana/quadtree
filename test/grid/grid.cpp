#include <cmath>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "nlohmann/json/json.hpp"

#include "geometry/layout.hpp"
#include "geometry/polygon.hpp"

#include "grid/grid.hpp"
#include "terrain.hpp"

using std::cerr;
using std::endl;
using std::isnan;
using std::string;

using Eigen::Vector2d;
using nlohmann::json;

using terrain::geometry::cell_default_value;

namespace terrain::grid {

TEST(GridTest, ConstructDefault) {
    Terrain<grid::Grid> terrain;
 
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     1.);
    EXPECT_EQ( terrain.get_layout().get_dimension(), 1);
    EXPECT_EQ( terrain.get_layout().get_size(),      1);
}

//                   +-----+-----+-----+-----+
// Index:            |  0  |  1  |  2  |  3  |
//                   +-----+-----+-----+-----+
//                   1     2     3     4     5
// Coordinate:                           
//
TEST(GridTest, ConstructWithSizeSpacingCenter) {
    grid::Grid g({1., 3.,3., 4.});
    Terrain terrain(g);
 
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         3.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         3.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     4.);
    EXPECT_EQ( terrain.get_layout().get_dimension(), 4);
    EXPECT_EQ( terrain.get_layout().get_size(),     16);
}

TEST(GridTest, ConstructWithUnitSize){
    grid::Grid g({1./16., 0, 0, 1.});
    Terrain terr(g);
 
    EXPECT_DOUBLE_EQ( terr.get_layout().get_precision(), 0.0625);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_width(),     1.);

    EXPECT_EQ( terr.get_layout().get_dimension(), 16);
    EXPECT_EQ( terr.get_layout().get_size(),     256);
}

TEST(GridTest, ConstructWithOddSize) {
    grid::Grid g({7.1, 0,0, 32.});
    Terrain terr(g);
 
    EXPECT_DOUBLE_EQ( terr.get_layout().get_precision(), 4.);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terr.get_layout().get_width(),    32.);

    EXPECT_EQ( terr.get_layout().get_dimension(), 8);
    EXPECT_EQ( terr.get_layout().get_size(),         64);
}

TEST( GridTest, LoadMalformedSource){
    grid::Grid g;
    Terrain terrain(g);

    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     1.);
    
    // this is simply a malformed document.  It should not parse.
    std::istringstream source(R"({"bounds": {"x": 100, "y": 100, "width": )");
    // this should fail. Gracefully.
    EXPECT_FALSE(terrain.load_from_json_stream(source));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     1.);
}

TEST( GridTest, LoadValidBoundsFromJSON){
    grid::Grid g;
    Terrain terrain(g);

    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     1.);

    // construct a valid document, with correct fields, but missing required fields:
    std::istringstream source(R"({"layout": {"precision": 1.0, "x": 100, "y": 100, "width": 64}} )");
    // this should fail. gracefully.
    EXPECT_FALSE(terrain.load_from_json_stream(source));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         0.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     1.);
}

TEST(GridTest, LoadGridFromJSON) {
    grid::Grid g;
    Terrain terrain(g);
    std::istringstream stream(R"(
        {"layout": {"precision": 1.0, "x": 4, "y": 4, "width": 8},
         "grid": [[88, 88, 88, 88, 88, 88, 88, 88],
                  [88, 88, 88,  0,  0, 88, 88, 88],
                  [88, 88,  0,  0,  0,  0, 88, 88],
                  [88,  0,  0,  0,  0,  0,  0, 88],
                  [88, 88, 88, 88,  0,  0,  0, 88],
                  [88, 88, 88, 88,  0,  0, 88, 88],
                  [88, 88, 88, 88,  0, 88, 88, 88],
                  [88, 88, 88, 88, 88, 88, 88, 88]]})");
    
    // Test Target
    ASSERT_TRUE(terrain.load_from_json_stream(stream));
    // Test Target
    
    // // DEBUG
    // terrain.debug();

    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         4.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         4.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),     8.);

    EXPECT_EQ( terrain.get_layout().get_dimension(), 8);
    EXPECT_EQ( terrain.get_layout().get_size(),     64);

    // test by index-space:
    ASSERT_EQ( g.get_cell(0,0), 88);
    ASSERT_EQ( g.get_cell(1,1), 88);
    ASSERT_EQ( g.get_cell(2,2), 88);
    ASSERT_EQ( g.get_cell(3,3), 88);
    ASSERT_EQ( g.get_cell(4,4),  0);
    ASSERT_EQ( g.get_cell(5,5),  0);
    ASSERT_EQ( g.get_cell(6,6), 88);
    ASSERT_EQ( g.get_cell(7,7), 88);

    ASSERT_EQ( g.get_cell(3,0), 88);
    ASSERT_EQ( g.get_cell(3,1), 88);
    ASSERT_EQ( g.get_cell(3,2), 88);
    ASSERT_EQ( g.get_cell(3,3), 88);
    ASSERT_EQ( g.get_cell(3,4),  0);
    ASSERT_EQ( g.get_cell(3,5),  0);
    ASSERT_EQ( g.get_cell(3,6),  0);
    ASSERT_EQ( g.get_cell(3,7), 88);

    ASSERT_EQ( g.get_cell(0,7), 88);
    ASSERT_EQ( g.get_cell(1,6), 88);
    ASSERT_EQ( g.get_cell(2,5),  0);

    // test by query-space
    ASSERT_EQ( g.classify({ 0.5, 0.5}), 88);
    ASSERT_EQ( g.classify({ 1.5, 1.5}), 88);
    ASSERT_EQ( g.classify({ 2.5, 2.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 3.5}), 88);
    ASSERT_EQ( g.classify({ 4.5, 4.5}),  0);
    ASSERT_EQ( g.classify({ 5.5, 5.5}),  0);
    ASSERT_EQ( g.classify({ 6.5, 6.5}), 88);
    ASSERT_EQ( g.classify({ 7.5, 7.5}), 88);
    ASSERT_EQ( g.classify({ 8.5, 8.5}), cell_default_value);
    ASSERT_EQ( g.classify({ 9.5, 9.5}), cell_default_value);

    ASSERT_EQ( g.classify({ 3.5, 0.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 1.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 2.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 3.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 4.5}),  0);
    ASSERT_EQ( g.classify({ 3.5, 5.5}),  0);
    ASSERT_EQ( g.classify({ 3.5, 6.5}),  0);
    ASSERT_EQ( g.classify({ 3.5, 7.5}), 88);
    ASSERT_EQ( g.classify({ 3.5, 8.5}), cell_default_value);

}

TEST(GridTest, LoadPolygonFromJSON) {
    grid::Grid g;
    Terrain terrain(g);

    constexpr double boundary_width = 16.;   // overall boundary
    constexpr double desired_precision = 1.0;
    // =====
    const json source = generate_diamond( boundary_width,
                                          desired_precision);

    std::istringstream stream(source.dump());

    EXPECT_TRUE(terrain.load_from_json_stream(stream));
    // print error, if it is set:
    ASSERT_TRUE( terrain.get_error().empty() ) << terrain.get_error();

    // // DEBUG
    // terrain.debug();

    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         8.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         8.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),    16.);

    EXPECT_EQ( terrain.get_layout().get_dimension(), 16);
    EXPECT_EQ( terrain.get_layout().get_size(),     256);

    ASSERT_EQ( g.get_cell(4,15), 0x99);
    ASSERT_EQ( g.get_cell(4,14), 0x99);
    ASSERT_EQ( g.get_cell(4,13), 0x99);
    ASSERT_EQ( g.get_cell(4,12),    0);
    ASSERT_EQ( g.get_cell(4,11),    0);
    ASSERT_EQ( g.get_cell(4, 9),    0);
    ASSERT_EQ( g.get_cell(4, 8),    0);
    ASSERT_EQ( g.get_cell(4, 7),    0);
    ASSERT_EQ( g.get_cell(4, 6),    0);
    ASSERT_EQ( g.get_cell(4, 5),    0);
    ASSERT_EQ( g.get_cell(4, 4),    0);
    ASSERT_EQ( g.get_cell(4, 3),    0);
    ASSERT_EQ( g.get_cell(4, 2), 0x99);
    ASSERT_EQ( g.get_cell(4, 1), 0x99);
    ASSERT_EQ( g.get_cell(4, 0), 0x99);

    ASSERT_EQ( g.get_cell( 0, 5), 0x99);
    ASSERT_EQ( g.get_cell( 1, 5), 0x99);
    ASSERT_EQ( g.get_cell( 2, 5),    0);
    ASSERT_EQ( g.get_cell( 3, 5),    0);
    ASSERT_EQ( g.get_cell( 4, 5),    0);
    ASSERT_EQ( g.get_cell( 5, 5),    0);
    ASSERT_EQ( g.get_cell( 6, 5),    0);
    ASSERT_EQ( g.get_cell( 7, 5),    0);
    ASSERT_EQ( g.get_cell( 8, 5),    0);
    ASSERT_EQ( g.get_cell( 9, 5),    0);
    ASSERT_EQ( g.get_cell(10, 5),    0);
    ASSERT_EQ( g.get_cell(11, 5),    0);
    ASSERT_EQ( g.get_cell(12, 5),    0);
    ASSERT_EQ( g.get_cell(13, 5), 0x99);
    ASSERT_EQ( g.get_cell(14, 5), 0x99);
    ASSERT_EQ( g.get_cell(15, 5), 0x99);
}

TEST(GridTest, SavePNG) {
    Terrain<Grid> terrain;
    const json source = generate_diamond(  16.,   // boundary_width
                                            1.0);  // desired_precision);

    std::istringstream stream(source.dump());

    ASSERT_TRUE(terrain.load_from_json_stream(stream));

    // // DEBUG
    // terrain.debug();

    EXPECT_DOUBLE_EQ( terrain.get_layout().get_precision(), 1.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_x(),         8.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_y(),         8.);
    EXPECT_DOUBLE_EQ( terrain.get_layout().get_width(),    16.);
    EXPECT_EQ( terrain.get_layout().get_dimension(), 16);
    EXPECT_EQ( terrain.get_layout().get_size(),     256);
    
    // const string filename("grid.test.png");
    // // Because this manually tested, turn off by default.
    // {
    //     FILE* dest = fopen(filename.c_str(), "wb");
    //     if(nullptr == dest){
    //         cerr << "could not open destination .png file ("<<filename<<") for reading." << endl;
    //         return;
    //     }
    //     terrain.png(dest);
    // }
}

}; // namespace terrain::grid
