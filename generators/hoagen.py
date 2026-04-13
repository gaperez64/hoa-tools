"""Shared helpers for emitting extended-HOA parity games.

Kept deliberately small: `hoaAccSign`, `genSuccLabel` and `emitHoa` together
produce the same output shape as ``pg2hoa.py`` so files round-trip through
``hoa2pg`` and ``hoacheck``.
"""

import math
import sys


def hoaAccSign(priority):
    if priority == 0:
        return "Inf(0)"
    rec = [""] * (priority + 1)
    rec[0] = "Inf(0)"
    for p in range(1, priority):
        if p % 2 == 0:
            rec[p] = f"(Inf({p}) | {rec[p - 1]})"
        else:
            rec[p] = f"(Fin({p}) & {rec[p - 1]})"
    if priority % 2 == 0:
        return f"Inf({priority}) | {rec[priority - 1]}"
    else:
        return f"Fin({priority}) & {rec[priority - 1]}"


def genSuccLabel(owner_i, noAP, j):
    negAPs = [a for a in range(noAP) if ((1 << a) & j) == 0]
    posAPs = [a for a in range(noAP) if a not in negAPs]
    if owner_i == 1:
        negAPs = [a + noAP for a in negAPs]
        posAPs = [a + noAP for a in posAPs]
    negLabel = " & ".join([f"!{a}" for a in negAPs])
    posLabel = " & ".join([str(a) for a in posAPs])
    return f"{negLabel} & {posLabel}"


def emitHoa(n_states, prio, owner, succ, names, out=sys.stdout):
    """Write an extended-HOA file describing a parity game.

    Arguments mirror the fields of pg2hoa.ParityGame: priorities, owners
    (0 or 1), successor lists (indices into [0, n_states)) and state names.
    """
    maxoutdeg = max(len(s) for s in succ)
    if maxoutdeg < 1:
        print("Error: every vertex must have at least one successor.",
              file=sys.stderr)
        sys.exit(1)
    noAP = math.floor(math.log(maxoutdeg, 2)) + 1
    maxpriority = max(prio)

    w = out.write

    w("HOA: v1\n")
    w(f"States: {n_states}\n")
    w("Start: 0\n")
    w(f"acc-name: parity max even {maxpriority + 1}\n")
    sign = hoaAccSign(maxpriority)
    w(f"Acceptance: {maxpriority + 1} {sign}\n")
    names0 = " ".join([f"\"pl0_{i}\"" for i in range(noAP)])
    names1 = " ".join([f"\"pl1_{i}\"" for i in range(noAP)])
    w(f"AP: {noAP * 2} {names0} {names1}\n")
    indices1 = " ".join([str(x) for x in range(noAP)])
    w(f"controllable-AP: {indices1}\n")
    w("properties: deterministic complete colored\n")
    w("--BODY--\n")
    for i in range(n_states):
        w(f"State: {i} \"{names[i]}\" {{ {prio[i]} }}\n")
        if len(succ[i]) == 1:
            w(f"[t] {succ[i][0]}\n")
        else:
            allLabels = []
            for j in range(1, len(succ[i])):
                label = genSuccLabel(owner[i], noAP, j)
                w(f"[{label}] {succ[i][j]}\n")
                allLabels.append(label)
            orLabels = " | ".join([f"({lab})" for lab in allLabels])
            negLabels = f"!({orLabels})"
            w(f"[{negLabels}] {succ[i][0]}\n")
    w("--END--\n")
