# Failed Test Analysis & Fix Plan

**Total Failed Tests:** 19
**Analysis Date:** After merge with main (modulo operator added)

---

## Category 1: Syntax Errors in Tests (2 tests) - QUICK FIX

### 1.1 `arrays/edge_find_contains_null.hml`
**Error:** `Runtime error: Unknown object type 'array'`
**Line 23:** `let arr3: array = [];`
**Issue:** `array` is not a valid type annotation in Hemlock
**Fix:** Change to `let arr3 = [];`
**Priority:** HIGH (easy fix)

### 1.2 `memory/edge_alloc_zero.hml`
**Error:** `[line 4] Error at 'ptr': Expect variable name`
**Issue:** `ptr` is a reserved keyword, cannot be used as variable name
**Fix:** Rename variable to `p` or `pointer`
**Priority:** HIGH (easy fix)

---

## Category 2: Implementation Too Strict (4 tests) - NEEDS CODE CHANGES

These tests reveal that slice/substr operations error instead of clamping to valid bounds. This is a design decision issue.

### 2.1 `strings/edge_slice_bounds.hml`
**Error:** `slice() end index 100 out of bounds (length=5)`
**Issue:** String slice() errors when end > length
**Expected behavior:** Should clamp to string length like Python/JavaScript
**Example:** `"hello".slice(0, 100)` should return `"hello"`, not error
**Priority:** MEDIUM (implementation change needed)

### 2.2 `strings/edge_substr_bounds.hml`
**Error:** `substr() start index 10 out of bounds (length=5)`
**Issue:** substr() errors when start >= length
**Expected behavior:** Should return empty string when start >= length
**Example:** `"hello".substr(10, 5)` should return `""`, not error
**Priority:** MEDIUM (implementation change needed)

### 2.3 `arrays/edge_empty_operations.hml`
**Error:** `slice() end index out of bounds`
**Issue:** Array slice() errors when end > length
**Expected behavior:** Should clamp to array length
**Example:** `[1,2,3].slice(0, 100)` should return `[1,2,3]`, not error
**Priority:** MEDIUM (implementation change needed)

### 2.4 `arrays/edge_slice_bounds.hml`
**Error:** `slice() end index out of bounds`
**Issue:** Same as 2.3
**Priority:** MEDIUM (implementation change needed)

---

## Category 3: Tests Not Catching Errors Properly (4 tests) - TEST LOGIC FIX

These tests use try-catch but errors happen before catch can handle them, OR tests expect wrong behavior.

### 3.1 `strings/edge_char_byte_at_bounds.hml`
**Error:** `char_at() index 10 out of bounds (length=5)`
**Issue:** Test has try-catch but error exits before catch
**Investigation needed:** Why isn't the try-catch working?
**Possible causes:**
- Error thrown at parse/compile time, not runtime
- Error is uncatchable (assertion failure, not exception)
**Fix:** Either make errors catchable OR update test expectations
**Priority:** HIGH (investigate)

### 3.2 `strings/edge_out_of_bounds.hml`
**Error:** `String index 10 out of bounds (length=5)`
**Issue:** Test expects error but error isn't caught
**Same as:** 3.1
**Priority:** HIGH (investigate)

### 3.3 `arrays/edge_out_of_bounds.hml`
**Error:** Array index out of bounds
**Issue:** Same as 3.1
**Priority:** HIGH (investigate)

### 3.4 `strings/edge_negative_index.hml`
**Status:** Actually PASSES (marked as expected error)
**No fix needed**

---

## Category 4: Async Test Logic Issues (3 tests) - TEST DESIGN FIX

### 4.1 `async/edge_detach_then_join.hml`
**Error:** `spawn() expects an async function`
**Issue:** Test is calling spawn incorrectly after detach
**Investigation needed:** Check test logic
**Priority:** MEDIUM (test fix)

### 4.2 `async/edge_channel_closed.hml`
**Error:** `cannot send to closed channel`
**Issue:** Test expects to catch error, but error happens immediately
**Expected:** send() on closed channel should be catchable error
**Actual:** Uncatchable runtime error
**Fix:** Either make error catchable OR update test expectations
**Priority:** MEDIUM (investigate)

