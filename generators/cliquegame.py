#!/usr/bin/env python3
"""Clique game generator in extended HOA format.

Reimplements pgsolver's ``cliquegame`` (src/lib/generators/cliquegame.ml).
Each node i has priority i, owner i % 2 and edges to every other node
(plus optional self-loop).
"""

import sys

from hoagen import emitHoa


def cliqueGame(n, with_self_cycles):
    prio = list(range(n))
    owner = [i % 2 for i in range(n)]
    succ = [[j for j in range(n) if with_self_cycles or j != i]
            for i in range(n)]
    names = [f"vertex{i}" for i in range(n)]
    emitHoa(n, prio, owner, succ, names)


def usage():
    print(f"{sys.argv[0]} requires 1 or 2 arguments: n [self]\n"
          "  n:    number of vertices (positive integer)\n"
          "  self: literal string 'self' to enable self-loops",
          file=sys.stderr)


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        usage()
        sys.exit(1)
    try:
        n = int(sys.argv[1])
    except ValueError:
        usage()
        sys.exit(1)
    if n <= 0:
        usage()
        sys.exit(1)
    with_self_cycles = False
    if len(sys.argv) == 3:
        if sys.argv[2] != "self":
            usage()
            sys.exit(1)
        with_self_cycles = True
    cliqueGame(n, with_self_cycles)
