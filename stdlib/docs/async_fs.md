# Hemlock Async File System Module

Non-blocking file system operations using a thread pool.

## Overview

The async_fs module provides asynchronous file and directory operations that don't block the main thread. All operations return Futures that can be awaited or polled.

This module uses a shared 4-worker thread pool internally for efficient I/O task distribution.

## Usage

```hemlock
import { async_read_file, async_write_file, read_files_parallel } from "@stdlib/async_fs";
```

---

## File Operations

### async_read_file

Read entire file contents asynchronously.

**Signature:**
```hemlock
async_read_file(path: string): Future<string>
```

**Parameters:**
- `path` - Path to file to read

**Returns:** Future that resolves to file contents as string

**Example:**
```hemlock
import { async_read_file } from "@stdlib/async_fs";

let future = async_read_file("data.txt");
// Do other work...
let content = future.get();
print(content);
```

---

### async_write_file

Write content to file asynchronously. Creates file if it doesn't exist, overwrites if it does.

**Signature:**
```hemlock
async_write_file(path: string, content: string): Future<null>
```

**Parameters:**
- `path` - Path to file to write
- `content` - Content to write

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_write_file } from "@stdlib/async_fs";

let future = async_write_file("output.txt", "Hello, World!");
future.get();  // Wait for write to complete
```

---

### async_append_file

Append content to file asynchronously.

**Signature:**
```hemlock
async_append_file(path: string, content: string): Future<null>
```

**Parameters:**
- `path` - Path to file to append to
- `content` - Content to append

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_append_file } from "@stdlib/async_fs";

let future = async_append_file("log.txt", "New log entry\n");
future.get();
```

---

### async_copy_file

Copy file asynchronously.

**Signature:**
```hemlock
async_copy_file(src: string, dst: string): Future<null>
```

**Parameters:**
- `src` - Source file path
- `dst` - Destination file path

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_copy_file } from "@stdlib/async_fs";

let future = async_copy_file("original.txt", "backup.txt");
future.get();
```

---

### async_remove_file

Remove file asynchronously.

**Signature:**
```hemlock
async_remove_file(path: string): Future<null>
```

**Parameters:**
- `path` - Path to file to remove

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_remove_file } from "@stdlib/async_fs";

let future = async_remove_file("temp.txt");
future.get();
```

---

### async_rename

Rename or move file asynchronously.

**Signature:**
```hemlock
async_rename(old_path: string, new_path: string): Future<null>
```

**Parameters:**
- `old_path` - Current path
- `new_path` - New path

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_rename } from "@stdlib/async_fs";

let future = async_rename("old_name.txt", "new_name.txt");
future.get();
```

---

### async_exists

Check if file or directory exists asynchronously.

**Signature:**
```hemlock
async_exists(path: string): Future<bool>
```

**Parameters:**
- `path` - Path to check

**Returns:** Future that resolves to boolean

**Example:**
```hemlock
import { async_exists } from "@stdlib/async_fs";

let future = async_exists("config.json");
if (future.get()) {
    print("Config file exists");
}
```

---

### async_file_stat

Get file statistics asynchronously.

**Signature:**
```hemlock
async_file_stat(path: string): Future<object>
```

**Parameters:**
- `path` - Path to file

**Returns:** Future that resolves to stat object

**Example:**
```hemlock
import { async_file_stat } from "@stdlib/async_fs";

let future = async_file_stat("data.txt");
let stat = future.get();
print("Size: " + stat.size);
```

---

## Directory Operations

### async_list_dir

List directory contents asynchronously.

**Signature:**
```hemlock
async_list_dir(path: string): Future<array<string>>
```

**Parameters:**
- `path` - Directory path

**Returns:** Future that resolves to array of filenames

**Example:**
```hemlock
import { async_list_dir } from "@stdlib/async_fs";

let future = async_list_dir("./src");
let files = future.get();
for (file in files) {
    print(file);
}
```

---

### async_make_dir

Create directory asynchronously.

**Signature:**
```hemlock
async_make_dir(path: string): Future<null>
```

**Parameters:**
- `path` - Directory path to create

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_make_dir } from "@stdlib/async_fs";

let future = async_make_dir("./output");
future.get();
```

