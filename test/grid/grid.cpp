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

const static Bounds& default_bounds = terrain::geometry::Layout::default_layout.bounds;

namespace terrain::grid {

TEST(GridTest, ConstructDefault) {
    grid::Grid g;
    Terrain terr(g);
 
    EXPECT_EQ( g.get_dimension(), 1);
    EXPECT_GE( g.size(),      1);

    EXPECT_DOUBLE_EQ( terr.get_precision(), 1.);

    EXPECT_TRUE( default_bounds == terr.get_bounds());
}

//                   +-----+-----+-----+-----+
// Index:            |  0  |  1  |  2  |  3  |
//                   +-----+-----+-----+-----+
//                   1     2     3     4     5
// Coordinate:                           
//
TEST(GridTest, ConstructWithSizeSpacingCenter) {
    grid::Grid g({{3.,3.}, 4.}, 1);
    Terrain terr(g);
 
    const auto& bounds = terr.get_bounds();
    EXPECT_DOUBLE_EQ( bounds.half_width,  2.);
    EXPECT_DOUBLE_EQ( bounds.center[0],    3.);
    EXPECT_DOUBLE_EQ( bounds.center[1],    3.);

    EXPECT_DOUBLE_EQ( terr.get_precision(), 1.0);

    EXPECT_EQ( g.get_dimension(), 4);
    EXPECT_GE( g.size(),     16);
}

TEST(GridTest, ConstructWithUnitSize){
    grid::Grid g({{0,0}, 1.}, 1./16.);
    Terrain terr(g);
 
    EXPECT_TRUE(Bounds({0,0}, 1.) == terr.get_bounds());

    EXPECT_DOUBLE_EQ( terr.get_precision(), 0.0625);

    EXPECT_EQ( g.get_dimension(), 16);
    EXPECT_GE( g.size(),     256);
}


TEST(GridTest, ConstructWithOddSize) {
    grid::Grid g({{0,0}, 32.}, 7.1);
    Terrain terr(g);
 
    EXPECT_TRUE(Bounds({0,0}, 32) == terr.get_bounds());

    EXPECT_DOUBLE_EQ( terr.get_precision(), 4.0);
    
    EXPECT_EQ( g.get_dimension(), 8);
    EXPECT_GE( g.size(),     64);
}

TEST( GridTest, LoadMalformedSource){
    grid::Grid g;
    Terrain terrain(g);
    ASSERT_TRUE(default_bounds == terrain.get_bounds());

    // this is simply a malformed document.  It should not parse.
    std::istringstream source(R"({"bounds": {"x": 100, "y": 100, "width": )");
    // this should fail. Gracefully.
    EXPECT_FALSE(terrain.load(source));

    // these tests should be *exactly* the same as before the 'load' call
    ASSERT_TRUE(default_bounds == terrain.get_bounds());
}

TEST( GridTest, LoadValidBoundsFromJSON){
    grid::Grid g;
    Terrain terrain(g);
    ASSERT_TRUE(default_bounds == terrain.get_bounds());

    // construct a valid document, with correct fields, but missing required fields:
    std::istringstream source(R"({"bounds": {"x": 100, "y": 100, "width": 64}} )");
    // this should fail. gracefully.
    EXPECT_FALSE(terrain.load(source));

    // these tests should be *exactly* the same as before the 'load' call
    ASSERT_TRUE(default_bounds == terrain.get_bounds());
}

TEST(GridTest, LoadGridFromJSON) {
    grid::Grid g;
    Terrain terrain(g);
    std::istringstream stream(R"(
        {"bounds": {"x": 4, "y": 4, "width": 8},
         "grid": [[88, 88, 88, 88, 88, 88, 88, 88],
                  [88, 88, 88,  0,  0, 88, 88, 88],
                  [88, 88,  0,  0,  0,  0, 88, 88],
                  [88,  0,  0,  0,  0,  0,  0, 88],
                  [88, 88, 88, 88,  0,  0,  0, 88],
                  [88, 88, 88, 88,  0,  0, 88, 88],
                  [88, 88, 88, 88,  0, 88, 88, 88],
                  [88, 88, 88, 88, 88, 88, 88, 88]]})");
    
    // Test Target
    ASSERT_TRUE(terrain.load(stream));
    // Test Target
    
    // // DEBUG
    // terrain.debug();

    EXPECT_EQ( g.get_dimension(), 8);
    EXPECT_EQ( g.size(), 64);
    EXPECT_EQ( g.width(), 8);

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.0);
    ASSERT_TRUE(Bounds({4.,4.}, 8.) == terrain.get_bounds());

    ASSERT_EQ( g.get_cell(0,0), 88);
    ASSERT_EQ( g.get_cell(1,1), 88);
    ASSERT_EQ( g.get_cell(2,2), 88);
    ASSERT_EQ( g.get_cell(2,3), 88);
    ASSERT_EQ( g.get_cell(2,4),  0);

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

    ASSERT_TRUE(terrain.load(stream));

    // // DEBUG
    // terrain.debug();

    EXPECT_EQ( g.get_dimension(), 16);
    EXPECT_EQ( g.size(),         256);

    EXPECT_DOUBLE_EQ( desired_precision, terrain.get_precision());

    ASSERT_TRUE( Bounds({8,8}, 16) == terrain.get_bounds());

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

    ASSERT_TRUE(terrain.load(stream));

    // // DEBUG
    // terrain.debug();

    EXPECT_TRUE(Bounds({8,8}, 16) == terrain.get_bounds());
    EXPECT_EQ( terrain.get_dimension(), 16);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.0);

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
