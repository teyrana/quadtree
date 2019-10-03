#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <nlohmann/json/json.hpp>

#include "quadtree/tree.hpp"
#include "geometry/point.hpp"
#include "geometry/polygon.hpp"
#include "terrain.hpp"
#include "terrain.inl"

using std::cerr;
using std::endl;
using std::string;
using std::vector;

using nlohmann::json;

using terrain::geometry::Point;
using terrain::geometry::Bounds;

namespace terrain::quadtree {

TEST(QuadTreeTest, ConstructDefault) {
    Tree tree;
    Terrain terrain(tree);

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 4.);
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());
    
    // // DEBUG for line above
    // ASSERT_DOUBLE_EQ(terrain.get_bounds().center.x,  0);
    // ASSERT_DOUBLE_EQ(terrain.get_bounds().center.y,  0);
    // ASSERT_DOUBLE_EQ(terrain.get_bounds().width(),  32);

    EXPECT_TRUE( tree.root->is_leaf() );
}

TEST(QuadTreeTest, ConstructByCenterAndSize) {
    Tree tree({{1,1}, 256}, 1.0);
    Terrain terrain(tree);

    EXPECT_TRUE( Bounds({1,1}, 256) == terrain.get_bounds());

    // functional tests:
    // (1) in bounds
    ASSERT_TRUE(tree.contains({0,0}));
    // (2) out of bounds in x
    ASSERT_FALSE(tree.contains({200,0}));
    // (3) out of bounds in y
    ASSERT_FALSE(tree.contains({0,200}));
    // (4) out of bounds in both x AND y
    ASSERT_FALSE(tree.contains({130,130}));
}


TEST( QuadTreeTest, ConstructAndSetBounds) {
    Tree tree({{5,3}, 17}, 1.);
    Terrain terrain(tree);

    // pre-conditions
    EXPECT_TRUE( Bounds({5,3}, 17) == terrain.get_bounds());
    EXPECT_DOUBLE_EQ( 1.0, terrain.get_precision());

    const Bounds new_bounds = {{1.,1}, 256};
    const double new_precision = 32.;
    // test target
    tree.reset(new_bounds, new_precision);
    // test target

    // post-conditions
    EXPECT_TRUE( Bounds({1,1}, 256) == terrain.get_bounds());
    EXPECT_DOUBLE_EQ( 32.0, terrain.get_precision());
}

TEST( QuadTreeTest, LoadMalformedSource){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 4.);
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());
    
    // this is simply a malformed document.  It should not parse.
    std::istringstream source(R"({"bounds": {"x": 100, "y": 100, "width": )");

    // this should fail. Gracefully.
    EXPECT_FALSE(terrain.load(source));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());
}

TEST( QuadTreeTest, LoadBoundsOnly){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 4.);
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());
    
    // construct a valid document, with correct fields, but simply .. missing things: 
    std::stringstream stream(R"({"bounds": {"x": 100, "y": 100, "width": 64}} )");

    // this should fail. (gracefully)
    EXPECT_FALSE(terrain.load(stream));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());
}

TEST( QuadTreeTest, LoadValidTree){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 4.);
    EXPECT_TRUE( Tree::default_bounds == terrain.get_bounds());

    json source = {{"bounds", {{"x", 0}, {"y", 0}, {"width", 1024}}},
                   {"precision", 1.},
                   {"tree", {{"NE", {{"NE", 0.0},
                                   {"NW", 0.0}, 
                                   {"SE", 0.0}, 
                                   {"SW", 0.0}}},
                             {"NW", 0.0},
                             {"SE", 0.0}, 
                             {"SW", 0.0}}}};

    // ... just ... ignore the rest of that document.  It's really not important.
    tree.load_tree(source["tree"]);

    // // debug
    // terrain.debug();

    // test shape
    ASSERT_FALSE(tree.root->is_leaf());
    ASSERT_FALSE(tree.root->get_northeast()->is_leaf());
    ASSERT_TRUE(tree.root->get_southwest()->is_leaf());
}

