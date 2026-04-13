#!/usr/bin/env python3
"""Towers-of-Hanoi reachability game in extended HOA format.

Reimplements pgsolver's ``towersofhanoi``
(src/lib/generators/towersofhanoi.ml). Every node is owned by player 0
(Even). The goal state (all disks moved from peg 0 to peg 1, with peg 2
also empty) has priority 0 with a self-loop; every other state has
priority 1. Thus player 0 wins iff she can reach the goal.

State representation: a 3-tuple of peg contents, each peg being a tuple
of disk sizes with the head being the top (smallest) disk.
"""

import sys
from collections import deque

from hoagen import emitHoa


MOVES = [(0, 1, 2), (0, 2, 1), (1, 0, 2), (1, 2, 0), (2, 0, 1), (2, 1, 0)]


def is_finished(st):
    return len(st[0]) == 0 and len(st[2]) == 0


def successors(st):
    if is_finished(st):
        return [st]  # absorbing self-loop
    tops = tuple(peg[0] if peg else 0 for peg in st)
    out = []
    for f, t, _ in MOVES:
        if tops[f] > 0 and (tops[t] == 0 or tops[f] < tops[t]):
            nst = list(st)
            nst[f] = st[f][1:]
            nst[t] = (tops[f],) + st[t]
            out.append(tuple(nst))
    return out


def format_state(st):
    def fmt(peg):
        return "[" + ",".join(str(x) for x in peg) + "]"
    return f"{fmt(st[0])} {fmt(st[1])} {fmt(st[2])}"


def tohGame(n):
    init = (tuple(range(1, n + 1)), (), ())
    index = {init: 0}
    order = [init]
    q = deque([init])
    while q:
        st = q.popleft()
        for s in successors(st):
            if s not in index:
                index[s] = len(order)
                order.append(s)
                q.append(s)

    N = len(order)
    prio = [0 if is_finished(s) else 1 for s in order]
    owner = [0] * N
    succ = [[index[s] for s in successors(st)] for st in order]
    names = [format_state(st) for st in order]
    emitHoa(N, prio, owner, succ, names)


def usage():
    print(f"{sys.argv[0]} requires one argument: n\n"
          "  n: number of disks (positive integer)",
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
    tohGame(n)
