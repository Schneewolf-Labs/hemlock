# Hemlock Bug Fix Implementation Plan

**Date:** 2025-11-13
**Branch:** claude/analyze-codebase-issues-01Jk6Y4bLimtQM7vJ6B4phyd
**Based on:** ANALYSIS.md findings

## Overview

This document outlines the implementation plan for fixing 7 identified issues in the Hemlock codebase. Fixes are organized into 4 phases based on priority, risk, and dependencies.

---

## Fix Phases

### Phase 1: Foundation Fixes (CRITICAL - Low Risk)
**Goal:** Fix the simplest but most dangerous memory corruption issues
**Estimated Time:** 30 minutes
**Risk Level:** Low (simple additions, minimal side effects)

### Phase 2: Channel Fixes (CRITICAL - Medium Risk)
**Goal:** Fix channel memory leak and concurrency bugs
**Estimated Time:** 45 minutes
**Risk Level:** Medium (affects concurrency, needs careful testing)

### Phase 3: Manual Memory Management (HIGH - Complex)
**Goal:** Fix or redesign builtin_free() behavior
**Estimated Time:** 1 hour
**Risk Level:** High (requires design decision, affects semantics)

### Phase 4: Enhancements (MEDIUM - Low Risk)
**Goal:** Add missing features and thread-safety
**Estimated Time:** 45 minutes
**Risk Level:** Low (incremental improvements)

---

## Phase 1: Foundation Fixes

### Fix 1.1: Initialize ref_count in String/Buffer Allocations
**Issue:** #2 - Uninitialized ref_count
**Severity:** CRITICAL
**Complexity:** Simple

**Files to Modify:**
1. `src/interpreter/io/file_methods.c` (4 locations)
2. `src/interpreter/io/string_methods.c` (1 location)

**Implementation:**

**Location 1: file_methods.c line 49 (after malloc String)**
```c
// Current (line 44-50):
String *str = malloc(sizeof(String));
str->data = buffer;
str->length = read_bytes;
str->char_length = -1;
str->capacity = size + 1;
return (Value){ .type = VAL_STRING, .as.as_string = str };

// Fixed:
String *str = malloc(sizeof(String));
str->data = buffer;
str->length = read_bytes;
str->char_length = -1;
str->capacity = size + 1;
str->ref_count = 0;  // ADD THIS LINE
return (Value){ .type = VAL_STRING, .as.as_string = str };
```

**Location 2: file_methods.c line 86 (after malloc String)**
```c
// Add after line 85:
str->ref_count = 0;
```

**Location 3: file_methods.c line 113 (after malloc Buffer)**
```c
// Current (line 109-114):
Buffer *buf = malloc(sizeof(Buffer));
buf->data = malloc(1);
buf->length = 0;
buf->capacity = 0;
return (Value){ .type = VAL_BUFFER, .as.as_buffer = buf };

// Fixed:
Buffer *buf = malloc(sizeof(Buffer));
buf->data = malloc(1);
buf->length = 0;
buf->capacity = 0;
buf->ref_count = 0;  // ADD THIS LINE
return (Value){ .type = VAL_BUFFER, .as.as_buffer = buf };
```

**Location 4: file_methods.c line 137 (after malloc Buffer)**
```c
// Add after line 136:
buf->ref_count = 0;
```

**Location 5: string_methods.c line 543 (after malloc Buffer)**
```c
// Current (line 538-544):
Buffer *buf = malloc(sizeof(Buffer));
buf->data = malloc(str->length);
memcpy(buf->data, str->data, str->length);
buf->length = str->length;
buf->capacity = str->length;
return (Value){ .type = VAL_BUFFER, .as.as_buffer = buf };

// Fixed:
Buffer *buf = malloc(sizeof(Buffer));
buf->data = malloc(str->length);
memcpy(buf->data, str->data, str->length);
buf->length = str->length;
buf->capacity = str->length;
buf->ref_count = 0;  // ADD THIS LINE
return (Value){ .type = VAL_BUFFER, .as.as_buffer = buf };
```

**Testing:**
- Run existing file I/O tests
- Run under Valgrind to verify no uninitialized memory reads
- Test file.read(), file.read_bytes(), string.to_bytes()

**Success Criteria:**
- All tests pass
- Valgrind shows no uninitialized memory access
- ref_count properly tracked in all operations

---