TEST(QuadTreeTest, LoadGridFromJSON) {
    Tree tree;
    Terrain terrain(tree);

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 4.0);
    EXPECT_TRUE( Bounds({0,0},32) == terrain.get_bounds());

    std::istringstream stream(R"(
        {"bounds": {"x": 1, "y": 1, "width": 256},
         "grid":[[88, 88, 88, 88,  0, 88, 88, 88],
                 [88, 88, 88,  0,  0,  0, 88, 88],
                 [88, 88,  0,  0,  0,  0,  0, 88],
                 [88,  0,  0,  0,  0,  0,  0,  0],
                 [ 0,  0,  0,  0, 88, 88, 88, 88],
                 [88,  0,  0,  0, 88, 88, 88, 88],
                 [88, 88,  0,  0, 88, 88, 88, 88],
                 [88, 88, 88,  0, 88, 88, 88, 88]]} )");
    // test target
    ASSERT_TRUE(terrain.load(stream));
    // test target

    // // DEBUG
    // terrain.debug();

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 32);
    ASSERT_TRUE(Bounds({1,1},256) == terrain.get_bounds());
    
    {// test tree shape
        EXPECT_EQ(tree.get_height(), 4);
        EXPECT_FALSE(tree.root->is_leaf());

        // spot check #1: RT-NE-SW-quadrant
        const auto* check1 = tree.root->get_northeast()->get_southwest();
        ASSERT_TRUE(check1->is_leaf());
        ASSERT_DOUBLE_EQ(check1->get_value(), 0);

        auto& check1_bounds = check1->get_bounds();
        ASSERT_DOUBLE_EQ(check1_bounds.center.x,   33);
        ASSERT_DOUBLE_EQ(check1_bounds.center.y,   33);
        ASSERT_DOUBLE_EQ(check1_bounds.width(),    64);

        // spot check #2: RT-NW-NE-NW quadrant
        const auto* check2 = tree.root->get_northwest()->get_northeast()->get_northwest();
        ASSERT_TRUE(check2->is_leaf());
        ASSERT_DOUBLE_EQ(check2->get_value(), 88);

        auto& check2_bounds = check2->get_bounds();
        ASSERT_DOUBLE_EQ(check2_bounds.center.x,  -47);
        ASSERT_DOUBLE_EQ(check2_bounds.center.y,  113);
        ASSERT_DOUBLE_EQ(check2_bounds.width(),    32);
    }
 
    const Tree& frozen = tree;
    EXPECT_EQ( frozen.search({  16,   16}),   0);
    EXPECT_EQ( frozen.search({  48,   48}),   0);
    EXPECT_EQ( frozen.search({  72,   72}),  88);
    EXPECT_EQ( frozen.search({ 104,  104}),  88);

    // [-127, -95, -63, -31, 1, 33, 65, 97, 129]
    EXPECT_EQ( frozen.search({ -70,  130}), 153);
    EXPECT_EQ( frozen.search({ -70,  129}),  88);
    EXPECT_EQ( frozen.search({ -70,   97}),  88);
    EXPECT_EQ( frozen.search({ -70,   65}),  88);
    EXPECT_EQ( frozen.search({ -70,   33}),   0);
    EXPECT_EQ( frozen.search({ -70,    1}),   0);
    EXPECT_EQ( frozen.search({ -70,  -31}),   0);
    EXPECT_EQ( frozen.search({ -70,  -63}),  88);
    EXPECT_EQ( frozen.search({ -70,  -95}),  88);
    EXPECT_EQ( frozen.search({ -70, -127}),  88);
    EXPECT_EQ( frozen.search({ -70, -130}), 153);

    EXPECT_EQ( frozen.search({  15,  130}), 153);
    EXPECT_EQ( frozen.search({  15,  129}),   0);
    EXPECT_EQ( frozen.search({  15,   97}),   0);
    EXPECT_EQ( frozen.search({  15,   65}),   0);
    EXPECT_EQ( frozen.search({  15,   33}),   0);
    EXPECT_EQ( frozen.search({  15,    1}),  88);
    EXPECT_EQ( frozen.search({  15,  -31}),  88);
    EXPECT_EQ( frozen.search({  15,  -63}),  88);
    EXPECT_EQ( frozen.search({  15,  -95}),  88);
    EXPECT_EQ( frozen.search({  15, -127}),  88);
    EXPECT_EQ( frozen.search({  15, -130}), 153);
}

