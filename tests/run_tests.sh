#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

HEMLOCK="./hemlock"
PASSED=0
FAILED=0

# Function to run a test that should succeed
test_success() {
    local test_file=$1
    local expected_output=$2
    
    echo -n "Testing $test_file... "
    
    actual_output=$($HEMLOCK "$test_file" 2>&1)
    
    if [ "$actual_output" = "$expected_output" ]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected: $expected_output"
        echo "  Got:      $actual_output"
        ((FAILED++))
    fi
}

# Function to run a test that should error
test_error() {
    local test_file=$1
    local expected_error=$2
    
    echo -n "Testing $test_file (should error)... "
    
    actual_output=$($HEMLOCK "$test_file" 2>&1)
    
    if echo "$actual_output" | grep -q "$expected_error"; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected error containing: $expected_error"
        echo "  Got: $actual_output"
        ((FAILED++))
    fi
}

echo "Running Hemlock Test Suite"
echo "=========================="
echo ""

# Make sure hemlock is built
if [ ! -f "$HEMLOCK" ]; then
    echo -e "${RED}Error: hemlock binary not found. Run 'make' first.${NC}"
    exit 1
fi

# Run tests
test_success "tests/primitives/i8_valid.hml" "127
-128
0"

test_success "tests/primitives/u8_valid.hml" "0
255
128"

test_success "tests/primitives/i32_valid.hml" "2147483647
-2147483648
0"

test_success "tests/primitives/floats.hml" "3.14
2.5
0"

test_error "tests/primitives/u8_overflow.hml" "out of range for u8"

test_error "tests/primitives/u8_negative.hml" "out of range for u8"

test_error "tests/primitives/i8_overflow.hml" "out of range for i8"

test_success "tests/conversions/int_to_float.hml" "8.14"

test_success "tests/conversions/u8_to_i32.hml" "30"

test_success "tests/conversions/mixed.hml" "98.14"

test_success "tests/strings/concat.hml" "hello world"

test_success "tests/strings/index.hml" "104
111"

test_success "tests/strings/mutate.hml" "Hello"

test_success "tests/strings/length.hml" "5"

test_success "tests/control/if_true.hml" "yes"

test_success "tests/control/if_false.hml" "no"

test_success "tests/control/while.hml" "5
4
3
2
1
0"

test_success "tests/bools/literals.hml" "true
false"

test_success "tests/bools/operators.hml" "false
true
true
false"

# Summary
echo ""
echo "=========================="
echo -e "Tests passed: ${GREEN}$PASSED${NC}"
echo -e "Tests failed: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi