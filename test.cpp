#include <assert.h>
#include <stdio.h>

#include "include/quad_tree.hpp"

using quadtree::QuadTree;

int main(int argc, const char *argv[])
{
    QuadTree tree;
    
    printf("==== QuadTree Init: ==== \n");
    printf("%s\n", tree.serialize().c_str());

}
