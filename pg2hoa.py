#!/usr/bin/env python3

import math
import re
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


class ParityGame:
    def __init__(self, filename):
        f = open(filename, "r")
        # information to be obtained from the file
        self.names = []
        self.succ = []
        self.owner = []
        self.prio = []
        # let's get to reading that file then
        header = True
        for line in f.readlines():
            # some string treatment here
            # (1) check lines end with semicolon, and remove it
            scolon = line.find(';')
            assert(scolon > -1)
            line = line[:scolon]
            # (2) replace commas followed by spaces and use comma only for
            # convenience
            line = re.sub(",\s*", ",", line)
            # now we can continue
            if header:
                header = False
                hdrItems = line.split()
                assert(len(hdrItems) == 2)
                assert(hdrItems[0] == "parity")
                noVtces = int(hdrItems[1]) + 1
                self.prio = [0] * noVtces
                self.owner = [0] * noVtces
                self.succ = [[]] * noVtces
                self.name = [""] * noVtces
                self.maxoutdeg = -1
                self.maxpriority = -1
            else:
                vtxInfo = line.split()
                assert(len(vtxInfo) in [4, 5])
                idx = int(vtxInfo[0])
                assert(idx < len(self.prio))
                # start filling details for this vertex then
                self.prio[idx] = int(vtxInfo[1])
                self.maxpriority = max(self.maxpriority,
                                       self.prio[idx])
                self.owner[idx] = int(vtxInfo[2])
                self.succ[idx] = [int(s) for s in vtxInfo[3].split(',')]
                self.maxoutdeg = max(self.maxoutdeg,
                                     len(self.succ[idx]))
                if len(vtxInfo) == 5:
                    self.name[idx] = vtxInfo[4].strip()[1:-1]
                else:
                    self.name[idx] = f"vertex{idx}"
        self.noAP = math.floor(math.log(self.maxoutdeg, 2)) + 1

    def genSuccLabel(self, i, j):
        negAPs = [a for a in range(self.noAP) if ((1 << a) & j) == 0]
        posAPs = [a for a in range(self.noAP) if a not in negAPs]
        if self.owner[i] == 1:
            negAPs = [a + self.noAP for a in negAPs]
            posAPs = [a + self.noAP for a in posAPs]
        negLabel = " & ".join([f"!{a}" for a in negAPs])
        posLabel = " & ".join([str(a) for a in posAPs])
        return f"{negLabel} & {posLabel}"

    def toHoa(self):
        print("HOA: v1")
        print(f"States: {len(self.prio)}")
        print("Start: 0")
        print(f"acc-name: parity max even {self.maxpriority + 1}")
        sign = hoaAccSign(self.maxpriority)
        print(f"Acceptance: {self.maxpriority + 1} {sign}")
        names0 = " ".join([f"\"pl0_{i}\"" for i in range(self.noAP)])
        names1 = " ".join([f"\"pl1_{i}\"" for i in range(self.noAP)])
        print(f"AP: {self.noAP * 2} {names0} {names1}")
        indices1 = " ".join([str(x) for x in range(self.noAP)])
        print(f"controllable-AP: {indices1}")
        print("properties: deterministic complete colored")
        print("--BODY--")
        for i in range(len(self.prio)):
            print(f"State: {i} \"{self.name[i]}\" { {self.prio[i]} }")
            if len(self.succ[i]) == 1:
                print(f"[t] {self.succ[i][0]}")
            else:
                # we exclude the first to handle it later
                allLabels = []
                for j in range(1, len(self.succ[i])):
                    label = self.genSuccLabel(i, j)
                    print(f"[{label}] {self.succ[i][j]}")
                    allLabels.append(label)
                orLabels = " | ".join([f"({lab})" for lab in allLabels])
                negLabels = f"!({orLabels})"
                print(f"[{negLabels}] {self.succ[i][0]}")

        print("--END--")
        return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"{sys.argv[0]} requires precisely one argument: "
              "a parity game in PGSolver format")
        exit(1)
    game = ParityGame(sys.argv[1])
    exit(game.toHoa())
