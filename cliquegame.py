#!/usr/bin/env python3

import math
import sys

from pg2hoa import hoaAccSign


def genSuccLabel(owner_i, noAP, j):
    negAPs = [a for a in range(noAP) if ((1 << a) & j) == 0]
    posAPs = [a for a in range(noAP) if a not in negAPs]
    if owner_i == 1:
        negAPs = [a + noAP for a in negAPs]
        posAPs = [a + noAP for a in posAPs]
    negLabel = " & ".join([f"!{a}" for a in negAPs])
    posLabel = " & ".join([str(a) for a in posAPs])
    return f"{negLabel} & {posLabel}"


def cliqueGame(n, with_self_cycles):
    prio = list(range(n))
    owner = [i % 2 for i in range(n)]
    succ = [[j for j in range(n) if with_self_cycles or j != i]
            for i in range(n)]
    maxoutdeg = max(len(s) for s in succ)
    if maxoutdeg < 1:
        print("Error: every vertex must have at least one successor; "
              "use the 'self' argument or increase n.", file=sys.stderr)
        exit(1)
    noAP = math.floor(math.log(maxoutdeg, 2)) + 1
    maxpriority = n - 1

    print("HOA: v1")
    print(f"States: {n}")
    print("Start: 0")
    print(f"acc-name: parity max even {maxpriority + 1}")
    sign = hoaAccSign(maxpriority)
    print(f"Acceptance: {maxpriority + 1} {sign}")
    names0 = " ".join([f"\"pl0_{i}\"" for i in range(noAP)])
    names1 = " ".join([f"\"pl1_{i}\"" for i in range(noAP)])
    print(f"AP: {noAP * 2} {names0} {names1}")
    indices1 = " ".join([str(x) for x in range(noAP)])
    print(f"controllable-AP: {indices1}")
    print("properties: deterministic complete colored")
    print("--BODY--")
    for i in range(n):
        print(f"State: {i} \"vertex{i}\" {{ {prio[i]} }}")
        if len(succ[i]) == 1:
            print(f"[t] {succ[i][0]}")
        else:
            allLabels = []
            for j in range(1, len(succ[i])):
                label = genSuccLabel(owner[i], noAP, j)
                print(f"[{label}] {succ[i][j]}")
                allLabels.append(label)
            orLabels = " | ".join([f"({lab})" for lab in allLabels])
            negLabels = f"!({orLabels})"
            print(f"[{negLabels}] {succ[i][0]}")
    print("--END--")
    return 0


def usage():
    print(f"{sys.argv[0]} requires 1 or 2 arguments: "
          "n [self]\n"
          "  n:    number of vertices (positive integer)\n"
          "  self: literal string 'self' to enable self-loops",
          file=sys.stderr)


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        usage()
        exit(1)
    try:
        n = int(sys.argv[1])
    except ValueError:
        usage()
        exit(1)
    if n <= 0:
        usage()
        exit(1)
    with_self_cycles = False
    if len(sys.argv) == 3:
        if sys.argv[2] != "self":
            usage()
            exit(1)
        with_self_cycles = True
    exit(cliqueGame(n, with_self_cycles))