TEST(QuadTreeTest, LoadPolygonFromJSON) {
    quadtree::Tree tree;
    Terrain terrain(tree);

    constexpr double boundary_width = 16.;   // overall boundary
    constexpr double diamond_width = 8.;
    constexpr double desired_precision = 1.0;
    // =====
    constexpr Point center(boundary_width/2, boundary_width/2);
    constexpr Bounds expected_bounds(center, boundary_width);
    json source = { {"bounds", {{"x", center.x}, {"y", center.y}, {"width", boundary_width}}},
                    {"precision", desired_precision},
                    {"allow", {{{center.x + diamond_width, center.y},
                                {center.x                , center.y + diamond_width},
                                {center.x - diamond_width, center.y},
                                {center.x                , center.y - diamond_width}}}}};
    std::istringstream stream(source.dump());

    // // DEBUG
    // cerr << "======\n" << source.dump(4) << "\n======\n" << endl;

    ASSERT_TRUE(terrain.load(stream));

    // // DEBUG
    // terrain.debug();

    EXPECT_DOUBLE_EQ( terrain.get_precision(), desired_precision);

    ASSERT_TRUE( expected_bounds == terrain.get_bounds());

    ASSERT_EQ( tree.search({ 4.5, 15.5}), 0x99);
    ASSERT_EQ( tree.search({ 4.5, 14.5}), 0x99);
    ASSERT_EQ( tree.search({ 4.5, 13.5}), 0x99);
    ASSERT_EQ( tree.search({ 4.5, 12.5}),    0);
    ASSERT_EQ( tree.search({ 4.5, 11.5}),    0);
    ASSERT_EQ( tree.search({ 4.5, 10.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  9.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  8.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  7.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  6.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  5.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  4.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  3.5}),    0);
    ASSERT_EQ( tree.search({ 4.5,  2.5}), 0x99);
    ASSERT_EQ( tree.search({ 4.5,  1.5}), 0x99);
    ASSERT_EQ( tree.search({ 4.5,  0.5}), 0x99);

    ASSERT_EQ( tree.search({  0.5, 5.5}), 0x99);
    ASSERT_EQ( tree.search({  1.5, 5.5}), 0x99);
    ASSERT_EQ( tree.search({  2.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  3.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  4.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  5.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  6.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  7.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  8.5, 5.5}),    0);
    ASSERT_EQ( tree.search({  9.5, 5.5}),    0);
    ASSERT_EQ( tree.search({ 10.5, 5.5}),    0);
    ASSERT_EQ( tree.search({ 11.5, 5.5}),    0);
    ASSERT_EQ( tree.search({ 12.5, 5.5}),    0);
    ASSERT_EQ( tree.search({ 13.5, 5.5}), 0x99);
    ASSERT_EQ( tree.search({ 14.5, 5.5}), 0x99);
    ASSERT_EQ( tree.search({ 15.5, 5.5}), 0x99);
}

