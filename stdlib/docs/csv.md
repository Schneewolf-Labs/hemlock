# @stdlib/csv - CSV Parsing and Generation

The `csv` module provides functions for reading and writing CSV (Comma-Separated Values) data.

## Quick Start

```hemlock
import { parse, stringify } from "@stdlib/csv";

// Parse CSV
let text = "name,age\nAlice,30\nBob,25";
let rows = parse(text);
print(rows[1][0]);  // "Alice"

// Generate CSV
let data = [["name", "age"], ["Alice", "30"]];
let csv = stringify(data);
print(csv);  // "name,age\nAlice,30"
```

## API Reference

### parse(text, options?): array

Parse a CSV string into an array of rows.

**Options:**
- `delimiter: string` - Field delimiter (default: ",")
- `quote: string` - Quote character (default: "\"")
- `skip_header: bool` - Skip first row (default: false)

```hemlock
import { parse } from "@stdlib/csv";

let csv = "a,b,c\n1,2,3";
let rows = parse(csv);
print(rows[0]);  // ["a", "b", "c"]
print(rows[1]);  // ["1", "2", "3"]

// Tab-separated values
let tsv = "a\tb\tc";
let tsv_rows = parse(tsv, { delimiter: "\t" });
```

### parse_objects(text, options?): array

Parse CSV with headers into an array of objects.

```hemlock
import { parse_objects } from "@stdlib/csv";

let csv = "name,age,city\nAlice,30,NYC\nBob,25,LA";
let people = parse_objects(csv);

print(people[0]["name"]);  // "Alice"
print(people[0]["age"]);   // "30"
```

### parse_row(line, options?): array

Parse a single CSV line.

```hemlock
import { parse_row } from "@stdlib/csv";

let fields = parse_row("a,b,c,d");
print(fields);  // ["a", "b", "c", "d"]
```

### stringify(rows, options?): string

Convert array of rows to CSV string.

**Options:**
- `delimiter: string` - Field delimiter (default: ",")
- `quote: string` - Quote character (default: "\"")
- `line_ending: string` - Line ending (default: "\n")

```hemlock
import { stringify } from "@stdlib/csv";

let data = [
    ["name", "value"],
    ["foo", "100"],
    ["bar", "200"]
];
let csv = stringify(data);
print(csv);
// name,value
// foo,100
// bar,200
```

### stringify_objects(objects, headers, options?): string

Convert array of objects to CSV string.

```hemlock
import { stringify_objects } from "@stdlib/csv";

let people = [
    { name: "Alice", age: 30 },
    { name: "Bob", age: 25 }
];
let csv = stringify_objects(people, ["name", "age"]);
print(csv);
// name,age
// Alice,30
// Bob,25
```

### stringify_row(row, options?): string

Convert a single row to CSV line.

```hemlock
import { stringify_row } from "@stdlib/csv";

let line = stringify_row(["hello", "world"]);
print(line);  // "hello,world"

// Fields with commas are quoted
let quoted = stringify_row(["normal", "has,comma"]);
print(quoted);  // "normal,\"has,comma\""
```

### get_column(rows, index): array

Extract a column from CSV data.

```hemlock
import { parse, get_column } from "@stdlib/csv";

let csv = "a,b,c\n1,2,3\n4,5,6";
let rows = parse(csv);
let col_b = get_column(rows, 1);
print(col_b);  // ["b", "2", "5"]
```

### row_count(rows): i32

Get number of rows.

### column_count(rows): i32

Get number of columns (from first row).

## Handling Special Cases

### Quoted Fields

Fields containing delimiters, quotes, or newlines are automatically quoted:

```hemlock
import { parse, stringify_row } from "@stdlib/csv";

// Parsing quoted fields
let csv = "name,desc\nAlice,\"Hello, World\"";
let rows = parse(csv);
print(rows[1][1]);  // "Hello, World"

// Generating quoted fields
let line = stringify_row(["name", "has,comma"]);
print(line);  // "name,\"has,comma\""
```

### Escaped Quotes

Quotes within quoted fields are escaped by doubling:

```hemlock
import { parse } from "@stdlib/csv";

let csv = "text\n\"He said \"\"hello\"\"\"";
let rows = parse(csv);
print(rows[1][0]);  // He said "hello"
```

### Windows Line Endings

Both `\n` (Unix) and `\r\n` (Windows) line endings are handled.

## Examples

### Reading a CSV File

```hemlock
import { parse_objects } from "@stdlib/csv";
import { read_file } from "@stdlib/fs";

let content = read_file("users.csv");
let users = parse_objects(content);

let i = 0;
while (i < users.length) {
    print(users[i]["name"] + ": " + users[i]["email"]);
    i = i + 1;
}
```

### Writing a CSV File

```hemlock
import { stringify } from "@stdlib/csv";
import { write_file } from "@stdlib/fs";

let data = [
    ["id", "name", "score"],
    ["1", "Alice", "95"],
    ["2", "Bob", "87"]
];

let csv = stringify(data);
write_file("scores.csv", csv);
```

## See Also

- [@stdlib/fs](fs.md) - File system operations
- [@stdlib/json](json.md) - JSON parsing
