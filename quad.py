#!/usr/bin/python3

import numpy as np

xs = np.linspace(-1, 1, 100)
ys = xs**2

xs = np.round(xs, 5)
ys = np.round(ys, 5)

with open("quad.csv", "w") as file:
    for x, y in zip(xs, ys):
        file.write(f"{x}, {y}, 0\n")