TEST( QuadTreeTest, WriteLoadCycle){
    Tree source_tree({{11,11}, 128}, 32);
    Terrain source_terrain(source_tree);

    // modify tree in a characteristic way
    source_tree.root->split();
    source_tree.root->get_northeast()->split();
    source_tree.root->get_southwest()->split();

    // Set interesting values
    source_tree.root->get_northeast()->get_northeast()->set_value(21);
    source_tree.root->get_northeast()->get_northwest()->set_value(22);
    source_tree.root->get_northeast()->get_southeast()->set_value(23);
    source_tree.root->get_northeast()->get_southwest()->set_value(24);
    source_tree.root->get_northwest()->set_value(11);
    source_tree.root->get_southwest()->get_northeast()->set_value(31);
    source_tree.root->get_southwest()->get_northwest()->set_value(32);
    source_tree.root->get_southwest()->get_southeast()->set_value(33);
    source_tree.root->get_southwest()->get_southwest()->set_value(34);
    source_tree.root->get_southeast()->set_value(55);
    
    // // DEBUG
    // source_tree.debug();

    // write tree #1 to the serialization buffer
    std::stringstream buffer;
    ASSERT_TRUE(source_terrain.json(buffer));
    
    // // DEBUG
    // cerr << buffer.str() << endl;
    
    // load contents into another tree
    Tree load_tree;
    Terrain load_terrain(load_tree);

    buffer.seekg(0);
    ASSERT_TRUE(load_terrain.load(buffer));

    // // DEBUG
    // load_tree.debug();

    // test contents of test_tree
    { // test bounds:
        auto& bounds = load_terrain.get_bounds();
        ASSERT_DOUBLE_EQ(bounds.center.x,    11);
        ASSERT_DOUBLE_EQ(bounds.center.y,    11);
        ASSERT_DOUBLE_EQ(bounds.width(),  128);
    }
    { // test tree shape
        auto & root = load_tree.root;
        ASSERT_FALSE(root->is_leaf());
        {
            auto* ne_quad = root->get_northeast();
            ASSERT_FALSE(ne_quad->is_leaf());
            ASSERT_TRUE(ne_quad->get_northeast()->is_leaf());
            ASSERT_TRUE(ne_quad->get_northwest()->is_leaf());
            ASSERT_TRUE(ne_quad->get_southwest()->is_leaf());
            ASSERT_TRUE(ne_quad->get_southeast()->is_leaf());
        }
        ASSERT_TRUE(root->get_northwest()->is_leaf());
        ASSERT_TRUE(root->get_southeast()->is_leaf());
        {
            auto* sw_quad = root->get_northeast();
            ASSERT_FALSE(sw_quad->is_leaf());
            ASSERT_TRUE(sw_quad->get_northeast()->is_leaf());
            ASSERT_TRUE(sw_quad->get_northwest()->is_leaf());
            ASSERT_TRUE(sw_quad->get_southwest()->is_leaf());
            ASSERT_TRUE(sw_quad->get_southeast()->is_leaf());	    
        }
    }
}
    
TEST( QuadTreeTest, TestSearchExplicitTree) {
    Tree tree({{0,0}, 100}, 50);
    Terrain terrain(tree);
    tree.root->split();

    ASSERT_FALSE(tree.root->is_leaf());

    ASSERT_TRUE(Bounds({0,0}, 100) == terrain.get_bounds());
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 50);

    cell_value_t false_value = 5;
    cell_value_t true_value = 14;

    // .... Out Of Bounds:
    ASSERT_EQ(tree.search({110, 110}), cell_error_value);

    // Set Quadrant I:
    tree.root->get_northeast()->set_value(true_value);
    // Set Quadrdant II:
    tree.root->get_northwest()->set_value(false_value);
    // Set Quadrant III:
    tree.root->get_southwest()->set_value(true_value);
    // Set Quadrant IV:
    tree.root->get_southeast()->set_value(false_value);

    // // DEBUG
    // tree.debug();

    // functional tests:
    // .... Quadrant I:
    EXPECT_EQ(tree.search({ 25,  25}), true_value);
    // .... Quadrant II:
    EXPECT_EQ(tree.search({-25,  25}), false_value);
    // .... Quadrant III:
    EXPECT_EQ(tree.search({-25, -25}), true_value);
    // .... Quadrant IV:
    EXPECT_EQ(tree.search({ 25, -25}), false_value);
}

struct TestPoint{
    const double x;
    const double y;
    const cell_value_t value;

    constexpr TestPoint(const double _x, const double _y, const cell_value_t _value): x(_x), y(_y), value(_value) {}
};

