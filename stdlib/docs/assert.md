# @stdlib/assert - Assertion Utilities

Provides assertion functions for testing and defensive programming. All assertions throw an error when the condition is not met.

## Import

```hemlock
import { assert, assert_eq, assert_ne, assert_throws } from "@stdlib/assert";
import { assert_type, assert_null, assert_contains } from "@stdlib/assert";
```

## Basic Assertions

### assert(condition, message?)

Assert that a condition is true.

```hemlock
assert(x > 0, "x must be positive");
assert(user != null);
```

### assert_false(condition, message?)

Assert that a condition is false.

```hemlock
assert_false(is_empty, "Collection should not be empty");
```

### assert_truthy(value, message?) / assert_falsy(value, message?)

Assert value truthiness.

```hemlock
assert_truthy(result);     // Not null, false, 0, or ""
assert_falsy(error_code);  // Is null, false, 0, or ""
```

## Equality Assertions

### assert_eq(actual, expected, message?)

Assert deep equality. Works with primitives, arrays, and objects.

```hemlock
assert_eq(1 + 1, 2);
assert_eq(user["name"], "Alice");
assert_eq([1, 2, 3], [1, 2, 3]);
assert_eq({a: 1, b: 2}, {a: 1, b: 2});
```

### assert_ne(actual, expected, message?)

Assert values are not equal.

```hemlock
assert_ne(result, null);
assert_ne(status, "error");
```

### assert_strict_eq(actual, expected, message?)

Assert strict equality (same type and value).

```hemlock
assert_strict_eq(42, 42);  // Passes (both i32)
// assert_strict_eq(42, 42.0);  // Fails (i32 vs f64)
```

## Comparison Assertions

```hemlock
assert_gt(5, 3);       // actual > expected
assert_gte(5, 5);      // actual >= expected
assert_lt(3, 5);       // actual < expected
assert_lte(5, 5);      // actual <= expected

assert_in_range(value, 1, 100);  // value in [1, 100]
```

## Type Assertions

### assert_type(value, type_name, message?)

```hemlock
assert_type(42, "i32");
assert_type("hello", "string");
assert_type([1, 2], "array");
assert_type({a: 1}, "object");
```

### Type-Specific Assertions

```hemlock
assert_null(value);       // value is null
assert_not_null(value);   // value is not null
assert_string(value);     // typeof(value) == "string"
assert_number(value);     // any numeric type
assert_bool(value);       // typeof(value) == "bool"
assert_array(value);      // typeof(value) == "array"
assert_object(value);     // typeof(value) == "object"
assert_function(value);   // typeof(value) == "function"
```

## Collection Assertions

### assert_contains(collection, value, message?)

Assert that array or string contains a value.

```hemlock
assert_contains("hello world", "world");
assert_contains([1, 2, 3], 2);
assert_contains(users, target_user);  // deep comparison for objects
```

### assert_not_contains(collection, value, message?)

```hemlock
assert_not_contains(errors, critical_error);
```

### Length Assertions

```hemlock
assert_empty([]);              // length == 0
assert_not_empty(results);     // length > 0
assert_length(items, 5);       // length == 5
```

### assert_has_key(obj, key, message?)

```hemlock
assert_has_key(config, "database");
assert_has_key(user, "email");
```

## String Assertions

```hemlock
assert_starts_with(url, "https://");
assert_ends_with(filename, ".hml");
assert_matches(email, "^[a-z]+@[a-z]+\\.[a-z]+$");
```

## Exception Assertions

### assert_throws(func, expected_message?, message?)

Assert that a function throws an error.

```hemlock
// Assert any exception is thrown
assert_throws(fn() {
    throw "error";
});

// Assert exception with specific message
assert_throws(fn() {
    throw "invalid input: x";
}, "invalid input");
```

### assert_no_throw(func, message?)

Assert that a function does not throw.

```hemlock
assert_no_throw(fn() {
    return process_data(input);
});
```

## Numeric Assertions

### assert_approx_eq(actual, expected, tolerance?, message?)

Assert approximate equality for floating-point values.

```hemlock
assert_approx_eq(3.14159, 3.14160, 0.001);  // Passes
assert_approx_eq(result, expected, 0.0001);
```

### Special Value Assertions

```hemlock
assert_nan(0.0 / 0.0);          // Value is NaN
assert_not_nan(result);         // Value is not NaN
assert_finite(value);           // Not NaN or Infinity
assert_positive(count);         // > 0
assert_negative(delta);         // < 0
assert_zero(error_count);       // == 0
```

## Array Assertions

### assert_array_eq(actual, expected, message?)

Deep array comparison with detailed error on mismatch.

```hemlock
assert_array_eq([1, 2, 3], [1, 2, 3]);
// Error: "element 1 differs - expected 2, got 5"
```

### assert_includes_all(arr, expected, message?)

```hemlock
let required = ["name", "email"];
assert_includes_all(fields, required);
```

## Examples

### Unit Testing

```hemlock
import { assert_eq, assert_throws, assert_not_null } from "@stdlib/assert";

fn test_add() {
    assert_eq(add(2, 3), 5);
    assert_eq(add(-1, 1), 0);
    assert_eq(add(0, 0), 0);
    print("test_add: PASSED");
}

fn test_divide() {
    assert_eq(divide(10, 2), 5);
    assert_throws(fn() { divide(1, 0); }, "division by zero");
    print("test_divide: PASSED");
}

fn test_parse_user() {
    let user = parse_user('{"name": "Alice", "age": 30}');
    assert_not_null(user);
    assert_eq(user["name"], "Alice");
    assert_eq(user["age"], 30);
    print("test_parse_user: PASSED");
}

test_add();
test_divide();
test_parse_user();
```

### Defensive Programming

```hemlock
import { assert, assert_not_null, assert_type, assert_in_range } from "@stdlib/assert";

fn process_order(order) {
    // Validate input
    assert_not_null(order, "Order cannot be null");
    assert_type(order, "object", "Order must be an object");
    assert(order["items"] != null, "Order must have items");
    assert_not_empty(order["items"], "Order must have at least one item");

    let i = 0;
    while (i < order["items"].length) {
        let item = order["items"][i];
        assert_in_range(item["quantity"], 1, 999, "Invalid quantity");
        assert_positive(item["price"], "Price must be positive");
        i = i + 1;
    }

    // Process order...
}
```

### API Response Validation

```hemlock
import { assert_eq, assert_type, assert_contains, assert_has_key } from "@stdlib/assert";

fn validate_response(response) {
    assert_type(response, "object");
    assert_has_key(response, "status");
    assert_has_key(response, "data");

    assert_eq(response["status"], "success");
    assert_type(response["data"], "array");

    let i = 0;
    while (i < response["data"].length) {
        let item = response["data"][i];
        assert_has_key(item, "id");
        assert_has_key(item, "name");
        i = i + 1;
    }
}
```

## Error Messages

All assertions produce descriptive error messages:

```
Values not equal: expected 5, got 3
Type mismatch: expected string, got i32
Value out of range: 150 not in [1, 100]
String does not start with prefix: 'hello' does not start with 'http://'
Expected exception: no exception thrown
Arrays not equal: element 2 differs - expected "c", got "x"
```

## See Also

- [@stdlib/testing](./testing.md) - Full testing framework with describe/test/expect
