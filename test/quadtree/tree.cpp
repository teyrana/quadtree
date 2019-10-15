#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Geometry>

#include <gtest/gtest.h>

#include <nlohmann/json/json.hpp>

#include "geometry/layout.hpp"
#include "geometry/polygon.hpp"
#include "quadtree/tree.hpp"
#include "terrain.hpp"

using std::cerr;
using std::endl;
using std::string;
using std::vector;

using Eigen::Vector2d;

using nlohmann::json;

using terrain::geometry::Bounds;

const Bounds& default_bounds = terrain::geometry::Layout::default_layout.bounds;

namespace terrain::quadtree {

TEST(QuadTreeTest, ConstructDefault) {
    Tree tree;
    Terrain terrain(tree);

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());
    
    // // DEBUG for line above
    // ASSERT_DOUBLE_EQ(terrain.get_bounds().center[0],  0);
    // ASSERT_DOUBLE_EQ(terrain.get_bounds().center[1],  0);
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
    EXPECT_DOUBLE_EQ( 0.53125, terrain.get_precision());

    const Bounds new_bounds = {{1.,1}, 256};
    const double new_precision = 32.; // this will very much get snapped down

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
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());
    
    // this is simply a malformed document.  It should not parse.
    std::istringstream source(R"({"bounds": {"x": 100, "y": 100, "width": )");

    // this should fail. Gracefully.
    EXPECT_FALSE(terrain.load(source));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_TRUE( default_bounds == terrain.get_bounds());
}

TEST( QuadTreeTest, LoadBoundsOnly){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());
    
    // construct a valid document, with correct fields, but simply .. missing things: 
    std::stringstream stream(R"({"bounds": {"x": 100, "y": 100, "width": 64}} )");

    // this should fail. (gracefully)
    EXPECT_FALSE(terrain.load(stream));

    // these tests should be *exactly* the same as before the 'load' call
    EXPECT_TRUE( default_bounds == terrain.get_bounds());
}

TEST( QuadTreeTest, LoadValidTree){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());

    const json source = {
            {"NE", {
                {"NE", 0.0},
                {"NW", 0.0}, 
                {"SE", 0.0}, 
                {"SW", 0.0}}},
            {"NW", 0.0},
            {"SE", 0.0}, 
            {"SW", 0.0}};

    tree.load_tree(source);

    // // debug
    // tree.debug_tree();

    // test shape
    ASSERT_FALSE( tree.root->is_leaf());
    ASSERT_FALSE( tree.root->get_northeast()->is_leaf());
    ASSERT_TRUE( tree.root->get_southwest()->is_leaf());
}

TEST( QuadTreeTest, CalculateFullLoading){
    EXPECT_EQ( Tree::calculate_complete_tree(0), 1);
    EXPECT_EQ( Tree::calculate_complete_tree(1), 5);
    EXPECT_EQ( Tree::calculate_complete_tree(2), 21);
    ASSERT_EQ( Tree::calculate_complete_tree(3), 85);
    ASSERT_EQ( Tree::calculate_complete_tree(4), 341);
    ASSERT_EQ( Tree::calculate_complete_tree(5), 1365);
}

TEST( QuadTreeTest, CalculateMemoryUsage){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());

    EXPECT_EQ(sizeof(Terrain<Tree>), 32);
    EXPECT_EQ(sizeof(Tree), 16);
    EXPECT_EQ(sizeof(Vector2d), 16);
    EXPECT_EQ(sizeof(Bounds), 32);   // :(
    EXPECT_EQ(sizeof(Layout), 64);   // :(
    EXPECT_EQ(sizeof(Node), 40);

}
    
TEST( QuadTreeTest, CalculateLoadFactor){
    Tree tree;
    Terrain terrain(tree);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());

    const json source = {
        {"NE", {
            {"NE", 0.0},
            {"NW", 0.0},
            {"SE", {
                {"NE", 0.0},
                {"NW", 0.0},
                {"SE", 0.0},
                {"SW", 0.0}}},
            {"SW", 0.0}}},
        {"NW", 0.0},
        {"SE", 0.0}, 
        {"SW", 0.0}};

    tree.load_tree(source);

    // // debug
    // tree.debug_tree();

    // test shape
    ASSERT_FALSE( tree.root->is_leaf());
    ASSERT_FALSE( tree.root->get_northeast()->is_leaf());
    ASSERT_TRUE( tree.root->get_northeast()->get_southeast()->get_southeast()->is_leaf());
    ASSERT_TRUE( tree.root->get_southwest()->is_leaf());

    ASSERT_EQ( tree.get_height(), 3);
    ASSERT_EQ( tree.size(), 13);
    ASSERT_NEAR( tree.get_load_factor(),  0.03812, 1e-4);
}

TEST(QuadTreeTest, LoadGridFromJSON) {
    Tree tree;
    Terrain terrain(tree);

    EXPECT_DOUBLE_EQ( terrain.get_precision(), 1.);
    EXPECT_TRUE( default_bounds == terrain.get_bounds());

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
        EXPECT_EQ(tree.get_height(), 3);
        EXPECT_EQ(tree.size(), 41);
        EXPECT_FALSE(tree.root->is_leaf());

        // spot check #1: RT-NE-SW-quadrant
        const auto* r_ne_sw = tree.root->get_northeast()->get_southwest();
        ASSERT_TRUE(r_ne_sw->is_leaf());
        ASSERT_DOUBLE_EQ(r_ne_sw->get_value(), 0);

        // spot check #2: RT-NW-NE-NW quadrant
        const auto* r_ne_nw = tree.root->get_northwest()->get_northeast()->get_northwest();
        ASSERT_TRUE(r_ne_nw->is_leaf());
        ASSERT_DOUBLE_EQ(r_ne_nw->get_value(), 88);
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
    const Vector2d center(boundary_width/2, boundary_width/2);
    const Bounds expected_bounds(center, boundary_width);
    json source = { {"bounds", {{"x", center[0]}, {"y", center[1]}, {"width", boundary_width}}},
                    {"precision", desired_precision},
                    {"allow", {{{center[0] + diamond_width, center[1]},
                                {center[0]                , center[1] + diamond_width},
                                {center[0] - diamond_width, center[1]},
                                {center[0]                , center[1] - diamond_width}}}}};
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
        ASSERT_DOUBLE_EQ(bounds.center[0],    11);
        ASSERT_DOUBLE_EQ(bounds.center[1],    11);
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
    
TEST( QuadTreeTest, SearchExplicitTree) {
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

TEST( QuadTreeTest, SavePNG) {
    Terrain<Tree> terrain;
    const json source = generate_diamond(  16.,   // boundary_width
                                            8.,    // diamond_width,
                                            0.4);  // desired_precision);

    std::istringstream stream(source.dump());

    ASSERT_TRUE(terrain.load(stream));

    // // DEBUG
    // terrain.debug();

    EXPECT_TRUE(Bounds({8,8}, 16) == terrain.get_bounds());
    EXPECT_EQ( terrain.get_dimension(), 64);
    EXPECT_DOUBLE_EQ( terrain.get_precision(), 0.25);

    // DEVEL
    // ASSERT_TRUE(false) << " !! Note: this is unique to the QuadTree storage, and is visible in both debug and png output.\n";

    // // Because this manually tested, comment this block until needed:
    // const string filename("tree.test.png");
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