### Fix 1.2: Add value_retain() to Array unshift/insert
**Issue:** #4 - Array unshift/insert missing ref counting
**Severity:** CRITICAL
**Complexity:** Simple

**Files to Modify:**
1. `src/interpreter/io/array_methods.c` (2 locations)

**Implementation:**

**Location 1: array_methods.c line 84 (unshift method)**
```c
// Current (line 80-86):
// Shift all elements right
for (int i = arr->length; i > 0; i--) {
    arr->elements[i] = arr->elements[i - 1];
}
arr->elements[0] = args[0];
arr->length++;
return val_null();

// Fixed:
// Shift all elements right
for (int i = arr->length; i > 0; i--) {
    arr->elements[i] = arr->elements[i - 1];
}
value_retain(args[0]);  // ADD THIS LINE
arr->elements[0] = args[0];
arr->length++;
return val_null();
```

**Location 2: array_methods.c line 113 (insert method)**
```c
// Current (line 109-115):
// Shift elements right from index
for (int i = arr->length; i > index; i--) {
    arr->elements[i] = arr->elements[i - 1];
}
arr->elements[index] = args[1];
arr->length++;
return val_null();

// Fixed:
// Shift elements right from index
for (int i = arr->length; i > index; i--) {
    arr->elements[i] = arr->elements[i - 1];
}
value_retain(args[1]);  // ADD THIS LINE
arr->elements[index] = args[1];
arr->length++;
return val_null();
```

**Testing:**
- Create test with reference-counted values (strings, buffers, objects)
- Test unshift() with values that go out of scope
- Test insert() with values that go out of scope
- Verify no use-after-free with Valgrind

**Test Script Example:**
```hemlock
// Test unshift with string
let arr = [];
{
    let s = "temporary string";
    arr.unshift(s);  // s should be retained
}
// arr[0] should still be valid
print(arr[0]);  // Should print "temporary string", not crash

// Test insert with object
let arr2 = [1, 2, 3];
{
    let obj = { x: 10, y: 20 };
    arr2.insert(1, obj);  // obj should be retained
}
// arr2[1] should still be valid
print(arr2[1].x);  // Should print 10, not crash
```

**Success Criteria:**
- Test script runs without crashes
- Valgrind shows no use-after-free
- Array elements remain valid after original references are released

---

## Phase 2: Channel Fixes

### Fix 2.1: Add VAL_CHANNEL to value_release()
**Issue:** #1 - Channel memory leak
**Severity:** CRITICAL
**Complexity:** Simple

**Files to Modify:**
1. `src/interpreter/values.c`

**Implementation:**

**Location: values.c line 1101 (add new case before default)**
```c
// Current (line 1097-1106):
case VAL_TASK:
    if (val.as.as_task) {
        task_release(val.as.as_task);
    }
    break;
// Other types don't need reference counting
default:
    break;

// Fixed:
case VAL_TASK:
    if (val.as.as_task) {
        task_release(val.as.as_task);
    }
    break;
case VAL_CHANNEL:  // ADD THIS CASE
    if (val.as.as_channel) {
        channel_free(val.as.as_channel);
    }
    break;
// Other types don't need reference counting
default:
    break;
```

**Note:** Channels are NOT reference-counted (no case in value_retain()), so they're freed immediately when released.

**Testing:**
- Create and destroy many channels in a loop
- Run under Valgrind to detect memory leaks
- Verify pthread resources are properly cleaned up

**Test Script:**
```hemlock
// Create and destroy 1000 channels
let i = 0;
while (i < 1000) {
    let ch = channel(10);
    ch.close();
    // Channel should be freed when ch goes out of scope
    i = i + 1;
}
```

**Success Criteria:**
- No memory leaks in Valgrind
- No pthread resource leaks (check with `ps` or system monitoring)
- Test script completes without errors

---

### Fix 2.2: Add Reference Counting to Channel send/recv
**Issue:** #3 - Channel send/recv use-after-free
**Severity:** CRITICAL
**Complexity:** Medium (affects concurrency)

**Files to Modify:**
1. `src/interpreter/io/channel_methods.c`

**Implementation:**