TEST( QuadTreeTest, TestInterpolateTree){
    Tree tree({{1,1}, 64}, 1.0);
    Terrain terrain(tree);
    tree.root->split();

    // Set Quadrant I:
    tree.root->get_northeast()->set_value(0);
    // Set Quadrdant II:
    tree.root->get_northwest()->set_value(50);
    // Set Quadrant III:
    tree.root->get_southwest()->set_value(100);
    // Set Quadrant IV:
    tree.root->get_southeast()->set_value(50);


    vector<TestPoint> test_cases;
    test_cases.emplace_back( -35,    4, cell_default_value);  // Start out of bounds
    test_cases.emplace_back( -33,    4, cell_default_value);
    test_cases.emplace_back( -32,    4, cell_default_value);
    test_cases.emplace_back( -31,    4,   70);  // border of tree
    test_cases.emplace_back( -30.9,  4,   70);
    test_cases.emplace_back( -30,    4,   70);
    test_cases.emplace_back( -20,    4,   70);
    test_cases.emplace_back( -17,    4,   70);
    test_cases.emplace_back( -16,    4,   70);
    test_cases.emplace_back( -15.1,  4,   70);
    test_cases.emplace_back( -15.0,  4,   70);  // breakpoint - center of outer cell
    test_cases.emplace_back( -14.9,  4,   70);
    test_cases.emplace_back( -10,    4,   62);
    test_cases.emplace_back( - 5,    4,   54);
    test_cases.emplace_back(   0,    4,   47);
    test_cases.emplace_back(   1,    4,   45);  // midpoint
    test_cases.emplace_back(   2,    4,   43);
    test_cases.emplace_back(  10,    4,   31);
    test_cases.emplace_back(  14,    4,   25);
    test_cases.emplace_back(  15,    4,   23);
    test_cases.emplace_back(  16,    4,   22);  // breakpoint - center of outer cell
    test_cases.emplace_back(  17,    4,   20);
    test_cases.emplace_back(  20,    4,   20);
    test_cases.emplace_back(  30,    4,   20);
    test_cases.emplace_back(  31,    4,   20);
    test_cases.emplace_back(  32,    4,   20);
    test_cases.emplace_back(  33,    4,   20);  // border of tree
    test_cases.emplace_back(  34,    4, cell_default_value);
    test_cases.emplace_back(  35,    4, cell_default_value);

    // =====================================
    for( const TestPoint& expect : test_cases){
        const auto& actual_value = tree.interp({expect.x, expect.y});

        std::ostringstream buf;
        buf << "@@  x=" << expect.x << "  y=" << expect.y << "  v=" << expect.value << endl;

        ASSERT_EQ(actual_value, expect.value) << buf.str();
    }
}


TEST(TreeTest, SavePNG) {
    quadtree::Tree tree;
    Terrain terrain(tree);

    constexpr double boundary_width = 16.;   // overall boundary
    constexpr double diamond_width = 8.;
    
    constexpr double desired_precision = 0.4; ///< exhibits 'stripe' noise:
    // constexpr double desired_precision = 0.25;  ///< displays cleanly

    // =====
    constexpr Point center(boundary_width/2, boundary_width/2);
    json source = { {"bounds", {{"x", center.x}, {"y", center.y}, {"width", boundary_width}}},
                    {"precision", desired_precision},
                    {"allow", {{{center.x + diamond_width, center.y},
                                {center.x                , center.y + diamond_width},
                                {center.x - diamond_width, center.y},
                                {center.x                , center.y - diamond_width}}}}};
    std::istringstream stream(source.dump());

    ASSERT_TRUE(terrain.load(stream));

    // DEBUG
    terrain.debug();
    ASSERT_TRUE(false) << " !! Note: this is unique to the QuadTree storage, and is visible in both debug and png output.\n";

    const string filename("tree.test.png");
    // because this manually tested, turn off by default.
    // {
    //     FILE* dest = fopen(filename.c_str(), "wb");
    //     if(nullptr == dest){
    //         cerr << "could not open destination .png file ("<<filename<<") for reading." << endl;
    //         return;
    //     }
    //     terrain.png(dest);
    // }
}

} // namespace quadtree
