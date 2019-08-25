#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "quadtree/tree.hpp"
#include "geometry/point.hpp"

using std::cout;
using std::endl;
using std::vector;

using geometry::Point;
using geometry::Bounds;

namespace quadtree {

TEST(TreeTest, ConstructDefault) {
    QuadTree tree;

    auto& bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(bounds.center.x,    0);
    ASSERT_DOUBLE_EQ(bounds.center.y,    0);
    ASSERT_DOUBLE_EQ(bounds.half_height, 512);
    ASSERT_DOUBLE_EQ(bounds.half_width,  512);
}

TEST( TreeTest, ConstructByCenterAndSize) {
    QuadTree tree({1,1}, 256);

    auto& bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(bounds.center.x,    1);
    ASSERT_DOUBLE_EQ(bounds.center.y,    1);
    ASSERT_DOUBLE_EQ(bounds.half_height, 128);
    ASSERT_DOUBLE_EQ(bounds.half_width,  128);

    // functional tests:
    // (1) in bounds
    ASSERT_TRUE(tree.contains(0,0));
    // (2) out of bounds in x
    ASSERT_FALSE(tree.contains(200,0));
    // (3) out of bounds in y
    ASSERT_FALSE(tree.contains(0,200));
    // (4) out of bounds in both x AND y
    ASSERT_FALSE(tree.contains(130,130));  
}

TEST( TreeTest, LoadMalformedSource){
    std::string document(R"({"bounds": {"x": 100, "y": 100, "width": 64}, )");

    std::istringstream source(document);
    
    QuadTree tree;
    auto & initial_bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(initial_bounds.center.x,    0);
    ASSERT_DOUBLE_EQ(initial_bounds.center.y,    0);
    ASSERT_DOUBLE_EQ(initial_bounds.half_height, 512);
    ASSERT_DOUBLE_EQ(initial_bounds.half_width,  512);

    tree.deserialize(source);

    // these tests should be *exactly* the same as before the 'load' call
    auto & loaded_bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(loaded_bounds.center.x,    0);
    ASSERT_DOUBLE_EQ(loaded_bounds.center.y,    0);
    ASSERT_DOUBLE_EQ(loaded_bounds.half_height, 512);
    ASSERT_DOUBLE_EQ(loaded_bounds.half_width,  512);
}

TEST( TreeTest, LoadValidSource){
    std::string document(R"({"bounds": {"x": 100, "y": 100, "width": 64},  "NE":{"NE":0.0, "NW": 0.0, "SE":0.0, "SW":0.0},"NW":0.0, "SE":0.0,"SW":0.0 })");
    std::istringstream source(document);
    
    QuadTree tree;

    auto & initial_bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(initial_bounds.center.x,    0);
    ASSERT_DOUBLE_EQ(initial_bounds.center.y,    0);
    ASSERT_DOUBLE_EQ(initial_bounds.half_height, 512);
    ASSERT_DOUBLE_EQ(initial_bounds.half_width,  512);

    tree.deserialize(source);

    // test bounds
    auto & loaded_bounds = tree.get_bounds();
    ASSERT_DOUBLE_EQ(loaded_bounds.center.x,    100);
    ASSERT_DOUBLE_EQ(loaded_bounds.center.y,    100);
    ASSERT_DOUBLE_EQ(loaded_bounds.half_height, 32);
    ASSERT_DOUBLE_EQ(loaded_bounds.half_width,  32);

    // test shape
    ASSERT_FALSE(tree.root->is_leaf());
    ASSERT_FALSE(tree.root->get_northeast()->is_leaf());
    ASSERT_TRUE(tree.root->get_southwest()->is_leaf());

    const auto& northeast_bounds = tree.root->get_northeast()->get_bounds();
    ASSERT_DOUBLE_EQ(northeast_bounds.center.x,    116);
    ASSERT_DOUBLE_EQ(northeast_bounds.center.y,    116);
    ASSERT_DOUBLE_EQ(northeast_bounds.half_height, 16);
    ASSERT_DOUBLE_EQ(northeast_bounds.half_width,  16);
}

TEST( TreeTest, WriteLoadCycle){

    QuadTree source_tree({11,11}, 2048);

    // modify tree in a characteristic way
    source_tree.root->split();
    source_tree.root->get_northeast()->split();
    source_tree.root->get_southwest()->split();
    
    // write tree #1 to the serialization buffer
    std::stringstream buffer;
    source_tree.serialize(buffer);

    // load contents into another tree
    QuadTree load_tree;
    buffer.seekg(0);
    load_tree.deserialize(buffer);
    
    // // DEBUG
    // source_tree.draw(cout);
    // load_tree.draw(cout);

    // test contents of test_tree
    { // test bounds:
        auto & bounds = load_tree.get_bounds();
        ASSERT_DOUBLE_EQ(bounds.center.x,    11);
        ASSERT_DOUBLE_EQ(bounds.center.y,    11);
        ASSERT_DOUBLE_EQ(bounds.half_height, 1024);
        ASSERT_DOUBLE_EQ(bounds.half_width,  1024);
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
    
TEST( TreeTest, TestSearchExplicitTree) {
    QuadTree tree({1,1}, 256);

    node_value_t false_value = 5;
    node_value_t true_value = 14;
    node_value_t default_value = -99;

    // .... Out Of Bounds:
    ASSERT_EQ(tree.search(150, 150, default_value), default_value);

    // Set Quadrant I:
    tree.root->get_northeast()->set_value(true_value);
    // Set Quadrdant II:
    tree.root->get_northwest()->set_value(false_value);
    // Set Quadrant III:
    tree.root->get_southwest()->set_value(true_value);
    // Set Quadrant IV:
    tree.root->get_southeast()->set_value(false_value);

    // functional tests:
    // .... Quadrant I:
    ASSERT_EQ(tree.search( 50,  50, -1), true_value);
    // .... Quadrant II:
    ASSERT_EQ(tree.search(-50,  50, -1), false_value);
    // .... Quadrant III:
    ASSERT_EQ(tree.search(-50, -50, -1), true_value);
    // .... Quadrant IV:
    ASSERT_EQ(tree.search( 50, -50, -1), false_value);
    
    // .... Out Of Bounds:
    ASSERT_TRUE(tree.search(150, 150, -1) == -1);
}

TEST( TreeTest, TestSearchImplicitTree){
    QuadTree tree({1,1}, 256);

    node_value_t false_value = 5;
    node_value_t true_value = 14;
    node_value_t default_value = -99;
    
    // .... Out Of Bounds:
    ASSERT_EQ(tree.search(150, 150, default_value), default_value);

    // const vector<Point> input_polygon{
    //     {232000, 806410},
    //     {232400, 820494},
    //     {243600, 842644},
    //     {220900, 874092},
    //     {221700, 906633},
    //     {238400, 960000},
    //     {247000, 960000},
    //     {378000, 960000},
    //     {378000, 762800},
    //     {232000, 762800},
    //     {232000, 806410}};
    // ASSERT_EQ(input_polygon.size(), 11);

    // tree.load(input_polygon);
    // { // test bounds:
    //     const auto & bounds = tree.get_bounds();
    //     ASSERT_DOUBLE_EQ(bounds.center.x,    491850);
    //     ASSERT_DOUBLE_EQ(bounds.center.y,    669000);
    //     ASSERT_DOUBLE_EQ(bounds.half_height, 291000);
    //     ASSERT_DOUBLE_EQ(bounds.half_width,  291000);
    // }

    // Set Quadrant I:
//    tree.set_value(10, 10, true_value);
    // Set Quadrdant II:
//    tree.root->get_northwest()->set_value(false_value);
    // Set Quadrant III:
//    tree.root->get_southwest()->set_value(true_value);
    // Set Quadrant IV:
//    tree.root->get_southeast()->set_value(false_value);

    // functional tests:
    // .... Quadrant I:
//    ASSERT_EQ(tree.search( 50,  50, -1), true_value);
    // .... Quadrant II:
//    ASSERT_EQ(tree.search(-50,  50, -1), false_value);
    // .... Quadrant III:
//    ASSERT_EQ(tree.search(-50, -50, -1), true_value);
    // .... Quadrant IV:
//    ASSERT_EQ(tree.search( 50, -50, -1), false_value);
    
}


} // namespace quadtree