**Location 1: channel_methods.c line 48 (send method)**
```c
// Current (line 46-51):
// Add message to buffer
ch->buffer[ch->tail] = msg;
ch->tail = (ch->tail + 1) % ch->capacity;
ch->count++;

// Signal that buffer is not empty
pthread_cond_signal(not_empty);

// Fixed:
// Add message to buffer
value_retain(msg);  // ADD THIS LINE - retain before storing
ch->buffer[ch->tail] = msg;
ch->tail = (ch->tail + 1) % ch->capacity;
ch->count++;

// Signal that buffer is not empty
pthread_cond_signal(not_empty);
```

**Location 2: channel_methods.c line 80 (recv method)**
```c
// Current (line 79-88):
// Get message from buffer
Value msg = ch->buffer[ch->head];
ch->head = (ch->head + 1) % ch->capacity;
ch->count--;

// Signal that buffer is not full
pthread_cond_signal(not_full);
pthread_mutex_unlock(mutex);

return msg;

// Fixed:
// Get message from buffer
Value msg = ch->buffer[ch->head];
ch->head = (ch->head + 1) % ch->capacity;
ch->count--;

// Signal that buffer is not full
pthread_cond_signal(not_full);
pthread_mutex_unlock(mutex);

value_release(val);  // ADD THIS LINE - release buffer's ownership
return msg;
```

**Important Consideration:**
The recv() releases the buffer's ownership of the message, but the caller now owns the returned message. This is correct semantics:
- send() increments ref_count (buffer takes ownership)
- recv() decrements ref_count (buffer releases ownership)
- Caller of recv() now owns the message

**Testing:**
- Test send/recv with reference-counted values (strings, objects, arrays)
- Test concurrent send/recv from multiple threads
- Test that sent values remain valid after sender releases them
- Verify no memory leaks or use-after-free with Valgrind

**Test Script:**
```hemlock
async fn producer(ch) {
    let i = 0;
    while (i < 100) {
        let obj = { value: i, data: "test string " + typeof(i) };
        ch.send(obj);  // obj should be retained by channel
        // obj can go out of scope safely
        i = i + 1;
    }
    ch.close();
}

async fn consumer(ch) {
    let count = 0;
    while (true) {
        let msg = ch.recv();
        if (msg == null) { break; }
        print("Received: " + typeof(msg.value));
        // msg should remain valid
        count = count + 1;
    }
    return count;
}

let ch = channel(10);
let p = spawn(producer, ch);
let c = spawn(consumer, ch);

join(p);
let total = join(c);
print("Total received: " + typeof(total));
```

**Success Criteria:**
- Test runs without crashes
- All 100 messages received correctly
- Valgrind shows no use-after-free or memory leaks
- Concurrent access is safe

---

## Phase 3: Manual Memory Management

### Fix 3.1: Redesign builtin_free() for Reference-Counted Types
**Issue:** #5 - builtin_free() double-free risk
**Severity:** HIGH
**Complexity:** Complex (requires design decision)

**Files to Modify:**
1. `src/interpreter/builtins/memory.c`

**Design Options:**

**Option A: Disallow free() on shared references**
```c
// Only allow free if ref_count <= 1
if (args[0].type == VAL_BUFFER) {
    Buffer *buf = args[0].as.as_buffer;
    if (buf->ref_count > 1) {
        fprintf(stderr, "Runtime error: Cannot free buffer with %d active references\n",
                buf->ref_count);
        exit(1);
    }
    // Safe to free
    free(buf->data);
    free(buf);
    return val_null();
}
```

**Option B: Only track manually freed for non-ref-counted types**
```c
// Remove ref_count manipulation, just add to manually_freed list
if (args[0].type == VAL_BUFFER) {
    Buffer *buf = args[0].as.as_buffer;
    // Add to manually freed list to prevent value_release from double-freeing
    add_manually_freed_pointer(buf);
    free(buf->data);
    free(buf);
    return val_null();
}
```

**Option C: Make free() decrement ref_count instead of freeing**
```c
// Just call value_release() - let reference counting handle it
if (args[0].type == VAL_BUFFER) {
    value_release(args[0]);
    return val_null();
}
```

**Recommendation:** **Option A** (disallow shared frees)
- Most explicit and safe
- Matches Hemlock's "explicit over implicit" philosophy
- Prevents accidental double-frees
- User must ensure exclusive ownership before manual free

**Implementation (Option A):**

