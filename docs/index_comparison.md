# Index Comparisons

## Grid

### Procedure

For this A/B test, the only variable was changing the addressing method from row-major addressing to geohash / z-curve addressing.

### Discussion

For the grid structure, clearly the simplicity of row-major indexing is a modest win. It averages about 5% faster, but in practice, the trials may have overlaping durations.

|                     |    *Grid*      |   *Grid*     |
|                     | Row-Major-Hash |    Z-Hash    |
|:--------------------|:---------------|:-------------|
| Dimension:          |   4096         |   4096       |
| Load time (sec)     |      0.091     |      0.128   |
| 100k searches (ms): |      8.091     |      9.741   |
| 1M Searches (ms):   |    486         |    510       |


## QuadTree

### Procedure

For this A/B test, the first change was to go from indexing by double-comparisons to Z-Order Curve index.

### Discussion

For the grid structure, clearly the simplicity of row-major indexing is a clear win. It is reliably 40% faster.

|                     |  *Reference Tree*   |    *Z-Space*    |
|                     | (double-comparison) | (Z-Order Curve) |
|:--------------------|:--------------------|:----------------|
| Dimension:          |     4096            |   4096          |
| Load time (sec)     |        3.27         |      3.17       |
| 1M searches (ms):   |       70.813        |     80.9        |
