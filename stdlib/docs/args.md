# @stdlib/args - Command-Line Argument Parsing

The `args` module provides utilities for parsing command-line arguments, including flags, options with values, and positional arguments.

## Quick Start

```hemlock
import { parse, has_flag, get_option, get_positionals } from "@stdlib/args";

// Parse command-line arguments
let parsed = parse(args);

// Check flags
if (has_flag(parsed, "verbose")) {
    print("Verbose mode enabled");
}

// Get options
let output = get_option(parsed, "output", "out.txt");
print("Output file: " + output);

// Get positional arguments
let files = get_positionals(parsed);
print("Files to process: " + files.join(", "));
```

## Understanding Command-Line Arguments

Hemlock provides a built-in `args` global array containing command-line arguments:

```hemlock
// Running: ./hemlock script.hml file1.txt file2.txt --verbose
print(args[0]);  // script.hml
print(args[1]);  // file1.txt
print(args[2]);  // file2.txt
print(args[3]);  // --verbose
```

This module helps parse these into structured data.

## API Reference

### parse(argv, options?): object

Parse command-line arguments into a structured object.

**Supported formats:**
- `--flag` or `-f` - Boolean flags
- `--option=value` - Options with `=` separator
- `--option value` - Options with space separator (requires config)
- `-o value` - Short options with value (requires config)
- `-abc` - Combined short flags
- `--` - End of options marker
- `positional` - Positional arguments

**Parameters:**
- `argv: array<string>` - Command-line arguments (typically the global `args`)
- `options: object` - Optional configuration:
  - `booleans: array<string>` - Names of boolean-only flags
  - `strings: array<string>` - Names of options that take values

**Returns:** `object` with:
- `script: string` - Script name (first argument)
- `flags: object` - Boolean flags (key: true)
- `options: object` - Options with values (key: value)
- `positionals: array` - Positional arguments
- `_raw: array` - Original arguments

```hemlock
import { parse } from "@stdlib/args";

// ./hemlock script.hml --verbose --output=file.txt input.txt
let parsed = parse(args);

print(parsed.script);           // script.hml
print(parsed.flags["verbose"]); // true
print(parsed.options["output"]); // file.txt
print(parsed.positionals[0]);   // input.txt
```

**With configuration:**

```hemlock
// Tell parser that "output" and "o" take values
let parsed = parse(args, {
    strings: ["output", "o"]
});

// Now --output value and -o value work
// ./hemlock script.hml --output file.txt
print(parsed.options["output"]); // file.txt
```

### has_flag(parsed, name): bool

Check if a flag is set.

```hemlock
import { parse, has_flag } from "@stdlib/args";

let parsed = parse(args);

if (has_flag(parsed, "verbose") || has_flag(parsed, "v")) {
    print("Verbose mode");
}

if (has_flag(parsed, "dry-run")) {
    print("Dry run mode");
}
```

### get_option(parsed, name, default_value?): string

Get an option value with optional default.

```hemlock
import { parse, get_option } from "@stdlib/args";

let parsed = parse(args);

let output = get_option(parsed, "output", "out.txt");
let port = get_option(parsed, "port", "8080");
let config = get_option(parsed, "config");  // null if not set
```

### get_positionals(parsed): array

Get positional arguments.

```hemlock
import { parse, get_positionals } from "@stdlib/args";

let parsed = parse(args);
let files = get_positionals(parsed);

let i = 0;
while (i < files.length) {
    print("Processing: " + files[i]);
    i = i + 1;
}
```

### get_script(parsed): string

Get the script name (first argument).

```hemlock
import { parse, get_script } from "@stdlib/args";

let parsed = parse(args);
print("Running: " + get_script(parsed));
```

### ArgParser(name, description?): object

Create a builder-style argument parser for defining CLI interfaces.

```hemlock
import { ArgParser } from "@stdlib/args";

let parser = ArgParser("myapp", "A sample application")
    .version("1.0.0")
    .flag("v", "verbose", "Enable verbose output")
    .flag("d", "debug", "Enable debug mode")
    .option("o", "output", "Output file path", "out.txt")
    .option("p", "port", "Port number", "8080")
    .positional("input", "Input file", true)
    .positional("extra", "Extra file", false);

let result = parser.parse_args(args);

if (result.help_requested) {
    print(result.help_text);
} else if (result.version_requested) {
    print(result.version_text);
} else if (result.error) {
    print("Error: " + result.error_message);
    print(parser.help());
} else {
    // Use parsed arguments
    if (result.flags["verbose"]) {
        print("Verbose mode enabled");
    }
    print("Output: " + result.options["output"]);
}
```

**ArgParser methods:**

