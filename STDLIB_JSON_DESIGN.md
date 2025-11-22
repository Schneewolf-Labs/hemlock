# JSON Standard Library Module - Design Document

## Overview

The `@stdlib/json` module provides comprehensive JSON functionality for Hemlock, building on the existing built-in `serialize()`/`deserialize()` methods with advanced features for parsing, formatting, validation, querying, and manipulation.

## Design Philosophy

Following Hemlock's core principles:
- **Explicit over implicit**: Clear parsing options, no magic conversions
- **Manual control**: Users choose formatting, validation strictness
- **No hidden behavior**: All operations and their costs are visible
- **Unsafe is a feature**: Raw parsing available, safe wrappers encouraged
- **Honest about tradeoffs**: Performance vs safety options documented

## Module API

### 1. Core Parsing & Serialization

```hemlock
import { parse, stringify, parse_file, stringify_file } from "@stdlib/json";

// Parse JSON string to value (wrapper around .deserialize())
let obj = parse('{"x":10,"y":20}');
print(obj.x);  // 10

// Serialize value to JSON string (wrapper around .serialize())
let json = stringify({ name: "Alice", age: 30 });
// {"name":"Alice","age":30}

// Parse JSON from file
let config = parse_file("config.json");

// Write JSON to file
stringify_file("output.json", data);
```

**Error handling:**
```hemlock
try {
    let obj = parse('{"invalid json}');
} catch (e) {
    print("Parse error: " + e);
}
```

### 2. Pretty Printing

```hemlock
import { pretty, pretty_file } from "@stdlib/json";

let data = { name: "Alice", items: [1, 2, 3], nested: { x: 10 } };

// Pretty print with indentation
let formatted = pretty(data);
print(formatted);
/*
{
  "name": "Alice",
  "items": [
    1,
    2,
    3
  ],
  "nested": {
    "x": 10
  }
}
*/

// Custom indentation
let formatted2 = pretty(data, 4);  // 4 spaces
let formatted3 = pretty(data, "\t");  // Tabs

// Pretty print to file
pretty_file("formatted.json", data, 2);
```

### 3. JSON Path Access (JSONPath-like)

```hemlock
import { get, set, has, delete } from "@stdlib/json";

let doc = {
    user: {
        name: "Alice",
        address: {
            city: "NYC",
            zip: 10001
        }
    },
    items: [1, 2, 3]
};

// Get value by path (dot notation)
let name = get(doc, "user.name");  // "Alice"
let city = get(doc, "user.address.city");  // "NYC"
let first = get(doc, "items.0");  // 1

// Set value by path
set(doc, "user.name", "Bob");
set(doc, "items.1", 99);

// Check if path exists
if (has(doc, "user.address.zip")) {
    print("Has zip code");
}

// Delete by path
delete(doc, "user.address.zip");

// Safe access with default
let phone = get(doc, "user.phone", "(none)");  // "(none)"
```

### 4. Validation

```hemlock
import { validate, is_valid } from "@stdlib/json";

// Check if string is valid JSON (doesn't parse)
if (is_valid('{"x":10}')) {
    print("Valid JSON");
}

// Validate and get detailed error
let result = validate('{"unclosed": ');
if (!result.valid) {
    print("Error at line " + typeof(result.line) + ": " + result.message);
}
```

### 5. Merge & Patch Operations

```hemlock
import { merge, patch } from "@stdlib/json";

let base = { x: 10, y: 20, nested: { a: 1 } };
let update = { y: 30, z: 40, nested: { b: 2 } };

// Deep merge (combines nested objects)
let merged = merge(base, update);
// { x: 10, y: 30, z: 40, nested: { a: 1, b: 2 } }

// Shallow merge (replaces nested objects)
let patched = patch(base, update);
// { x: 10, y: 30, z: 40, nested: { b: 2 } }

// Merge multiple objects
let combined = merge(obj1, obj2, obj3);
```

### 6. Type Checking & Utilities

```hemlock
import {
    is_object, is_array, is_string, is_number, is_bool, is_null,
    type_of, clone
} from "@stdlib/json";

let data = parse('{"items":[1,2,3]}');

// Type checking
if (is_object(data)) {
    print("Root is object");
}

if (is_array(data.items)) {
    print("items is array");
}

// Get JSON type as string
print(type_of(data.items));  // "array"

// Deep clone (creates independent copy)
let copy = clone(data);
copy.items.push(4);  // doesn't affect original
```

