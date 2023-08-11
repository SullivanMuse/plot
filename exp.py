#!/usr/bin/python3

import numpy as np

xs = np.linspace(-1, 1, 1000)
ys = np.exp(xs)

with open("exp.csv", "w") as file:
    for x, y in zip(xs, ys):
        file.write(f"{x}, {y}, 0\n")
