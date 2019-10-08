/// The MIT License 
// (c) 2019 Daniel Williams

#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

#include <sys/stat.h> // may not be standard

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

constexpr double boundary_width = 4096.;   // overall boundary
constexpr double diamond_width =  1024.;
constexpr double desired_precision = 1.;
// =====
constexpr Point center(boundary_width/2, boundary_width/2);
json source = { {"bounds", {{"x", center.x}, {"y", center.y}, {"width", boundary_width}}},
                {"precision", desired_precision},
                {"allow", {{{center.x + diamond_width, center.y},
                            {center.x                , center.y + diamond_width},
                            {center.x - diamond_width, center.y},
                            {center.x                , center.y - diamond_width}}}}};

constexpr size_t test_seed = 55;
static std::mt19937 generator;

template <typename T>
void profile_terrain(T& terrain, const size_t iteration_limit){
    const double max = terrain.get_bounds().get_x_max();
    const double min = terrain.get_bounds().get_x_min();

    generator.seed(test_seed);

    std::cout << ">> Starting testing:\n";
    cell_value_t dump = 0;
    
    auto start = std::chrono::high_resolution_clock::now(); 
    size_t iteration = 0;
    for( iteration = 0; iteration < iteration_limit; ++iteration ){
        const double x = static_cast<double>(generator()) / generator.max() * max + min;

        const double y = static_cast<double>(generator()) / generator.max() * max + min;
        //std::cout << "random value: x: " << x << ", " << y << '\n';

        dump = terrain.search({x,y});
        //std::cout << "             =>: " << static_cast<int>(value) << '\n';
    }
    auto stop = std::chrono::high_resolution_clock::now(); 

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count(); 
  
    std::cout << "<< Finished testing:\n";
    std::cout << "   Ran " << iteration << " iterations in " << duration << " \u03BCs \n\n";
}


int main(int argc, char* argv[]){
    std::string filename;
    std::unique_ptr<std::istream> document_stream;
    size_t trial_size = 10;
    
    if( 1 == argc ){
        // load default json source data
        filename = "<default>";
        document_stream.reset(new std::istringstream(source.dump()));
    }else if( 1 < argc ){
        filename = argv[1];
        cerr << "    ## Selected file:       " << filename << endl;

        struct stat buf;
        if( stat(filename.c_str(), &buf) == -1){
            cerr << "!? could not find file!\n";
            return -1;
        }
        std::ifstream document_stream = std::ifstream(filename);

        if( 2 < argc ){
            filename = argv[1];
            trial_size = atoi(argv[2]);
            cerr << "    ## Selected Trial Size: " << trial_size << endl;
        }
    }

    { // Profiling Grid
        Terrain<Grid> grid;

        if( ! grid.load(*document_stream)){
            cerr << "!!!! error while loading into the grid!!!!\n";
            cerr << grid.get_error() << endl;
            return -1;
        }

        cerr << "?? loaded grid." << endl;
        
        // // DEBUG
        // grid.debug();
        cerr << "====== Grid Stats: ======\n";
        cerr << "##  bounds:     " << grid.get_bounds().str() << endl;
        cerr << "##  precision:  " << grid.get_precision() << endl;
        cerr << "##  dimension:  " << grid.get_dimension() << endl;
        cerr << endl;
        
        profile_terrain(grid, trial_size);

        // cerr << "##>> saving grid to .png...\n";
        // const std::string grid_test_filename("tree.test.png");
        // grid.png(grid_test_filename);
    }

    document_stream->seekg(0);
    { // Profiling the quadtree
        Terrain<Tree> tree;

        if( ! tree.load(*document_stream)){
            cerr << "!!!! error while loading into the tree!!!!\n";
            cerr << tree.get_error();
            return -1;
        }


        // tree.debug();
        cerr << "====== Tree Stats: ======\n";
        cerr << "##  bounds:     " << tree.get_bounds().str() << endl;
        // BROKEN! TODO: FIXME!!
        //cerr << "##  loading:    " << tree.impl.get_load_factor() << endl;
        cerr << "##  precision:  " << tree.get_precision() << endl;
        cerr << "##  dimension:  " << tree.get_dimension() << endl;
        cerr << endl;

        profile_terrain(tree, trial_size);

        // cerr << "##>> saving tree to .png...\n";
        // const std::string tree_test_filename("grid.test.png");
        // tree.png(tree_test_filename);
    }
      
    return 0;
}
