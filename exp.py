#!/usr/bin/python3

import numpy as np

xs = np.linspace(-1, 1, 100)
ys = np.exp(xs)
ys = ys / max(ys)

with open("exp.csv", "w") as file:
    for x, y in zip(xs, ys):
        file.write(f"{x}, {y}, 0\n")
