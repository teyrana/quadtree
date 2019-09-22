#include <cmath>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "geometry/polygon.hpp"
#include "quadtree/grid.hpp"

using std::cerr;
using std::endl;
using std::isnan;
using std::string;

namespace quadtree {

//                   +-----+-----+-----+-----+
// Index:            |  0  |  1  |  2  |  3  |
//                   +-----+-----+-----+-----+
//                   1     2     3     4     5
// Coordinate:                           
//
TEST(GridTest, ConstructWithSizeSpacingCenter) {
    quadtree::Grid g(4., 1., {3.,3.});

 
    EXPECT_EQ( g.size(), 16);
    EXPECT_EQ( g.width(), 4);

    EXPECT_DOUBLE_EQ( g.spacing, 1.0);
    EXPECT_DOUBLE_EQ( g.bounds.half_width, 2.);
    EXPECT_DOUBLE_EQ( g.bounds.center.x, 3.);
    EXPECT_DOUBLE_EQ( g.bounds.center.y, 3.);

    EXPECT_DOUBLE_EQ( g.bounds.get_x_min(), 1.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_min(), 1.);
    EXPECT_DOUBLE_EQ( g.bounds.get_x_max(), 5.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_max(), 5.);

    EXPECT_DOUBLE_EQ( g.anchor().x, 1.);
    EXPECT_DOUBLE_EQ( g.anchor().y, 1.);
}

TEST(GridTest, XYToIndex) {
    quadtree::Grid g(8., 0.5, {5.,5.});
    
    EXPECT_DOUBLE_EQ( g.spacing, 0.5);
    EXPECT_DOUBLE_EQ( g.bounds.half_width, 4.);
    EXPECT_DOUBLE_EQ( g.bounds.center.x, 5.);
    EXPECT_DOUBLE_EQ( g.bounds.center.y, 5.);

    EXPECT_EQ(g.x_to_index(0.5),  0); // <- past-the-post attempt

    EXPECT_EQ(g.x_to_index(1.0),  0); // <- actual first cell
    EXPECT_EQ(g.x_to_index(1.1),  0); // <- actual first cell
    EXPECT_EQ(g.x_to_index(1.49),  0); // <- actual first cell
    EXPECT_EQ(g.x_to_index(1.6),  1);
    EXPECT_EQ(g.x_to_index(2.1),  2);
    EXPECT_EQ(g.x_to_index(2.6),  3);
    EXPECT_EQ(g.x_to_index(3.1),  4);
    EXPECT_EQ(g.x_to_index(3.6),  5);
    EXPECT_EQ(g.x_to_index(4.1),  6);
    EXPECT_EQ(g.x_to_index(4.6),  7);
    EXPECT_EQ(g.x_to_index(5.1),  8);
    EXPECT_EQ(g.x_to_index(5.6),  9);
    EXPECT_EQ(g.x_to_index(6.1), 10);
    EXPECT_EQ(g.x_to_index(6.6), 11);
    EXPECT_EQ(g.x_to_index(7.1), 12);
    EXPECT_EQ(g.x_to_index(7.6), 13);
    EXPECT_EQ(g.x_to_index(8.1), 14);
    EXPECT_EQ(g.x_to_index(8.5), 15); // <- actual last cell 
    EXPECT_EQ(g.x_to_index(8.6), 15); // <- actual last cell 
    EXPECT_EQ(g.x_to_index(8.99), 15); // <- actual last cell 
    EXPECT_EQ(g.x_to_index(9.1), 15); // <- past-the-post attempt
}

TEST(GridTest, LoadGridFromFile) {
    quadtree::Grid g(9., 1., {4.5,4.5});

    EXPECT_EQ( g.size(), 81);
    EXPECT_EQ( g.width(), 9);
    EXPECT_DOUBLE_EQ( g.spacing, 1.0);
    EXPECT_DOUBLE_EQ( g.bounds.center.x,    4.5);
    EXPECT_DOUBLE_EQ( g.bounds.center.y,    4.5);
    EXPECT_DOUBLE_EQ( g.bounds.half_width,  4.5);

    EXPECT_DOUBLE_EQ( g.bounds.get_x_min(),   0.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_min(),   0.);
    EXPECT_DOUBLE_EQ( g.bounds.get_x_max(),   9.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_max(),   9.);

    string arrow_csv = "99, 99, 99, 99,  0, 99, 99, 99, 99,\n\
                        99, 99, 99,  0,  0,  0, 99, 99, 99,\n\
                        99, 99,  0,  0,  0,  0,  0, 99, 99,\n\
                        99,  0,  0,  0,  0,  0,  0,  0, 99,\n\
                         0,  0,  0,  0,  0,  0,  0,  0,  0,\n\
                        99, 99, 99,  0,  0,  0, 99, 99, 99,\n\
                        99, 99, 99,  0,  0,  0, 99, 99, 99,\n\
                        99, 99, 99,  0,  0,  0, 99, 99, 99,\n\
                        99, 99, 99,  0,  0,  0, 99, 99, 99,\n";
    std::istringstream source(arrow_csv);
    g.load_grid(source);

    // // DEBUG
    // g.draw(cerr);

    ASSERT_EQ( g(0,0), 99);
    ASSERT_EQ( g(1,1), 99);
    ASSERT_EQ( g(2,2), 99);
    ASSERT_EQ( g(2,3), 99);
    ASSERT_EQ( g(2,4),  0);

    ASSERT_EQ( g(3,0),  0);
    ASSERT_EQ( g(3,1),  0);
    ASSERT_EQ( g(3,2),  0);
    ASSERT_EQ( g(3,3),  0);
    ASSERT_EQ( g(3,4),  0);
    ASSERT_EQ( g(3,5),  0);
    ASSERT_EQ( g(3,6),  0);
    ASSERT_EQ( g(3,7),  0);
    ASSERT_EQ( g(3,8), 99);

    ASSERT_EQ( g(0,8), 99);
    ASSERT_EQ( g(1,7), 99);
    ASSERT_EQ( g(2,6),  0);
}

TEST(GridTest, LoadPolygonFromVector) {
    quadtree::Grid g(8., 0.5, {8.,8.});

    EXPECT_EQ( g.size(), 256);
    EXPECT_EQ( g.width(), 16);
    EXPECT_DOUBLE_EQ( g.spacing, 0.5);
    EXPECT_DOUBLE_EQ( g.bounds.center.x,    8.);
    EXPECT_DOUBLE_EQ( g.bounds.center.y,    8.);
    EXPECT_DOUBLE_EQ( g.bounds.half_width,  4.);

    EXPECT_DOUBLE_EQ( g.bounds.get_x_min(),   4.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_min(),   4.);
    EXPECT_DOUBLE_EQ( g.bounds.get_x_max(),  12.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_max(),  12.);

    // reset grid
    g.fill(99);

    geometry::Polygon diamond("ExpConfigPoly", {{11.5,  8.0},
                                                { 8.0, 11.5},
                                                { 4.5,  8.0},
                                                { 8.0,  4.5}});
    g.fill(diamond, 0);

    // // DEBUG
    // g.draw(cerr);

    g.to_png("gridtest.png");

    ASSERT_EQ( g(4,15), 99);
    ASSERT_EQ( g(4,14), 99);
    ASSERT_EQ( g(4,13), 99);
    ASSERT_EQ( g(4,12), 99);
    ASSERT_EQ( g(4,11),  0);
    ASSERT_EQ( g(4, 9),  0);
    ASSERT_EQ( g(4, 8),  0);
    ASSERT_EQ( g(4, 7),  0);
    ASSERT_EQ( g(4, 6),  0);
    ASSERT_EQ( g(4, 5),  0);
    ASSERT_EQ( g(4, 4), 99);
    ASSERT_EQ( g(4, 3), 99);
    ASSERT_EQ( g(4, 2), 99);
    ASSERT_EQ( g(4, 1), 99);
    ASSERT_EQ( g(4, 0), 99);

    ASSERT_EQ( g( 0, 5), 99);
    ASSERT_EQ( g( 1, 5), 99);
    ASSERT_EQ( g( 2, 5), 99);
    ASSERT_EQ( g( 3, 5), 99);
    ASSERT_EQ( g( 4, 5),  0);
    ASSERT_EQ( g( 5, 5),  0);
    ASSERT_EQ( g( 6, 5),  0);
    ASSERT_EQ( g( 7, 5),  0);
    ASSERT_EQ( g( 8, 5),  0);
    ASSERT_EQ( g( 9, 5),  0);
    ASSERT_EQ( g(10, 5),  0);
    ASSERT_EQ( g(11, 5),  0);
    ASSERT_EQ( g(12, 5), 99);
    ASSERT_EQ( g(13, 5), 99);
    ASSERT_EQ( g(14, 5), 99);
    ASSERT_EQ( g(15, 5), 99);
}

TEST(GridTest, SavePNG) {
    quadtree::Grid g(9., 1., {4.5,4.5});

    EXPECT_EQ( g.size(), 81);
    EXPECT_EQ( g.width(), 9);
    EXPECT_DOUBLE_EQ( g.spacing, 1.0);
    EXPECT_DOUBLE_EQ( g.bounds.center.x,    4.5);
    EXPECT_DOUBLE_EQ( g.bounds.center.y,    4.5);
    EXPECT_DOUBLE_EQ( g.bounds.half_width,  4.5);

    EXPECT_DOUBLE_EQ( g.bounds.get_x_min(), 0.0);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_min(), 0.0);
    EXPECT_DOUBLE_EQ( g.bounds.get_x_max(), 9.0);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_max(), 9.0);
    
    string arrow_csv = "242, 242, 242, 242,  0, 242, 242, 242, 242,\n\
                        242, 242, 242,   0,  0,   0, 242, 242, 242,\n\
                        242, 242,   0,   0,  0,   0,   0, 242, 242,\n\
                        242,   0,   0,   0,  0,   0,   0,   0, 242,\n\
                          0,   0,   0,   0,  0,   0,   0,   0,   0,\n\
                        242,   0,   0,   0,  0, 242, 242, 242, 242,\n\
                        242, 242,   0,   0,  0, 242, 242, 242, 242,\n\
                        242, 242, 242,   0,  0, 242, 242, 242, 242,\n\
                        242, 242, 242, 242,  0, 242, 242, 242, 242,\n";
    std::istringstream source(arrow_csv);
    g.load_grid(source);
    
    // DEBUG
    g.draw(cerr);

    // // because this manually tested, turn off by default.
    g.to_png("gridtest.png");
}

