/// The MIT License 
// (c) 2019 Daniel Williams

#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

#include <sys/stat.h> // may not be standard

using std::cerr;

#include <cxxopts/cxxopts.hpp>

#include <Eigen/Geometry>

#include <nlohmann/json/json.hpp>
using nlohmann::json;

#include "grid/grid.hpp"
#include "quadtree/tree.hpp"
#include "terrain.hpp"

using terrain::grid::Grid;
using terrain::quadtree::Tree;
using terrain::Terrain;

constexpr double boundary_width = 4096.;
constexpr double desired_precision = 1.0;
// =====
const json source = terrain::generate_diamond(  boundary_width, desired_precision);

constexpr size_t test_seed = 55;
static std::mt19937 generator;


cxxopts::Options command_line_options("TerrainProfile", "Run metrics and speed tests of the terrain library");

void configure_parser_options(cxxopts::Options& opts){
    // define command line options
    opts.add_options()
        // ("d,debug", "Enable debugging")
        ("i,input", "input json path", cxxopts::value<std::string>())
        ("c,count", "Iteration Count", cxxopts::value<size_t>())
        ("tree", "Use the QuadTree backend", cxxopts::value<bool>())
        ("grid", "Use the Grid backend", cxxopts::value<bool>())
        ("o,image", "Output image filen name", cxxopts::value<std::string>())
    ;
}

template <typename T>
void profile_terrain(T& terrain, const size_t iteration_limit){
    const double max = terrain.get_layout().get_x_max();
    const double min = terrain.get_layout().get_x_min();

    generator.seed(test_seed);

    std::cout << ">> Starting testing:\n";
    cell_value_t dump = 0;
    
    auto start = std::chrono::high_resolution_clock::now(); 
    size_t iteration = 0;
    for( iteration = 0; iteration < iteration_limit; ++iteration ){
        const double x = static_cast<double>(generator()) / generator.max() * max + min;

        const double y = static_cast<double>(generator()) / generator.max() * max + min;
        //std::cout << "random value: x: " << x << ", " << y << '\n';

        dump = terrain.classify({x,y});
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
    
    configure_parser_options(command_line_options);
    auto result = command_line_options.parse(argc, argv);

    // configure input:
    if( 0 == result.count("input")){
        cerr << "  ## using default input.\n";
        // load default json source data
        filename = "<default>";
        document_stream.reset(new std::istringstream(source.dump()));
    }else if( 0 < result.count("input")){
        filename = result["opt"].as<std::string>();
        cerr << "  ## File input; with:  " << filename << endl;

        struct stat buf;
        if( stat(filename.c_str(), &buf) == -1){
            cerr << "!? could not find file!\n";
            return -1;
        }
        std::ifstream document_stream = std::ifstream(filename);
    }

    if( 0 < result.count("count")){
        trial_size = result["count"].as<size_t>();
        cerr << "    ## Selected Trial Size: " << trial_size << endl;
    }

    
    bool use_grid = false;
    Terrain<Grid> grid;
    Terrain<Tree> tree;
    if( 0 < result.count("grid")){
        use_grid = true;
        cerr << "  ## Using grid.\n";
    }else if( 0 < result.count("tree")){
        use_grid = false;
        cerr << "  ## Using tree.\n";
    }else{
        use_grid = false;
        cerr << "  ## default to tree.\n";
    }

    bool write_output = false;
    std::string output_path;
    if( 0 < result.count("image")){
        write_output = true;
        output_path = result["image"].as<std::string>();
        if( output_path.empty() ){
            output_path = "terrain.output.png";
        }
        cerr << "  ## Save output to: " << output_path << '\n';
    }

    // ^^^^ Configuration
    // vvvv Execution

    cerr << ">> loading terrain ... \n";
    const auto start_load = std::chrono::high_resolution_clock::now(); 
    
    bool load_success;
    if(use_grid){
        load_success = grid.load_from_json_stream(*document_stream);
    }else{
        load_success = tree.load_from_json_stream(*document_stream);
    }
    if(!load_success){
        cerr << "!!!! error while loading into the tree!!!!\n";
        cerr << tree.get_error();
        return -1;
    }
    const auto finish_load = std::chrono::high_resolution_clock::now(); 
    const auto load_duration = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(finish_load - start_load).count())/1000;
    cerr << "<< Loaded in:   " << load_duration << " s \n";

    if(use_grid){
        // grid.debug();
        cerr << grid.summary();
        profile_terrain(grid, trial_size);
    }else{
        // tree.debug();
        cerr << tree.summary();
        profile_terrain(tree, trial_size);
    }

    if(write_output){
        cerr << "##>> writing output...\n";
        if(use_grid){
            grid.to_png(output_path);
        }else{
            tree.to_png(output_path);
        }
    }

      
    return 0;
}
