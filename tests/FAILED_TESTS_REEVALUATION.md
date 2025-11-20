# Failed Tests Reevaluation After Main Merge

**Date:** After merging typed arrays feature from main
**Previous Failed:** 19 tests
**Current Failed:** 18 tests (-1 ✅)
**Passed:** 326 (+4 from 322)
**Expected Errors:** 28 (+4 from 24)

---

## What Was Fixed by Main Merge

### ✅ `arrays/edge_find_contains_null.hml` - FIXED
**Previous Error:** `Unknown object type 'array'`
**Fix:** Typed arrays feature now supports `array` type annotation
**Status:** Now passing!

---

## Remaining 18 Failed Tests - Updated Analysis

### Category 1: Syntax Error (1 test) - EASY FIX

#### `memory/edge_alloc_zero.hml`
**Error:** `[line 4] Error at 'ptr': Expect variable name`
**Issue:** `ptr` is a reserved keyword
**Fix:** Rename `ptr` to `p` or `pointer`
**Priority:** P0 - 5 minutes

---

### Category 2: Bounds Checking Too Strict (5 tests) - IMPLEMENTATION CHANGE NEEDED

These all fail because slice/substr don't clamp to valid bounds:

#### String Operations (3 tests)
1. `strings/edge_slice_bounds.hml` - `slice() end index 100 out of bounds`
2. `strings/edge_substr_bounds.hml` - `substr() start index 10 out of bounds`
3. `strings/edge_empty_operations.hml` - slice bounds issue

**Current Behavior:** Errors when indices exceed bounds
**Expected Behavior:** Clamp to string length (like Python/JS/Rust)
**Example:** `"hello".slice(0, 100)` should return `"hello"`, not error

#### Array Operations (2 tests)
4. `arrays/edge_empty_operations.hml` - `slice() end index out of bounds`
5. `arrays/edge_slice_bounds.hml` - `slice() end index out of bounds`

**Current Behavior:** Errors when indices exceed bounds
**Expected Behavior:** Clamp to array length
**Example:** `[1,2,3].slice(0, 100)` should return `[1,2,3]`, not error

**Implementation Fix Required:**
- Modify `slice()` in `src/interpreter/io/string_methods.c`
- Modify `slice()` in `src/interpreter/io/array_methods.c`
- Modify `substr()` in `src/interpreter/io/string_methods.c`
- Clamp indices to valid ranges instead of throwing errors

**Priority:** P1 - 1-2 hours (code change)

---

### Category 3: Errors Not Catchable by try-catch (6 tests) - DESIGN ISSUE

These tests use try-catch blocks but errors exit immediately instead of being caught:

#### String/Array Index Errors (3 tests)
1. `strings/edge_char_byte_at_bounds.hml` - char_at/byte_at out of bounds
2. `strings/edge_out_of_bounds.hml` - string index out of bounds
3. `arrays/edge_out_of_bounds.hml` - array index out of bounds

**Test Code Example:**
```hemlock
try {
    let ch = s.char_at(10);  // Should be caught
    print("ERROR: Should have thrown");
} catch (e) {
    print("Caught: " + e);  // Never reaches here
}
```

**Actual Behavior:** Program exits with `Runtime error: char_at() index 10 out of bounds`
**Expected Behavior:** Error should be catchable with try-catch

#### Async Errors (3 tests)
4. `async/edge_channel_closed.hml` - `cannot send to closed channel`
5. `async/edge_join_twice.hml` - `task handle already joined`
6. `async/edge_detach_then_join.hml` - `spawn() expects an async function`

**Same Issue:** Errors exit immediately instead of being caught

**Root Cause Investigation Needed:**
- Are these errors thrown with `throw` or `exit()`?
- Are they using `fprintf(stderr)` + `exit(1)` instead of exceptions?
- Should these be catchable or are they intentional fatal errors?

**Possible Solutions:**
1. **Option A:** Make errors catchable (change implementation to throw exceptions)
2. **Option B:** Update tests to expect program exit (not catchable)
3. **Option C:** Document as fatal errors, mark tests as expected-error

**Priority:** P1 - Investigation needed (1 hour) + fix decision

---

### Category 4: Other Issues (6 tests)

#### `control/edge_for_in_empty.hml`
**Error:** Runtime error (need to investigate)
**Priority:** P2

