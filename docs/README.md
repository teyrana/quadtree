# Motivation

So there I was enjoying my coffee, dear reader, when I started to wonder how robots would *actually* track the space around us.

(note: This article won't go into any of the algorithms for motion planning itself.  This is an exploration of how to build the underlying infrastructure for those techniques.  It also assumes that a perfect information source exists, with which to populate our planning space.)

Ideally, you have a nice, rectangular room, with four walls.  In a perfect world, all of our rooms are rectangular! Very easy to think about:

```
+-----------------------------------+
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
+-----------------------------------+
```

Of course, if you're a human, your rooms are probably more complicated.   So it probably looks something like this:

```
                      +------------------------------------------------+
                      |      ______________________                    |
           <door>     |     /_______|_______|______\  +------+         |
+---------=========---+    | |      |       |     | | |      |         |
|          \ . . .         |_|      |       |     |_| |      |         |
|           \ . .            +------^-------^------+  +------+         |
|            \.                                                        |
|                                                                      |
|+---+                      +-------------------+                      |
|| . |                      |                   |                      |
|| O |                      +-------------------+                      |
|| . |                                                                 |
|| . |                                                                 |
|| O |                     _ ------.------.------_                     |  
|| . |                    | |      |      |      | |                   |
|+---+                    | |______|______|______| |                   |
|                          \______________________/                    |
+----------------------------------------------------------------------+

````

And while this ASCII art looks ugly enough to a human, all a robot wants to know is "where can I can go?". So let's simplify!

```
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXX      XXXXXXXXXXXXXXXXXXXXXXX                   X
XXXXXXXXXXXXXXXXXXXXXXX     XXXXXXXXXXXXXXXXXXXXXXXXX XXXXXXXX         X
XXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         X
X          X               XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         X
X           X                XXXXXXXXXXXXXXXXXXXXXXX  XXXXXXXX         X
X            X                                                         X
X                                                                      X
XXXXXX                      XXXXXXXXXXXXXXXXXXXXX                      X
XXXXXX                      XXXXXXXXXXXXXXXXXXXXX                      X
XXXXXX                      XXXXXXXXXXXXXXXXXXXXX                      X
XXXXXX                                                                 X
XXXXXX                                                                 X
XXXXXX                       XXXXXXXXXXXXXXXXXXXX                      X
XXXXXX                    XXXXXXXXXXXXXXXXXXXXXXXXXX                   X
XXXXXX                    XXXXXXXXXXXXXXXXXXXXXXXXXX                   X
X                           XXXXXXXXXXXXXXXXXXXXXXXX                   X
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

There we go -- any location with an `X` is blocked, and anything else is allowed.  That's all our little robot needs to know to navigate this tiny room, especially if the resolution, above is accurate enough!

But wait... what if there are more rooms? Suppose this is one room in a 500 room hotel?  Accuracy and scaling are both concerns here.  More on this later.

## Procedure

To start, we will compare two implementations.  Each tracks a single
byte of data per location.

### (Option #1) Simple 2d-array

Simple data array, indexed in the x and y axes.  The index for a given coordinate is purely a function of the x,y measurements:  `(i,j) = f(x,y)`.  The value at every `i,j` is stored in memory.

The space complexity of the array is obviously `O(w^2)`.     (And in the real-world, the overhead is 1 byte per grid cell -- very simple and small.)

The node count in a fully-populated quadtree is: `(4^(log(w^2)/log(4) + 1) - 1)/3`.  This is multiplied by the size of
each node, which is implementation dependent.  (In my first version, this was 64 bytes.  More on this later)


### (Option #2) Quadtree

[Quadtree on Wikipedia](https://en.wikipedia.org/wiki/Quadtree) 
Our implementation is a region quadtree, with the caveat that each leaf node may represent a real-world volume of space: a 1-meter by 1-meter square, for example.  That square may represent an arbritrary number source input points, and discards the exact input. The design intent is to represent the underlying terrain, not the sensor points; in other words, represent the wall, not the sensor returns from the wall.  Therefore when searching this quadtree, the first leaf node encountered is returned.

The space complexity of a fully-loaded tree is `O(4 ^ (log(w^2) )`, where m is the branch factor -- i.e. 4 for our quadtree.  In other
words, exponential.  So the total memory usage for a quadtree is never going to be better than an array.

Possible wins only happen when the underlying structure can be compressed to use less memory than the grid.

Two major factors affect this:
- size / precision of the area
- complexity of the underlying terrain -> i.e. compression factor of the quadtree
- sizeof each node

## Hypothesis

At small scales, the grid will be the faster search method.  At larger scales, the quadtree will be faster, due to its higher compression ratio.

## Results

| Dimension:     |    8192         |    4096           |     512          |
|:---------------|:----------------|:------------------|:-----------------|
|*Grid*          |
|Loading Factor: |       1.        |       1.          |       1.         |
|Load time:      |       0.8 sec   |       0.221 sec   |       0.0003 sec |
|Memory:         |      67.1 Mb    |      16.7 Mb      |     262 kb       |
|100k searches:  |      10.7 ms    |      10.7 ms      |      10.7 ms     |
| *Tree*         |                 |
|Loading Factor: |       0.000366  |       0.00073     |        0.0015    |
|Loaded in:      |      25.883 s   |       6.5 s       |        0.107 s   |
|Memory:         |       2.0 Mb    |       1.0 Mb      |      131 kb      |
|100k searches:  |      15.9 ms    |      16.1 ms      |       16.0 ms    |

## Discussion

For a byte-wide storage cell, the grid data structure is clearly the
better choice.

The Quadtree fails to provide any speedup, even for a highly efficient
quad-tree (i.e. one with a low loading factor) which should be the
best-case scenario, because the tree (in theory) provides a higher
degree of compression, and definitely uses less memory .... overall.

Obviously, the grid still has an edge in execution performance complexity:
- Grid lookup is `O(1)`
- Quadtree lookup is `O(height) ~= O(log4(grid_size))` for the quadtree.  

In the real world, lookup speed still depends on cache misses.

OTOH, the memory savings are certainly clear.  A 30x improvement in memory usage is nothing to ignore !


# Step 2: Maintainability: Convert to Eigen Library

Load times INCREASED.  :(

|                    |    *Grid*         |   *Tree*      |
|:-------------------|:------------------|:--------------|
| Dimension:         |    4096           |   4096        |
|Loading Factor:     |       1.          |      0.00073  |
|Load time (sec)     |       6.6         |     42.6      |
|Memory (Mb):        |      16.7         |      1.0      |
|100k searches (ms): |      


(probbaly because node size went up?) ==> this explains the tree, but not the grid.

# Phase 3:
Reduce Node Size:


|                    |    *Grid*         |   *Tree*      |
|:-------------------|:------------------|:--------------|
| Dimension:         |    4096           |   4096        |
|Loading Factor:     |       1.          |      0.00036  |
|Load time (sec)     |       6.8         |     91.01     |
|Memory (Mb):        |      16.7         |      1.3      |
|100k searches (ms): |      95.7 ms      |    264        |