### 7. Query & Filter

```hemlock
import { filter, map, find_all } from "@stdlib/json";

let data = {
    users: [
        { name: "Alice", age: 30, active: true },
        { name: "Bob", age: 25, active: false },
        { name: "Carol", age: 35, active: true }
    ]
};

// Filter array by predicate
let active_users = filter(data.users, fn(user) {
    return user.active;
});

// Map array
let names = map(data.users, fn(user) {
    return user.name;
});
// ["Alice", "Bob", "Carol"]

// Find all values matching path pattern
let ages = find_all(data, "users.*.age");
// [30, 25, 35]
```

### 8. Comparison & Diff

```hemlock
import { equals, diff } from "@stdlib/json";

let obj1 = { x: 10, y: 20, items: [1, 2, 3] };
let obj2 = { x: 10, y: 30, items: [1, 2, 3] };

// Deep equality check
if (equals(obj1, obj2)) {
    print("Objects are equal");
} else {
    print("Objects differ");
}

// Get differences
let changes = diff(obj1, obj2);
// [{ path: "y", old: 20, new: 30 }]
```

### 9. Parsing Options

```hemlock
import { parse_strict, parse_lenient } from "@stdlib/json";

// Strict mode: standard JSON only
let obj1 = parse_strict('{"x":10}');  // OK
let obj2 = parse_strict("{x:10}");    // Error: unquoted keys

// Lenient mode: allows common JSON5-like extensions
let obj3 = parse_lenient("{x:10,}");  // OK: unquoted keys, trailing commas
let obj4 = parse_lenient("// comment\n{x:10}");  // OK: comments
```

### 10. Streaming for Large Files

```hemlock
import { JsonReader } from "@stdlib/json";

// Stream large JSON files without loading entire file
let reader = JsonReader("large.json");
defer reader.close();

// Read top-level keys incrementally
while (reader.has_next()) {
    let key = reader.next_key();
    let value = reader.next_value();
    process(key, value);
}
```

## Implementation Structure

### File Organization

```
stdlib/
├── json.hml              # Main module implementation
└── docs/
    └── json.md           # API documentation
```

### Module Structure

```hemlock
// @stdlib/json implementation

// Re-export built-in functionality
fn parse(json_str: string) {
    try {
        return json_str.deserialize();
    } catch (e) {
        throw "JSON parse error: " + e;
    }
}

fn stringify(value) {
    try {
        return value.serialize();
    } catch (e) {
        throw "JSON stringify error: " + e;
    }
}

// Pretty printing
fn pretty(value, indent?) {
    if (indent == null) {
        indent = 2;
    }
    // Implementation using recursive formatting
    return format_value(value, 0, indent);
}

// Path access
fn get(obj, path: string, default_val?) {
    let parts = path.split(".");
    let current = obj;
    let i = 0;

    while (i < parts.length) {
        if (current == null) {
            return default_val;
        }

        let part = parts[i];
        // Handle array indices (e.g., "items.0")
        if (is_array_index(part)) {
            let idx = parse_int(part);
            current = current[idx];
        } else {
            current = current[part];
        }

        i = i + 1;
    }

    if (current == null && default_val != null) {
        return default_val;
    }

    return current;
}

// Deep clone
fn clone(value) {
    // Serialize and deserialize for deep copy
    return parse(stringify(value));
}

// Deep merge
fn merge(base, update) {
    if (!is_object(base) || !is_object(update)) {
        return update;
    }

    let result = clone(base);

    // Iterate over update fields
    let keys = object_keys(update);
    let i = 0;
    while (i < keys.length) {
        let key = keys[i];

        if (is_object(result[key]) && is_object(update[key])) {
            result[key] = merge(result[key], update[key]);
        } else {
            result[key] = update[key];
        }

        i = i + 1;
    }

    return result;
}

// Export all public functions
```

## Error Handling

All JSON operations use Hemlock's try/catch mechanism:

```hemlock
try {
    let obj = parse(malformed_json);
} catch (e) {
    print("Parse failed: " + e);
}

try {
    let circular = { };
    circular.self = circular;
    stringify(circular);
} catch (e) {
    print("Stringify failed: " + e);
    // "serialize() detected circular reference"
}
```

**Error types:**
- Parse errors: Invalid JSON syntax
- Path errors: Invalid path expression
- Type errors: Operation not supported for type
- Circular reference errors: Detected during serialization