**Location: memory.c lines 71-78**
```c
// Current:
else if (args[0].type == VAL_BUFFER) {
    // Manually free and set ref_count to 0 to prevent double-free
    if (args[0].as.as_buffer->ref_count > 0) {
        args[0].as.as_buffer->ref_count = 0;
        free(args[0].as.as_buffer->data);
        free(args[0].as.as_buffer);
    }
    return val_null();
}

// Fixed:
else if (args[0].type == VAL_BUFFER) {
    Buffer *buf = args[0].as.as_buffer;

    // Safety check: don't allow free on shared references
    if (buf->ref_count > 1) {
        fprintf(stderr, "Runtime error: Cannot free buffer with %d active references. "
                "Use value_release() or ensure exclusive ownership.\n", buf->ref_count);
        exit(1);
    }

    // Mark as manually freed to prevent double-free from value_release
    if (buf->ref_count == 1) {
        add_manually_freed_pointer(buf);
    }

    free(buf->data);
    free(buf);
    return val_null();
}
```

**Similar changes needed for:**
- VAL_OBJECT (same ref_count check)
- VAL_ARRAY (same ref_count check)

**Testing:**
- Test free() on exclusively owned buffers (ref_count == 0 or 1)
- Test that free() on shared buffers (ref_count > 1) fails with error
- Verify no double-free with Valgrind
- Test interaction with array.push(), object fields, etc.

**Test Script:**
```hemlock
// Should succeed: exclusive ownership
let buf1 = buffer(10);
free(buf1);  // OK - ref_count == 0

// Should fail: shared ownership
let buf2 = buffer(10);
let arr = [buf2];  // ref_count becomes 1
free(buf2);  // ERROR - ref_count > 1, cannot free

// Should succeed after removing from array
let buf3 = buffer(10);
let arr2 = [buf3];
arr2.remove(0);  // ref_count back to 0
free(buf3);  // OK - exclusive ownership
```

**Success Criteria:**
- Exclusive ownership frees succeed
- Shared ownership frees fail with clear error message
- No double-frees in Valgrind
- Documentation updated to explain new behavior

---

## Phase 4: Enhancements

### Fix 4.1: Add FFI String Return Type Support
**Issue:** #6 - FFI missing string return type
**Severity:** MEDIUM
**Complexity:** Simple

**Files to Modify:**
1. `src/interpreter/ffi.c`

**Implementation:**

**Location: ffi.c line 167 (add case for TYPE_STRING)**
```c
// Current (lines ~160-170):
case TYPE_F64:
    return val_f64(*(double*)c_value);
// TYPE_STRING case missing - falls through to default!
default:
    fprintf(stderr, "Error: Cannot convert C type to Hemlock: %d\n", type->kind);
    exit(1);

// Fixed (add before default):
case TYPE_F64:
    return val_f64(*(double*)c_value);
case TYPE_STRING: {
    char *str = *(char**)c_value;
    if (str == NULL) {
        return val_null();
    }
    return val_string(str);
}
default:
    fprintf(stderr, "Error: Cannot convert C type to Hemlock: %d\n", type->kind);
    exit(1);
```

**Testing:**
- Test FFI call to C function returning char*
- Test NULL string return
- Verify string memory is properly managed

**Test Script:**
```hemlock
// Assuming libffi_test.so has get_string() function
let lib = load_library("./tests/ffi/libffi_test.so");
let get_string = lib.get_function("get_string", [], "string");
let result = get_string();
print(result);  // Should print the returned string
```

**Success Criteria:**
- FFI can call string-returning C functions
- NULL strings handled correctly
- No memory leaks in Valgrind

---

### Fix 4.2: Add Thread-Safety to manually_freed_pointers
**Issue:** #7 - Race condition in global list
**Severity:** MEDIUM
**Complexity:** Medium

**Files to Modify:**
1. `src/interpreter/environment.c`

**Implementation:**