---

### async_remove_dir

Remove directory asynchronously.

**Signature:**
```hemlock
async_remove_dir(path: string): Future<null>
```

**Parameters:**
- `path` - Directory path to remove

**Returns:** Future that resolves to null on success

**Example:**
```hemlock
import { async_remove_dir } from "@stdlib/async_fs";

let future = async_remove_dir("./temp");
future.get();
```

---

## Parallel Convenience Functions

### read_files_parallel

Read multiple files in parallel.

**Signature:**
```hemlock
read_files_parallel(paths: array<string>): array<string>
```

**Parameters:**
- `paths` - Array of file paths to read

**Returns:** Array of file contents in same order as input paths

**Example:**
```hemlock
import { read_files_parallel } from "@stdlib/async_fs";

let paths = ["file1.txt", "file2.txt", "file3.txt"];
let contents = read_files_parallel(paths);

let i = 0;
while (i < contents.length) {
    print("File " + i + ": " + contents[i].length + " bytes");
    i = i + 1;
}
```

---

### write_files_parallel

Write multiple files in parallel.

**Signature:**
```hemlock
write_files_parallel(files: array<object>): null
```

**Parameters:**
- `files` - Array of objects with `path` and `content` properties

**Returns:** null (blocks until all writes complete)

**Example:**
```hemlock
import { write_files_parallel } from "@stdlib/async_fs";

let files = [
    { path: "output1.txt", content: "Content 1" },
    { path: "output2.txt", content: "Content 2" },
    { path: "output3.txt", content: "Content 3" }
];

write_files_parallel(files);
print("All files written");
```

---

### copy_files_parallel

Copy multiple files in parallel.

**Signature:**
```hemlock
copy_files_parallel(copies: array<object>): null
```

**Parameters:**
- `copies` - Array of objects with `src` and `dst` properties

**Returns:** null (blocks until all copies complete)

**Example:**
```hemlock
import { copy_files_parallel } from "@stdlib/async_fs";

let copies = [
    { src: "file1.txt", dst: "backup/file1.txt" },
    { src: "file2.txt", dst: "backup/file2.txt" },
    { src: "file3.txt", dst: "backup/file3.txt" }
];

copy_files_parallel(copies);
print("All files copied");
```

---

### shutdown_async_fs

Shutdown the internal thread pool. Call when done with async file I/O to release resources.

**Signature:**
```hemlock
shutdown_async_fs(): null
```

**Example:**
```hemlock
import { async_read_file, shutdown_async_fs } from "@stdlib/async_fs";

// Use async file operations...
let content = async_read_file("data.txt").get();

// Cleanup when done
shutdown_async_fs();
```

**Note:** The thread pool is lazily initialized on first use. If you never use async_fs functions, no pool is created.

---

## Complete Example

```hemlock
import {
    async_read_file,
    async_write_file,
    read_files_parallel,
    shutdown_async_fs
} from "@stdlib/async_fs";

// Process multiple config files in parallel
let config_files = ["app.json", "db.json", "cache.json"];
let configs = read_files_parallel(config_files);

print("Loaded " + configs.length + " config files");

// Process configs and write results
let i = 0;
let futures = [];
while (i < configs.length) {
    let processed = "Processed: " + configs[i];
    futures.push(async_write_file("output_" + i + ".txt", processed));
    i = i + 1;
}

// Wait for all writes
i = 0;
while (i < futures.length) {
    futures[i].get();
    i = i + 1;
}

print("All processing complete");

// Cleanup
shutdown_async_fs();
```

---

## Error Handling

Errors in async operations are captured and re-thrown when `get()` is called:

```hemlock
import { async_read_file } from "@stdlib/async_fs";

let future = async_read_file("nonexistent.txt");

try {
    let content = future.get();
} catch (e) {
    print("Failed to read file: " + e);
}
```

---

## See Also

- [async](async.md) - ThreadPool and parallel_map utilities
- [fs](fs.md) - Synchronous file system operations