void fill_gradient(quadtree::Grid& g, uint8_t min_value, uint8_t max_value){
    assert( min_value < max_value );
    // note: quantization to integers may cause some loss in "expected" precision, here
    const int value_span = static_cast<int>(max_value) - static_cast<int>(min_value);
    const double x_incr = static_cast<double>(value_span) /2 /g.dimension;
    const double y_incr = static_cast<double>(value_span) /2 /g.dimension;
    
    for( int yi = g.dimension-1; 0 <= yi; --yi){
        for( int xi = g.dimension-1; 0 <= xi; --xi){
            g(xi,yi) = min_value + xi*x_incr + yi*y_incr;
        }
    }
}
    
TEST(GridTest, FillGradient) {
    quadtree::Grid g(256., 0.5, {0.,0.});

    EXPECT_EQ( g.size(), 512*512);
    EXPECT_EQ( g.width(), 512);
    EXPECT_DOUBLE_EQ( g.spacing, 0.5);
    EXPECT_DOUBLE_EQ( g.bounds.center.x,      0.);
    EXPECT_DOUBLE_EQ( g.bounds.center.y,      0.);
    EXPECT_DOUBLE_EQ( g.bounds.half_width,  128.);

    EXPECT_DOUBLE_EQ( g.bounds.get_x_min(), -128.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_min(), -128.);
    EXPECT_DOUBLE_EQ( g.bounds.get_x_max(),  128.);
    EXPECT_DOUBLE_EQ( g.bounds.get_y_max(),  128.);

    const uint8_t min_value = 0;
    const uint8_t max_value = 255;
    fill_gradient(g, min_value, max_value);

    // // DEBUG
    // g.draw(cerr);
    
    g.to_png("gridtest.png");
}

};
