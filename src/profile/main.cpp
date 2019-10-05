/// The MIT License 
// (c) 2019 Daniel Williams

#include <iostream>

using std::cerr;

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "geometry/point.hpp"
#include "geometry/bounds.hpp"
#include "grid/grid.hpp"
#include "quadtree/tree.hpp"
#include "terrain.hpp"

using terrain::geometry::Point;
using terrain::geometry::Bounds;
using terrain::grid::Grid;
using terrain::quadtree::Tree;
using terrain::Terrain;

constexpr double boundary_width = 64.;   // overall boundary
constexpr double diamond_width = 32.;
constexpr double desired_precision = 0.25;
// =====
constexpr Point center(boundary_width/2, boundary_width/2);
json source = { {"bounds", {{"x", center.x}, {"y", center.y}, {"width", boundary_width}}},
                {"precision", desired_precision},
                {"allow", {{{center.x + diamond_width, center.y},
                            {center.x                , center.y + diamond_width},
                            {center.x - diamond_width, center.y},
                            {center.x                , center.y - diamond_width}}}}};
std::istringstream document_stream(source.dump());


int main(int argc, char* argv[]){
    Tree source_tree;
    Terrain tree(source_tree);

    Grid source_grid;
    Terrain grid(source_grid);

    if(source.is_discarded()){
        cerr << ">>!! json input is malformed !!<<\n";
        cerr << source.dump(4);
        cerr << ';' << '\n';
        return -1;
    }else{
        if( ! grid.load(document_stream)){
            cerr << "!!!! error while loading into the grid!!!!\n";
            return -1;
        }
        document_stream.seekg(0);
        if( ! tree.load(document_stream)){
            cerr << "!!!! error while loading into the tree!!!!\n";
            return -1;
        }
    }

    // // DEBUG
    // grid.debug();
    // tree.debug();

    // DEVEL
    // cerr << " !! Note: this is unique to the QuadTree storage, and is visible in both debug and png output.\n";

    cerr << "##>> saving grid to .png...\n";
    const std::string grid_test_filename("tree.test.png");
    grid.png(grid_test_filename);

    cerr << "##>> saving tree to .png...\n";
    const std::string tree_test_filename("grid.test.png");
    tree.png(tree_test_filename);
    
    return 0;
}