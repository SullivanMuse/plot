#!/usr/bin/python3

import numpy as np

xs = np.linspace(-1, 1, 1000)
ys = xs**2

with open("quad.csv", "w") as file:
    for x, y in zip(xs, ys):
        file.write(f"{x}, {y}, 0\n")
