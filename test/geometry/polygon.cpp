#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "geometry/polygon.hpp"

using std::vector;

using Eigen::Vector2d;

using terrain::geometry::Polygon;


namespace terrain::geometry {

TEST(PolygonTest, DefaultConfiguration) {
    const Polygon shape;

    const auto& bounds = shape.get_bounds();
    EXPECT_DOUBLE_EQ(bounds.center[0],    0.5);
    EXPECT_DOUBLE_EQ(bounds.center[1],    0.5);
    ASSERT_DOUBLE_EQ(bounds.get_x_max(), 1.);
    ASSERT_DOUBLE_EQ(bounds.get_x_min(), 0.);
    ASSERT_DOUBLE_EQ(bounds.get_y_max(), 1.);
    ASSERT_DOUBLE_EQ(bounds.get_y_min(), 0.);

    const auto& points = shape.points;
    ASSERT_TRUE(points[0].isApprox(Vector2d(0,0)));
    ASSERT_TRUE(points[1].isApprox(Vector2d(1,0)));
    ASSERT_TRUE(points[2].isApprox(Vector2d(1,1)));
    ASSERT_TRUE(points[3].isApprox(Vector2d(0,1)));
}

#include <iostream>
TEST(PolygonTest, LoadList_5Point) {
    // Note:  this polygen is configured as CW:
    //    it will be enclosed, and reversed, internally
    Polygon shape( {{ 3, 4},
                    { 5,11},
                    {12, 8},
                    { 9, 5},
                    { 5, 6}});

    // DEBUG
    //shape.write_yaml(std::cerr, "    ");
    
    const auto& bounds = shape.bounds;
    ASSERT_NEAR(bounds.center[0],     7.5, 1e-6);
    ASSERT_NEAR(bounds.center[1],     7.5, 1e-6);
    ASSERT_NEAR(bounds.half_width,   4.5, 1e-6);

    ASSERT_NEAR(bounds.get_x_max(), 12.0, 1e-6);
    ASSERT_NEAR(bounds.get_x_min(),  3.0, 1e-6);
    ASSERT_NEAR(bounds.get_y_max(), 12.0, 1e-6);
    ASSERT_NEAR(bounds.get_y_min(),  3.0, 1e-6);
}


TEST(PolygonTest, LoadList_DiamondRhombus) {
    Polygon shape({{1,0},{0,1},{-1,0},{0,-1}});

    // DEBUG
    //shape.write_yaml(std::cerr, "    ");

    const auto& bounds = shape.bounds;
    ASSERT_NEAR(bounds.center[0],     0.0, 1e-6);
    ASSERT_NEAR(bounds.center[1],     0.0, 1e-6);
    ASSERT_NEAR(bounds.half_width,   1.0, 1e-6);

    ASSERT_NEAR(bounds.get_x_max(),  1.0, 1e-6);
    ASSERT_NEAR(bounds.get_x_min(), -1.0, 1e-6);
    ASSERT_NEAR(bounds.get_y_max(),  1.0, 1e-6);
    ASSERT_NEAR(bounds.get_y_min(), -1.0, 1e-6);
}

// TEST(BoundAreaTests, InBoundingBoxByX) {
//     BoundaryPolygon bounds;
//     bounds.setParam("margin", "20.");
//     bounds.setParam("priority_function", "linear");
//     bounds.setParam("points", "-200,-200: 200,-200: 200,200: -200,200");

//     // ====== ====== Preconditions ====== ======
//     ASSERT_DOUBLE_EQ(bounds.min.x, -200);
//     ASSERT_DOUBLE_EQ(bounds.max.x,  200);
//     ASSERT_DOUBLE_EQ(bounds.min.y, -200);
//     ASSERT_DOUBLE_EQ(bounds.max.y,  200);
//     ASSERT_DOUBLE_EQ(bounds.margin,  20);
//     ASSERT_DOUBLE_EQ(bounds.center[0], 0);
//     ASSERT_DOUBLE_EQ(bounds.center[1], 0);
//     ASSERT_TRUE(bounds.is_convex);

//     // ====== ====== Graphics Message Output ====== ======
//     const string& poly2 = bounds.getBoundaryPolygonGraphics(true,"aqua");
//     const string exp2("label=Boundary,active=true,pts={-200,-200:200,-200:200,200:-200,200:-200,-200},edge_color=aqua");
//     ASSERT_EQ(poly2, exp2);

//     vector<TestPoint> test_cases;
//     test_cases.emplace_back( -205,  0, 1.00,   90);
//     test_cases.emplace_back( -201,  0, 1.00,   90);
//     test_cases.emplace_back( -200,  0, 1.00,   90);
//     test_cases.emplace_back( -199,  0,    0,    0);
//     test_cases.emplace_back( -190,  0,    0,    0);
//     test_cases.emplace_back( -181,  0,    0,    0);
//     test_cases.emplace_back( -180,  0,    0,    0);
//     test_cases.emplace_back( -179,  0,    0,    0);
//     test_cases.emplace_back(    0,  0,    0,    0);
//     test_cases.emplace_back(  179,  0,    0,    0);
//     test_cases.emplace_back(  180,  0,    0,    0);
//     test_cases.emplace_back(  181,  0,    0,    0);
//     test_cases.emplace_back(  190,  0,    0,    0);
//     test_cases.emplace_back(  199,  0,    0,    0);
//     test_cases.emplace_back(  200,  0, 1.00,  270);
//     test_cases.emplace_back(  201,  0, 1.00,  270);

//     // =====================================
//     for( uint i=0; i < test_cases.size(); i++){
//         const TestPoint& expect = test_cases[i];

//         const PolarVector2D& actual = bounds.getHeadingDesiredBoundingBox(expect.x,expect.y);

//         ostringstream buf;
//         buf << "@@ x=" << expect.x << " y=" << expect.y;
//         buf << "    =>    " << actual.magnitude << " \u2220 "<< actual.heading <<"\u00b0";
//         const string& case_descriptor = buf.str();

//         ASSERT_NEAR(actual.magnitude, expect.magnitude, 0.01) << case_descriptor;
//         ASSERT_NEAR(actual.heading,   expect.heading, 0.1) << case_descriptor;
//     }
// }



};  // namespace terrain::geometry
