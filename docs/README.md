Motivation:
-----------
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
| +---+                      +-------------------+                     |
| | O |                      |                   |                      |
| |   |                      +-------------------+                     |
| | O |                                                                |
| |   |                     _ ------.------.------_                    |  
| | O |                    | |      |      |      | |                  |
| +---+                    | |______|______|______| |                  |
|                           \______________________/                   |
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
XXXXXXX                      XXXXXXXXXXXXXXXXXXXXX                     X
XXXXXXX                      XXXXXXXXXXXXXXXXXXXXX                     X
XXXXXXX                      XXXXXXXXXXXXXXXXXXXXX                     X
XXXXXXX                                                                X
XXXXXXX                       XXXXXXXXXXXXXXXXXXXX                     X
XXXXXXX                    XXXXXXXXXXXXXXXXXXXXXXXXXX                  X
XXXXXXX                    XXXXXXXXXXXXXXXXXXXXXXXXXX                  X
X                           XXXXXXXXXXXXXXXXXXXXXXXX                   X
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```





