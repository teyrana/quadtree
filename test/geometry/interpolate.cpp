#include <cmath>

#include <gtest/gtest.h>

#include "geometry/interpolate.hpp"

using std::isnan;

namespace terrain::geometry {

TEST( InterpolateTest, InterpolateValueBetween) {
    const Sample s1 = {{ 0.0,  0.0}, 0};
    const Sample s2 = {{10.0, 10.0}, 50};
    
    ASSERT_EQ( interpolate_linear({-1, -1}, s1, s2),  0);
    ASSERT_EQ( interpolate_linear({ 0,  0}, s1, s2),  0);
    ASSERT_EQ( interpolate_linear({ 1,  1}, s1, s2),  5);
    ASSERT_EQ( interpolate_linear({ 2,  2}, s1, s2), 10);
    ASSERT_EQ( interpolate_linear({ 3,  3}, s1, s2), 15);
    ASSERT_EQ( interpolate_linear({ 4,  4}, s1, s2), 20);
    ASSERT_EQ( interpolate_linear({ 5,  5}, s1, s2), 25);
    ASSERT_EQ( interpolate_linear({ 6,  6}, s1, s2), 30);
    ASSERT_EQ( interpolate_linear({ 7,  7}, s1, s2), 35);
    ASSERT_EQ( interpolate_linear({ 8,  8}, s1, s2), 40);
    ASSERT_EQ( interpolate_linear({ 9,  9}, s1, s2), 45);
    ASSERT_EQ( interpolate_linear({10, 10}, s1, s2), 50);
    ASSERT_EQ( interpolate_linear({11, 11}, s1, s2), 50);
}

TEST( InterpolateTest, InterpolateValueOffset) {
    const Sample s1 = {{-15.0, -15.0}, 100};
    const Sample s2 = {{ 17.0, -15.0},  50};

    ASSERT_EQ( interpolate_linear({-15.1, -15.0}, s1, s2), 100);
    ASSERT_EQ( interpolate_linear({-15.0, -15.0}, s1, s2), 100);
    ASSERT_EQ( interpolate_linear({-14.9, -15.0}, s1, s2), 100);
    ASSERT_EQ( interpolate_linear({-14.8, -15.0}, s1, s2), 100);
    ASSERT_EQ( interpolate_linear({-14.0, -15.0}, s1, s2),  98);
    ASSERT_EQ( interpolate_linear({-10.0, -15.0}, s1, s2),  92);
    ASSERT_EQ( interpolate_linear({ -5.0, -15.0}, s1, s2),  84);
    ASSERT_EQ( interpolate_linear({  0.0, -15.0}, s1, s2),  77);
    ASSERT_EQ( interpolate_linear({  1.0, -15.0}, s1, s2),  75);
    ASSERT_EQ( interpolate_linear({  2.0, -15.0}, s1, s2),  73);
    ASSERT_EQ( interpolate_linear({  5.0, -15.0}, s1, s2),  69);
    ASSERT_EQ( interpolate_linear({ 10.0, -15.0}, s1, s2),  61);
    ASSERT_EQ( interpolate_linear({ 11.0, -15.0}, s1, s2),  59);
    ASSERT_EQ( interpolate_linear({ 12.0, -15.0}, s1, s2),  58);
    ASSERT_EQ( interpolate_linear({ 13.0, -15.0}, s1, s2),  56);
    ASSERT_EQ( interpolate_linear({ 14.0, -15.0}, s1, s2),  55);
    ASSERT_EQ( interpolate_linear({ 15.0, -15.0}, s1, s2),  53);
    ASSERT_EQ( interpolate_linear({ 16.0, -15.0}, s1, s2),  52);
    ASSERT_EQ( interpolate_linear({ 16.9, -15.0}, s1, s2),  50);
    ASSERT_EQ( interpolate_linear({ 17.0, -15.0}, s1, s2),  50);
    ASSERT_EQ( interpolate_linear({ 17.1, -15.0}, s1, s2),  50);
}

TEST( InterpolateTest, InterpolateQuad){
    // Set Quadrant I:
    const Sample ne = {{ 10, 10}, 100};
    // Set Quadrdant II:
    const Sample nw = {{  0, 10},  50};
    // Set Quadrant III:
    const Sample sw = {{  0,  0},   0};
    // Set Quadrant IV:
    const Sample se = {{ 10,  0},  50};

    // main sequence:
    // ASSERT_EQ( interpolate_bilinear( { 26,   4}, ne, nw, se, sw), cell_default_value);  // Start out of bounds
    // ASSERT_EQ( interpolate_bilinear( { 25.1, 4}, ne, nw, se, sw), cell_default_value);

    ASSERT_EQ( interpolate_bilinear( { 10,   1}, ne, nw, sw, se),   55);  // border of tree
    ASSERT_EQ( interpolate_bilinear( {  9.9, 1}, ne, nw, sw, se),   55);
    ASSERT_EQ( interpolate_bilinear( {  9,   1}, ne, nw, sw, se),   50);
    ASSERT_EQ( interpolate_bilinear( {  8,   1}, ne, nw, sw, se),   45);
    ASSERT_EQ( interpolate_bilinear( {  7,   1}, ne, nw, sw, se),   40);
    ASSERT_EQ( interpolate_bilinear( {  6,   1}, ne, nw, sw, se),   35);
    ASSERT_EQ( interpolate_bilinear( {  5,   1}, ne, nw, sw, se),   30);  // midpoint
    ASSERT_EQ( interpolate_bilinear( {  4,   1}, ne, nw, sw, se),   25);
    ASSERT_EQ( interpolate_bilinear( {  3,   1}, ne, nw, sw, se),   20);
    ASSERT_EQ( interpolate_bilinear( {  2,   1}, ne, nw, sw, se),   15);
    ASSERT_EQ( interpolate_bilinear( {  1,   1}, ne, nw, sw, se),   10);
    ASSERT_EQ( interpolate_bilinear( {  0.1, 1}, ne, nw, sw, se),    6);
    ASSERT_EQ( interpolate_bilinear( {  0.0, 1}, ne, nw, sw, se),    5); // border of tree

    // ASSERT_EQ( interpolate_bilinear( { -0.1, 4}, ne, nw, se, sw),   20);
    // ASSERT_EQ( interpolate_bilinear( { -1,   4}, ne, nw, se, sw), cell_default_value);
    // ASSERT_EQ( interpolate_bilinear( { -2,   4}, ne, nw, se, sw), cell_default_value);

    // shuffle the order, and see if that affects anything:
    ASSERT_EQ( interpolate_bilinear( {  8,  8}, ne, nw, sw, se),   80);  // control case: correct order
    {//  swap east <-> west: same answer
	ASSERT_EQ( interpolate_bilinear( {  8,  8}, nw, ne, se, sw),   80);
    }{ // swap north <-> south: same anwser
	ASSERT_EQ( interpolate_bilinear(  {  8,  8}, se, sw, ne, nw),   80);
    }{ // swap vertical AND horizontal: ? 
	ASSERT_EQ( interpolate_bilinear(  {  8,  8}, sw, se, ne, nw),   80);
    }
}

} // namespace terrain::geometry