#### `functions/edge_arity_mismatch.hml`
**Error:** `Function expects 2 arguments, got 1`
**Issue:** Error not catchable (same as Category 3)
**Priority:** P1

#### `functions/edge_recursive_no_base.hml`
**Error:** Infinite recursion / stack overflow
**Issue:** This SHOULD fail - testing stack overflow detection
**Status:** May be timing out or segfaulting
**Fix:** Mark as expected crash or add timeout handling
**Priority:** P3 (expected behavior)

#### `io/edge_closed_file_ops.hml`
**Error:** File operations on closed file
**Investigation needed:** Are errors catchable?
**Priority:** P2

#### `memory/edge_free_null.hml`
**Error:** `free() requires a pointer, buffer, object, or array`
**Issue:** `free(null)` should be a safe no-op (like C)
**Current:** Errors instead of ignoring
**Fix Required:** Update `free()` implementation to accept null
**Priority:** P1 - Easy implementation fix (30 minutes)

#### `arrays/edge_insert_bounds.hml`
**Error:** `insert() index out of bounds`
**Investigation needed:** What's expected behavior for insert beyond bounds?
**Priority:** P2

---

## Updated Fix Plan

### Phase 1: Quick Fixes (30 minutes)
- [ ] Fix `memory/edge_alloc_zero.hml` - rename `ptr` variable
- [ ] Fix `memory/edge_free_null.hml` - make `free(null)` safe
- **Expected Result:** 2 tests fixed, 16 remaining

### Phase 2: Investigation (1-2 hours)
- [ ] Investigate why errors aren't catchable
  - Check error throwing mechanism in string/array methods
  - Check async error handling
  - Check function arity checking
- [ ] Determine: Should these be catchable or fatal?
- [ ] Document findings

### Phase 3: Decision Point
**Based on Phase 2 findings:**

**If errors should be catchable:**
- [ ] Update implementations to throw catchable exceptions
- [ ] Verify all 6 tests now pass
- **Estimated:** 4-6 hours

**If errors are intentionally fatal:**
- [ ] Update tests to mark as expected-error
- [ ] Add comments explaining why
- **Estimated:** 30 minutes

### Phase 4: Bounds Clamping (2-3 hours)
- [ ] Update `slice()` for strings to clamp indices
- [ ] Update `slice()` for arrays to clamp indices
- [ ] Update `substr()` for strings to clamp indices
- [ ] Run tests, verify 5 tests now pass

### Phase 5: Remaining Issues (1-2 hours)
- [ ] Investigate `control/edge_for_in_empty.hml`
- [ ] Investigate `io/edge_closed_file_ops.hml`
- [ ] Investigate `arrays/edge_insert_bounds.hml`
- [ ] Mark `functions/edge_recursive_no_base.hml` appropriately

---

## Prioritized Action Items

### Do Now (P0) - 30 min
1. Fix `ptr` variable name syntax error
2. Make `free(null)` a safe no-op

### Do Next (P1) - 2-3 hours
3. Investigate error catchability (Category 3)
4. Update slice/substr to clamp instead of error (Category 2)

### Do Later (P2) - 1-2 hours
5. Investigate remaining edge cases
6. Document behavior

### Low Priority (P3)
7. Handle infinite recursion test appropriately

---

## Expected Outcomes

### After Phase 1 (Quick Fixes):
- **Passing:** 328 (+2)
- **Failed:** 16 (-2)

### After Phase 2-3 (Error Catchability):
- **If made catchable:** Passing: 334 (+6)
- **If marked expected:** Passing: 328, Expected Errors: 34 (+6)

### After Phase 4 (Bounds Clamping):
- **Passing:** 339 (+5)
- **Failed:** 5-8 (only edge cases)

### After Phase 5 (All Fixes):
- **Passing:** 342-345
- **Expected Errors:** 28-34
- **Failed:** 0-3 (genuine bugs only)

---

## Recommendation

**Start with Phase 1** (quick fixes) to immediately improve test results, then **investigate error catchability** (Phase 2) to inform the right approach for the remaining 16 tests.

The main question to answer: **Should bounds checking and other errors be catchable exceptions, or intentional fatal errors?** This decision affects 6-11 of the remaining 18 tests.