### 4.3 `async/edge_join_twice.hml`
**Error:** `task handle already joined`
**Issue:** Same as 4.2 - error not catchable
**Priority:** MEDIUM (investigate)

---

## Category 5: Other Test Issues (6 tests)

### 5.1 `control/edge_for_in_empty.hml`
**Error:** Generic runtime error
**Investigation needed:** Check what's failing
**Priority:** MEDIUM

### 5.2 `functions/edge_arity_mismatch.hml`
**Error:** `Function expects 2 arguments, got 1`
**Issue:** Test expects to catch error, but error is uncatchable
**Fix:** Either make arity errors catchable OR update test
**Priority:** MEDIUM

### 5.3 `functions/edge_recursive_no_base.hml`
**Error:** Infinite recursion / stack overflow
**Issue:** This SHOULD fail - it's testing stack overflow detection
**Status:** Might be timing out or crashing
**Fix:** Make this a known-crash test or add timeout handling
**Priority:** LOW (expected to fail)

### 5.4 `io/edge_closed_file_ops.hml`
**Investigation needed**
**Priority:** MEDIUM

### 5.5 `memory/edge_free_null.hml`
**Error:** `free() requires a pointer, buffer, object, or array`
**Issue:** free(null) errors instead of being a safe no-op
**Expected:** free(null) should be safe (like C's free)
**Fix:** Update free() implementation to accept null
**Priority:** MEDIUM (implementation change)

### 5.6 `arrays/edge_insert_bounds.hml`
**Error:** `insert() index out of bounds`
**Issue:** Test expects insert beyond bounds to work or give specific behavior
**Investigation needed:** What's the expected behavior?
**Priority:** MEDIUM

---

## Fix Plan Summary

### Phase 1: Quick Fixes (Test Syntax) - 30 minutes
- [ ] Fix `arrays/edge_find_contains_null.hml` (remove type annotation)
- [ ] Fix `memory/edge_alloc_zero.hml` (rename `ptr` variable)
- [ ] Run tests, confirm fixes work

### Phase 2: Investigate Catchability Issues - 1 hour
- [ ] Investigate why try-catch doesn't catch:
  - String/array index out of bounds errors
  - char_at/byte_at errors
  - Async errors (channel closed, join twice)
  - Function arity errors
- [ ] Determine: Are these design choices (uncatchable) or bugs?
- [ ] Document findings

### Phase 3: Update Tests Based on Investigation - 2 hours
**Option A:** If errors should be catchable:
- [ ] File issues for making errors catchable
- [ ] Update tests to note current limitation

**Option B:** If errors are intentionally uncatchable:
- [ ] Update tests to expect crashes instead of catch
- [ ] Mark as expected-error tests
- [ ] Document in test comments

### Phase 4: Implementation Changes (Optional) - 4-8 hours
These require code changes to Hemlock internals:

**High Value:**
- [ ] Make slice/substr clamp to bounds instead of error
- [ ] Make free(null) a safe no-op

**Medium Value:**
- [ ] Make certain errors catchable (if Phase 2 determines they should be)

**Low Value:**
- [ ] Define insert() beyond-bounds behavior

---

## Recommended Approach

### Immediate (Do Now):
1. **Fix syntax errors** (Category 1) - 2 tests fixed immediately
2. **Investigate catchability** (Category 3) - Understand the issue
3. **Update test expectations** based on findings

### Short Term (This Week):
4. **Fix test logic issues** (Categories 3-5) - Update tests to match reality
5. **Document current behavior** in test comments
6. **Create issues** for implementation improvements

### Long Term (Future PR):
7. **Improve slice/substr** to clamp instead of error
8. **Make free(null) safe** like C
9. **Make more errors catchable** (if design permits)

---

## Expected Outcome

After Phase 1-3 (test fixes only):
- **Passing:** 337+ (fixing 15 test issues)
- **Expected errors:** 24-28 (marking uncatchable errors)
- **Failed:** 0-4 (genuine bugs only)

This will give a much clearer picture of what's working vs. what needs implementation changes.

---

## Priority Classification

**P0 (Do Now):** Syntax errors (2 tests)
**P1 (This Session):** Investigate catchability, update test expectations (8-10 tests)
**P2 (Future PR):** Implementation improvements (4-6 tests)
**P3 (Nice to Have):** Edge cases that may not matter (2-3 tests)
