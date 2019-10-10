QuadTree
===============================================================

This project is primarily a demo and learning project; implementing a quadtree data structure: QuadTree [1].  Specifically, to provide a performant lookup into x,y and return whether a location is allowable, or not.

Purpose
-------

While quadtrees have a wide variety of uses, this implementation is designed to partition space into allowable area, and blocked area.  (Both may be convex, concave, or disjoint.)  The allowable nature of the space is tracked on a square-area basis (in fact, the include reference implementation is a grid.)

For instance, imagine a robot in a room; in order to locate the robot in the room, and the boundaries of the room. 


Build
-----
This project uses CMake as its primary build system. `build.sh` in the base directory should automatically build all executables.

### Dependencies
This project makes use of several libraries:
- Eigen Math / Linear Algebra Library[7] - http://eigen.tuxfamily.org/index.php?title=Main_Page
- nlohmann/json [4] - header-only json I/O libray.  Vendored at `vendor/nlohmann/json`.
- libpng [5] - [optional] Used to output the contents of a quadtree or grid as an image (grayscale height map).
- Google Test Framework [6]- used to run all of the development tests

References
----------
- [1] [Quadtree (Wikipedia)]( http://en.wikipedia.org/wiki/Quadtree )
- [2] [varunpant/Quadtree (Java Implementation)]( https://github.com/varunpant/Quadtree )
- [3] [Jefflarson/quadtree (C Implementation)]( https://github.com/thejefflarson/quadtree )
- [4] [nlohmann/json](https://github.com/nlohmann/json)
- [5] [libpng](http://www.libpng.org/)
- [6] [Google Test Framework](https://github.com/google/googletest)
- [7] [Eigen library](http://eigen.tuxfamily.org/index.php?title=Main_Page)