**Add mutex for manually_freed_pointers:**
```c
// At top of file (after includes):
static pthread_mutex_t manually_freed_mutex = PTHREAD_MUTEX_INITIALIZER;

// Wrap all accesses to manually_freed_pointers:

// In add_manually_freed_pointer() (line ~55):
void add_manually_freed_pointer(void *ptr) {
    pthread_mutex_lock(&manually_freed_mutex);  // ADD

    if (manually_freed_count >= manually_freed_capacity) {
        manually_freed_capacity *= 2;
        manually_freed_pointers = realloc(manually_freed_pointers,
                                         manually_freed_capacity * sizeof(void*));
    }
    manually_freed_pointers[manually_freed_count++] = ptr;

    pthread_mutex_unlock(&manually_freed_mutex);  // ADD
}

// In is_manually_freed_pointer() (line ~65):
bool is_manually_freed_pointer(void *ptr) {
    pthread_mutex_lock(&manually_freed_mutex);  // ADD

    for (int i = 0; i < manually_freed_count; i++) {
        if (manually_freed_pointers[i] == ptr) {
            pthread_mutex_unlock(&manually_freed_mutex);  // ADD
            return true;
        }
    }

    pthread_mutex_unlock(&manually_freed_mutex);  // ADD
    return false;
}
```

**Testing:**
- Run concurrent tests with multiple threads calling free()
- Stress test with spawn/detach creating many threads
- Verify no race conditions with thread sanitizer (tsan)

**Success Criteria:**
- No race conditions detected by thread sanitizer
- Concurrent free() operations work correctly
- No crashes under concurrent load

---

## Testing Strategy

### Per-Fix Testing
Each fix has specific tests outlined above.

### Integration Testing
After all fixes:
1. Run full test suite: `make test`
2. Run under Valgrind: `valgrind --leak-check=full ./hemlock tests/...`
3. Run under Thread Sanitizer: `make clean && make CFLAGS="-fsanitize=thread" && ./hemlock tests/async/*`
4. Stress test with high concurrency and memory pressure

### Regression Prevention
- Add new tests for each fixed bug
- Document expected behavior in test comments
- Add to CI/CD pipeline if available

---

## Implementation Order

### Recommended Execution Order:

1. **Phase 1.1** - Initialize ref_count (30 min)
   - Low risk, high impact
   - Prerequisite for other fixes

2. **Phase 1.2** - Array unshift/insert (15 min)
   - Low risk, simple addition
   - Independent of other fixes

3. **Phase 2.1** - Channel value_release (15 min)
   - Simple addition
   - Independent fix

4. **Phase 2.2** - Channel send/recv (30 min)
   - Medium complexity
   - Requires careful testing
   - Depends on 1.1 being done first

5. **Phase 3.1** - builtin_free redesign (1 hour)
   - Requires design decision
   - Most complex change
   - Should be done after ref_count fixes are stable

6. **Phase 4.1** - FFI string support (15 min)
   - Independent enhancement
   - Low risk

7. **Phase 4.2** - Thread-safety (30 min)
   - Independent enhancement
   - Should be tested with full concurrency suite

**Total Estimated Time: 3-4 hours**

---

## Risk Mitigation

### High-Risk Changes:
- Phase 2.2 (Channel send/recv) - Affects concurrency
- Phase 3.1 (builtin_free) - Changes semantics

### Mitigation Strategies:
1. **Incremental commits** - One fix per commit
2. **Test after each fix** - Don't proceed if tests fail
3. **Valgrind validation** - Required for all memory fixes
4. **Backup branch** - Keep current state in separate branch
5. **Code review** - Get second pair of eyes on Phase 2.2 and 3.1

---

## Success Criteria (Overall)

### Must Have:
- ✅ All existing tests pass
- ✅ No memory leaks in Valgrind
- ✅ No use-after-free in Valgrind
- ✅ No double-free errors
- ✅ Concurrent tests pass without races

### Should Have:
- ✅ New tests added for each fixed bug
- ✅ Documentation updated (if semantics changed)
- ✅ Performance not degraded

### Nice to Have:
- ✅ Thread sanitizer clean
- ✅ Additional stress tests
- ✅ Benchmark improvements

---

## Rollback Plan

If a fix causes unexpected issues:

1. **Immediate:** Revert the specific commit
2. **Investigate:** Run tests to identify root cause
3. **Fix:** Address the issue
4. **Re-test:** Ensure fix works correctly
5. **Re-apply:** Commit the corrected fix

Each fix is independent and can be reverted without affecting others (except Phase 2.2 should be reverted with Phase 1.1 if necessary).

---

## Documentation Updates

After fixes are complete, update:

1. **CLAUDE.md** - Update "Current Limitations" and "Known Issues" sections
2. **ANALYSIS.md** - Add "FIXED" markers to resolved issues
3. **CHANGELOG.md** - Document all bug fixes
4. **Test documentation** - Add new test descriptions

---

**End of Fix Plan**
