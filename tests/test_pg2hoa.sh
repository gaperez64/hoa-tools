#!/bin/bash
# Regression test suite for pg2hoa.py
# Tests that pg2hoa produces valid HOA output accepted by hoacheck
#
# Usage: ./tests/test_pg2hoa.sh [path/to/hoacheck]
# If hoacheck is not provided, it will be built automatically.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
FIXTURES="$SCRIPT_DIR/pg_fixtures"

# Build or locate hoacheck
if [ $# -ge 1 ]; then
    HOACHECK="$1"
else
    HOACHECK="$ROOT_DIR/hoacheck"
    if [ ! -x "$HOACHECK" ]; then
        echo "Building hoacheck..."
        make -C "$ROOT_DIR" hoacheck
    fi
fi

if [ ! -x "$HOACHECK" ]; then
    echo "ERROR: hoacheck not found or not executable at $HOACHECK"
    exit 1
fi

PG2HOA="$ROOT_DIR/pg2hoa.py"
passed=0
failed=0
errors=""

run_test() {
    local pg_file="$1"
    local test_name="$(basename "$pg_file" .pg)"
    local hoa_output

    # Generate HOA from parity game
    hoa_output=$(python3 "$PG2HOA" "$pg_file" 2>&1) || {
        echo "FAIL: $test_name (pg2hoa.py crashed)"
        errors="$errors\n  $test_name: pg2hoa.py exited with error"
        failed=$((failed + 1))
        return
    }

    # Validate with hoacheck
    local check_err
    check_err=$(echo "$hoa_output" | "$HOACHECK" 2>&1) || {
        echo "FAIL: $test_name (hoacheck rejected output)"
        errors="$errors\n  $test_name: $check_err"
        failed=$((failed + 1))
        return
    }

    echo "PASS: $test_name"
    passed=$((passed + 1))
}

# Test: acceptance condition content for priority 0 (the bug from issue #4)
test_prio0_acceptance() {
    local pg_file="$FIXTURES/prio0_only.pg"
    local test_name="prio0_acceptance_content"
    local hoa_output

    hoa_output=$(python3 "$PG2HOA" "$pg_file" 2>&1) || {
        echo "FAIL: $test_name (pg2hoa.py crashed)"
        errors="$errors\n  $test_name: pg2hoa.py exited with error"
        failed=$((failed + 1))
        return
    }

    local acc_line
    acc_line=$(echo "$hoa_output" | grep '^Acceptance:')

    if echo "$acc_line" | grep -q 'Inf(0)'; then
        echo "PASS: $test_name (got: $acc_line)"
        passed=$((passed + 1))
    else
        echo "FAIL: $test_name (expected Inf(0), got: $acc_line)"
        errors="$errors\n  $test_name: expected 'Acceptance: 1 Inf(0)', got '$acc_line'"
        failed=$((failed + 1))
    fi
}

echo "=== pg2hoa.py regression tests ==="
echo ""

# Run the specific issue #4 regression test
echo "--- Issue #4: priority 0 acceptance ---"
test_prio0_acceptance
echo ""

# Run hoacheck validation on all fixtures
echo "--- hoacheck validation on all fixtures ---"
for pg_file in "$FIXTURES"/*.pg; do
    run_test "$pg_file"
done

echo ""
echo "=== Results: $passed passed, $failed failed ==="
if [ $failed -gt 0 ]; then
    echo -e "Failures:$errors"
    exit 1
fi
