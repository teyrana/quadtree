#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>

#include "include/quad_tree.hpp"

using quadtree::QuadTree;

int main(int argc, const char *argv[])
{
    QuadTree tree;

    std::string filename("test/trees/simple.json");
    std::fstream infile(filename);
  
    printf("==== QuadTree Init: ==== \n");
    tree.deserialize(infile);
    tree.serialize(std::cout);

    printf("\n");

    return 0;
}
