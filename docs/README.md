Motivation:
===========
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


Implementation:
===============
To start, we will compare two implementations to keep track of navigable space:

### (Reference) Simple 2d-array

Simple data array, indexed in the x and y axes.  The index for a given coordinate is purely a function of the x,y measurements:  `(i,j) = f(x,y)`.  The value at every `i,j` is stored in memory.

### (Option #2) Quadtree

[Quadtree on Wikipedia](https://en.wikipedia.org/wiki/Quadtree) 
Our implementation is a region quadtree, with the caveat that each leaf node may represent a real-world volume of space: a 1-meter by 1-meter square, for example.  That square may represent an arbritrary number source input points, and discards the exact input. The design intent is to represent the underlying terrain, not the sensor points; in other words, represent the wall, not the sensor returns from the wall.  Therefore when searching this quadtree, the first leaf node encountered is returned.


Hypothesis:
===========
At small scales, the grid will be the faster search method.  At larger scales, the quadtree will be faster, due to its higher compression ratio.
