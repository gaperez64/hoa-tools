#!/usr/bin/env python3
"""Steady game generator in extended HOA format.

Reimplements pgsolver's ``steadygame`` (src/lib/generators/steadygame.ml).
Produces a random parity game with controlled in/out-degree bounds.

Vertex i has priority i; the owner is chosen uniformly at random.
Successor lists are built by repeatedly matching a random source (still
needing out-edges) to a random target (still able to receive in-edges),
avoiding self-loops and duplicate edges where possible. Matches the
pgsolver behaviour including its quirk of allowing a duplicate/self-loop
when only one target remains.

CLI: steadygame.py n l h x y [seed]
"""

import random
import sys

from hoagen import emitHoa


def steadyGame(size, outdegmin, outdegmax, indegmin, indegmax, seed=None):
    if seed is not None:
        random.seed(seed)

    outavail = list(range(size))
    inavail = list(range(size))
    outtodo = size
    intodo = size
    pg = [[] for _ in range(size)]
    pgtr = [0] * size

    while ((outtodo > 0 and len(inavail) > 0)
           or (intodo > 0 and len(outavail) > 0)):
        ii = random.randrange(len(outavail))
        i = outavail[ii]
        forb = set(pg[i])
        forb.add(i)
        while True:
            jj = random.randrange(len(inavail))
            if inavail[jj] not in forb or len(inavail) == 1:
                break
        j = inavail[jj]
        pg[i].append(j)
        pgtr[j] += 1
        leni = len(pg[i])
        lenj = pgtr[j]
        if leni == outdegmin:
            outtodo -= 1
        elif leni == outdegmax:
            outavail.pop(ii)
        if lenj == indegmin:
            intodo -= 1
        elif lenj == indegmax:
            inavail.pop(jj)

    prio = list(range(size))
    owner = [random.randint(0, 1) for _ in range(size)]
    names = [f"vertex{i}" for i in range(size)]

    for i in range(size):
        if not pg[i]:
            print(f"Warning: vertex {i} has no successors; output is not a "
                  "valid parity game. Increase degree bounds or n.",
                  file=sys.stderr)
            sys.exit(1)

    emitHoa(size, prio, owner, pg, names)


def usage():
    print(f"{sys.argv[0]} requires 5 or 6 arguments: n l h x y [seed]\n"
          "  n:    number of vertices (positive integer)\n"
          "  l:    lowest possible out-degree (>= 1)\n"
          "  h:    highest possible out-degree (>= l)\n"
          "  x:    lowest possible in-degree (>= 1)\n"
          "  y:    highest possible in-degree (>= x)\n"
          "  seed: optional integer seed for reproducibility",
          file=sys.stderr)


if __name__ == "__main__":
    if len(sys.argv) not in (6, 7):
        usage()
        sys.exit(1)
    try:
        n, l, h, x, y = (int(a) for a in sys.argv[1:6])
        seed = int(sys.argv[6]) if len(sys.argv) == 7 else None
    except ValueError:
        usage()
        sys.exit(1)
    if n <= 0 or l < 1 or x < 1 or h < l or y < x:
        usage()
        sys.exit(1)
    steadyGame(n, l, h, x, y, seed)