## Performance Considerations

### Fast Operations
- `parse()` / `stringify()` - Built-in C implementation
- `clone()` - Uses built-in serialize/deserialize
- `get()` / `set()` - Direct property access

### Slower Operations
- `pretty()` - Recursive formatting with string concatenation
- `merge()` - Deep cloning and recursion
- `diff()` - Deep comparison
- `filter()` / `map()` - Array iteration with callbacks

### Memory Usage
- `clone()` - Creates full independent copy
- `merge()` - Allocates new object with combined data
- Streaming - Processes incrementally (low memory)

## Usage Examples

### Configuration File Handling

```hemlock
import { parse_file, pretty_file, get, set } from "@stdlib/json";

// Load config
let config = parse_file("config.json");

// Read nested settings
let port = get(config, "server.port", 8080);
let db_host = get(config, "database.host", "localhost");

// Update settings
set(config, "server.port", 9000);
set(config, "database.pool_size", 10);

// Save with formatting
pretty_file("config.json", config, 2);
```

### API Response Processing

```hemlock
import { parse, get, filter, map } from "@stdlib/json";

let response = parse(http_response_body);

// Extract data
let users = get(response, "data.users", []);

// Filter and transform
let active_users = filter(users, fn(u) { return u.active; });
let names = map(active_users, fn(u) { return u.name; });

print("Active users: " + names.join(", "));
```

### Configuration Merging

```hemlock
import { parse_file, merge, pretty_file } from "@stdlib/json";

// Load base config and environment-specific overrides
let base_config = parse_file("config.base.json");
let prod_config = parse_file("config.production.json");

// Merge (production overrides base)
let final_config = merge(base_config, prod_config);

// Save merged config
pretty_file("config.json", final_config);
```

### Data Validation

```hemlock
import { parse, is_object, is_array, has } from "@stdlib/json";

fn validate_user(user_json: string): bool {
    try {
        let user = parse(user_json);

        // Check structure
        if (!is_object(user)) {
            throw "User must be object";
        }

        // Check required fields
        if (!has(user, "name") || !has(user, "email")) {
            throw "Missing required fields";
        }

        return true;
    } catch (e) {
        print("Validation error: " + e);
        return false;
    }
}
```

## Testing Strategy

```
tests/stdlib_json/
├── parse_test.hml              # Basic parsing
├── stringify_test.hml          # Basic serialization
├── pretty_test.hml             # Pretty printing
├── path_test.hml               # get/set/has/delete
├── merge_test.hml              # merge/patch operations
├── validation_test.hml         # validate/is_valid
├── clone_test.hml              # Deep cloning
├── filter_test.hml             # filter/map operations
├── equals_test.hml             # Deep equality
├── errors_test.hml             # Error handling
└── integration_test.hml        # Real-world scenarios
```

## Future Enhancements

### Phase 2
- **JSON Schema validation** - Validate against JSON Schema
- **JSON Patch (RFC 6902)** - Standard patch operations
- **JSON Pointer (RFC 6901)** - Standard path syntax
- **Streaming writer** - Write large JSON incrementally

### Phase 3
- **JSON5 support** - Comments, trailing commas, unquoted keys
- **YAML parsing** - Convert YAML to JSON
- **CSV to JSON** - Tabular data conversion
- **XML to JSON** - XML parsing and conversion

## Compatibility

- **Requires:** Hemlock v0.1+ (object serialize/deserialize)
- **Dependencies:** None (pure Hemlock)
- **Platform:** All (platform-independent)

## Documentation

Full API reference in `stdlib/docs/json.md` following the pattern of existing stdlib modules.

---

## Design Decisions

### Why build on serialize/deserialize?
- Leverage existing, tested C implementation
- Avoid duplicating parsing logic
- Focus module on higher-level utilities

### Why include path access (get/set)?
- Common pattern in JSON manipulation
- Avoids verbose nested property access
- Safe defaults for missing paths

### Why deep clone via serialize/deserialize?
- Reuses battle-tested code
- Handles all JSON types correctly
- Clear performance tradeoff (explicit)

### Why no JSON Schema in v1?
- Keep initial release focused
- Schema validation is complex
- Can be added as separate module

### Why manual file operations?
- User controls when I/O happens
- Can batch multiple operations
- Explicit about file system access

---

This design provides a comprehensive, production-ready JSON module that follows Hemlock's philosophy while delivering practical functionality for real-world applications.
