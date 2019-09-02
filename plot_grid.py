#!/usr/local/bin/python3

from io import StringIO   # StringIO behaves like a file object
from matplotlib import cm
import matplotlib.pyplot as plt
import numpy as np

# grid = []
with open('grid.csv') as f:
    heights = np.loadtxt(f, delimiter=',')

xdim = heights.shape[0]
ydim = heights.shape[1]

show_2d = True
show_3d = False

fig = plt.figure()
plt.title('Priority Map')

if show_2d:
    # show height map in 2d
    p = plt.imshow(heights, cmap=cm.coolwarm)
    plt.colorbar(p)

# show height map in 3d
if show_3d:
    x = np.arange(-1, 1, (2/xdim))
    y = np.arange(-1, 1, (2/ydim))
    x,y = np.meshgrid(x,y)

    ax = fig.add_subplot(111, projection='3d')

    surf = ax.plot_surface(x, y, heights, cmap=cm.coolwarm, linewidth=0, antialiased=False)

    # Add a color bar which maps values to colors.
    fig.colorbar(surf)

plt.show()
