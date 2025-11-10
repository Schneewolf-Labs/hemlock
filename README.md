![Hemlock](logo.png)

# Hemlock

> A small, unsafe language for writing unsafe things safely.

Hemlock is a systems scripting language that combines the power of C with the ergonomics of modern scripting languages. It embraces manual memory management and explicit control while providing structured async concurrency built-in.

## Design Philosophy

- **Explicit over implicit** - No hidden behavior, no magic
- **Manual memory management** - You allocate, you free
- **Dynamic by default, typed by choice** - Optional type annotations with runtime checking
- **Unsafe is a feature** - Full control when you need it, safety tools when you want them

## Features

- **Familiar syntax** - C-like with modern improvements
- **Rich type system** - i8/i16/i32, u8/u16/u32, f32/f64, bool, string, ptr, buffer
- **Two pointer types** - Raw `ptr` for experts, safe `buffer` with bounds checking
- **Memory API** - alloc, free, memset, memcpy, realloc, talloc, sizeof
- **Mutable strings** - First-class UTF-8 strings with indexing and concatenation
- **Structured concurrency** - async/await, spawn/join/detach (coming soon)

## Quick Examples

### Hello World
```hemlock
print("Hello, World!");
```

### Memory Management
```hemlock
// Raw pointer (dangerous but flexible)
let p: ptr = alloc(64);
memset(p, 0, 64);
free(p);

// Safe buffer (bounds checked)
let buf: buffer = buffer(64);
buf[0] = 65;
print(buf.length);  // 64
free(buf);
```

### Type System
```hemlock
let x = 42;              // i32 inferred
let y: u8 = 255;         // explicit u8
let z = x + y;           // promotes to i32
let pi: f64 = 3.14159;   // explicit precision
```

### String Operations
```hemlock
let s = "hello";
s[0] = 72;              // mutate to "Hello"
print(s.length);        // 5
let msg = s + " world"; // "Hello world"
```

## Building

```bash
make
```

## Running Tests

```bash
make test
```

## Running Programs

```bash
./hemlock program.hml
```

## Project Status

Hemlock is currently in early development (v0.1). The following features are implemented:

- ✅ Primitives and type system
- ✅ Memory management (ptr, buffer, alloc, free)
- ✅ String operations
- ✅ Control flow (if/else, while)
- 🚧 Functions and closures
- 🚧 Structs and methods
- 🚧 Async/await and structured concurrency

## Why Hemlock?

Hemlock is for programmers who want:
- The power of C without the boilerplate
- Manual memory management without the verbosity
- Dynamic types with optional static guarantees
- Structured concurrency without the complexity

Hemlock is **NOT** memory-safe. Dangling pointers, use-after-free, and buffer overflows are your responsibility. We provide tools to help (`buffer`, type annotations, bounds checking) but we don't force you to use them.

## Philosophy on Safety

> "We give you the tools to be safe (`buffer`, type annotations, bounds checking) but we don't force you to use them (`ptr`, manual memory, unsafe operations). The default should guide toward safety, but the escape hatch should always be available."

## License

MIT License

## Contributing

Hemlock is experimental and evolving. If you're interested in contributing, please read `CLAUDE.md` first to understand the design philosophy.