| Method | Description |
|--------|-------------|
| `version(v)` | Set version string |
| `flag(short, long, desc)` | Add a boolean flag |
| `option(short, long, desc, default)` | Add an option with value |
| `positional(name, desc, required)` | Add a positional argument |
| `parse_args(argv)` | Parse arguments and return result |
| `help()` | Generate help text |

### shift(argv, n?): array

Remove first N elements from arguments array.

```hemlock
import { shift } from "@stdlib/args";

// Remove script name
let user_args = shift(args, 1);

// Get subcommand args
let cmd = args[1];
let cmd_args = shift(args, 2);
```

### join_args(argv, separator?): string

Join arguments into a string.

```hemlock
import { join_args } from "@stdlib/args";

// Join remaining args with spaces
let command = join_args(["echo", "hello", "world"]);
print(command);  // echo hello world
```

## Examples

### Simple Flag Checking

```hemlock
import { parse, has_flag } from "@stdlib/args";

let parsed = parse(args);

let verbose = has_flag(parsed, "verbose") || has_flag(parsed, "v");
let quiet = has_flag(parsed, "quiet") || has_flag(parsed, "q");
let force = has_flag(parsed, "force") || has_flag(parsed, "f");

if (verbose && quiet) {
    print("Error: Cannot be both verbose and quiet");
}
```

### File Processing Script

```hemlock
import { parse, has_flag, get_option, get_positionals } from "@stdlib/args";

let parsed = parse(args, { strings: ["output", "o"] });

let output = get_option(parsed, "output");
if (output == null) {
    output = get_option(parsed, "o", "output.txt");
}

let files = get_positionals(parsed);
if (files.length == 0) {
    print("Error: No input files specified");
    print("Usage: script.hml [OPTIONS] <files...>");
} else {
    let i = 0;
    while (i < files.length) {
        if (has_flag(parsed, "verbose")) {
            print("Processing: " + files[i]);
        }
        // Process file...
        i = i + 1;
    }
    print("Output written to: " + output);
}
```

### Subcommand Pattern

```hemlock
import { parse, get_positionals, shift } from "@stdlib/args";

let parsed = parse(args);
let positionals = get_positionals(parsed);

if (positionals.length == 0) {
    print("Usage: tool <command> [args...]");
    print("Commands: build, test, run");
} else {
    let cmd = positionals[0];
    let cmd_args = shift(positionals, 1);

    if (cmd == "build") {
        print("Building with args: " + cmd_args.join(" "));
    } else if (cmd == "test") {
        print("Testing with args: " + cmd_args.join(" "));
    } else if (cmd == "run") {
        print("Running with args: " + cmd_args.join(" "));
    } else {
        print("Unknown command: " + cmd);
    }
}
```

### Full CLI Application

```hemlock
import { ArgParser } from "@stdlib/args";

let parser = ArgParser("mygrep", "Search for patterns in files")
    .version("1.0.0")
    .flag("i", "ignore-case", "Ignore case distinctions")
    .flag("v", "invert-match", "Select non-matching lines")
    .flag("c", "count", "Only print count of matching lines")
    .flag("n", "line-number", "Print line numbers")
    .option("A", "after-context", "Print NUM lines after match", "0")
    .option("B", "before-context", "Print NUM lines before match", "0")
    .positional("pattern", "Search pattern", true)
    .positional("file", "File to search", true);

let result = parser.parse_args(args);

if (result.help_requested) {
    print(result.help_text);
} else if (result.version_requested) {
    print(result.version_text);
} else if (result.error) {
    print("Error: " + result.error_message);
    print("\n" + parser.help());
} else {
    let pattern = result.positionals[0];
    let file = result.positionals[1];

    print("Searching for '" + pattern + "' in " + file);

    if (result.flags["ignore-case"]) {
        print("  (case insensitive)");
    }
    if (result.flags["line-number"]) {
        print("  (showing line numbers)");
    }
}
```

## End of Options Marker

Use `--` to mark the end of options. Everything after is treated as positional:

```hemlock
// ./hemlock script.hml --verbose -- --not-a-flag file.txt
let parsed = parse(args);

print(parsed.flags["verbose"]);     // true
print(parsed.positionals[0]);       // --not-a-flag
print(parsed.positionals[1]);       // file.txt
```

## Error Handling

```hemlock
import { parse, get_option } from "@stdlib/args";

let parsed = parse(args, { strings: ["port"] });
let port_str = get_option(parsed, "port", "8080");

// Validate port number
let port = parse_int(port_str);
if (port < 1 || port > 65535) {
    print("Error: Invalid port number: " + port_str);
}
```

## See Also

- [@stdlib/env](env.md) - Environment variables
- [@stdlib/process](process.md) - Process management
