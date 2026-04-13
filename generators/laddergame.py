#!/usr/bin/env python3
"""Ladder game generator in extended HOA format.

Reimplements pgsolver's ``laddergame`` (src/lib/generators/laddergame.ml).
Builds a cycle on m = 2n vertices; vertex v has priority v % 2, owner
v % 2 (plr_benefits) and successors (v+1) % m and (v+2) % m.
"""

import sys

from hoagen import emitHoa


def ladderGame(n):
    m = 2 * n
    prio = [v % 2 for v in range(m)]
    owner = [v % 2 for v in range(m)]
    succ = [[(v + 1) % m, (v + 2) % m] for v in range(m)]
    names = [f"vertex{v}" for v in range(m)]
    emitHoa(m, prio, owner, succ, names)


def usage():
    print(f"{sys.argv[0]} requires one argument: n\n"
          "  n: positive integer; produces a ladder game with 2n vertices",
          file=sys.stderr)


if __name__ == "__main__":
    if len(sys.argv) != 2:
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
    ladderGame(n)
