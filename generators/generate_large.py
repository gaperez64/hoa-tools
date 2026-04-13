#!/usr/bin/env python3
"""Generate 20 large parity games per generator in extended HOA format.

For each of ``cliquegame``, ``laddergame``, ``towersofhanoi`` and
``steadygame`` this script produces up to 20 games with increasing
parameters, skipping any configuration that would exceed the 10MB file
size cap. Output files go to ``output/`` next to this script (override
with a single argv argument).

Note: ``towersofhanoi``'s reachable state space is 3^n, so only n in
roughly 2..10 fits under 10MB. The script emits what it can and warns
when it produces fewer than 20 games for a generator.
"""

import os
import subprocess
import sys

MB = 1024 * 1024
SIZE_LIMIT = 10 * MB
PER_GEN = 20
HERE = os.path.dirname(os.path.abspath(__file__))


def params_cliquegame():
    # n^2 edge lines dominate; n ~ 305 approaches 10MB.
    return [(str(n),) for n in range(20, 306, 15)]


def params_laddergame():
    # 2n states, each with ~3 short edge lines; ~135 bytes per state.
    return [(str(n),) for n in range(2500, 52500, 2500)]


def params_towersofhanoi():
    # 3^n states. n=10 is ~6.4MB, n=11 would blow past 10MB.
    return [(str(n),) for n in range(2, 11)]


def params_steadygame():
    # Random game of n vertices. Seed derived from n for reproducibility.
    return [(str(n), "5", "15", "5", "15", str(n))
            for n in range(500, 10500, 500)]


def run(script, args, out_path):
    with open(out_path, "wb") as f:
        proc = subprocess.run(
            [sys.executable, os.path.join(HERE, script), *args],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
        )
    if proc.returncode != 0:
        os.unlink(out_path)
        return None, proc.stderr.decode("utf-8", errors="replace").strip()
    if len(proc.stdout) > SIZE_LIMIT:
        os.unlink(out_path)
        return len(proc.stdout), "over-limit"
    with open(out_path, "wb") as f:
        f.write(proc.stdout)
    return len(proc.stdout), None


def drive(name, script, paramlist, out_dir):
    print(f"\n=== {name} ===")
    count = 0
    for params in paramlist:
        if count >= PER_GEN:
            break
        out_path = os.path.join(out_dir, f"{name}_{count + 1:02d}.ehoa")
        size, err = run(script, params, out_path)
        if err == "over-limit":
            print(f"  skip {params}: {size / MB:.2f} MB exceeds {SIZE_LIMIT // MB} MB")
            continue
        if err is not None:
            print(f"  fail {params}: {err}")
            continue
        count += 1
        print(f"  {out_path}  args={' '.join(params)}  size={size / MB:.2f} MB")
    if count < PER_GEN:
        print(f"  WARNING: {name} produced only {count} games "
              f"(parameter space too small for {PER_GEN} under {SIZE_LIMIT // MB} MB)")
    return count


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "output")
    os.makedirs(out_dir, exist_ok=True)
    plan = [
        ("cliquegame",    "cliquegame.py",    params_cliquegame()),
        ("laddergame",    "laddergame.py",    params_laddergame()),
        ("towersofhanoi", "towersofhanoi.py", params_towersofhanoi()),
        ("steadygame",    "steadygame.py",    params_steadygame()),
    ]
    total = 0
    for name, script, paramlist in plan:
        total += drive(name, script, paramlist, out_dir)
    print(f"\nDone. Wrote {total} games to {out_dir}")


if __name__ == "__main__":
    main()
