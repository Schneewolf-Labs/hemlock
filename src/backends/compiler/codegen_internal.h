/*
 * Hemlock Code Generator - Internal Declarations
 *
 * Shared internal declarations for the modular codegen implementation.
 * This header is only included by codegen implementation files.
 */

#ifndef HEMLOCK_CODEGEN_INTERNAL_H
#define HEMLOCK_CODEGEN_INTERNAL_H

#define _GNU_SOURCE
#include "codegen.h"
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

// ========== IN-MEMORY BUFFER SUPPORT ==========

// In-memory buffer for code generation (replaces tmpfile for better performance)
typedef struct {
    char *data;      // Buffer data (dynamically allocated by open_memstream)
    size_t size;     // Current size of data
    FILE *stream;    // Stream for writing (NULL after close)
} MemBuffer;

// Create a new in-memory buffer
MemBuffer* membuf_new(void);

// Flush buffer contents to a FILE* (can be called multiple times)
void membuf_flush_to(MemBuffer *buf, FILE *output);

// Close and free the buffer
void membuf_free(MemBuffer *buf);

// ========== INTERNAL HELPER FUNCTIONS ==========

// Remove a local variable from scope (used for catch params that go out of scope)
void codegen_remove_local(CodegenContext *ctx, const char *name);

// Shadow variable tracking (locals that shadow main vars, like catch params)
void codegen_add_shadow(CodegenContext *ctx, const char *name);
int codegen_is_shadow(CodegenContext *ctx, const char *name);
void codegen_remove_shadow(CodegenContext *ctx, const char *name);

// Const variable tracking (for preventing reassignment)
void codegen_add_const(CodegenContext *ctx, const char *name);
int codegen_is_const(CodegenContext *ctx, const char *name);

// Try-finally context tracking
void codegen_push_try_finally(CodegenContext *ctx, const char *finally_label,
                              const char *return_value_var, const char *has_return_var);
void codegen_pop_try_finally(CodegenContext *ctx);
const char* codegen_get_finally_label(CodegenContext *ctx);
const char* codegen_get_return_value_var(CodegenContext *ctx);
const char* codegen_get_has_return_var(CodegenContext *ctx);

// Switch context tracking (for break -> goto)
void codegen_push_switch(CodegenContext *ctx, const char *end_label);
void codegen_pop_switch(CodegenContext *ctx);
const char* codegen_get_switch_end_label(CodegenContext *ctx);

// For-loop continue tracking (continue -> goto increment label)
void codegen_push_for_continue(CodegenContext *ctx, const char *continue_label);
void codegen_pop_for_continue(CodegenContext *ctx);
const char* codegen_get_for_continue_label(CodegenContext *ctx);

// Main file variable tracking
void codegen_add_main_var(CodegenContext *ctx, const char *name);
int codegen_is_main_var(CodegenContext *ctx, const char *name);

// Main file function definitions
void codegen_add_main_func(CodegenContext *ctx, const char *name, int num_params, int has_rest);
int codegen_is_main_func(CodegenContext *ctx, const char *name);
int codegen_get_main_func_params(CodegenContext *ctx, const char *name);
int codegen_get_main_func_has_rest(CodegenContext *ctx, const char *name);

// Main file import tracking
void codegen_add_main_import(CodegenContext *ctx, const char *local_name,
                             const char *original_name, const char *module_prefix,
                             int is_function, int num_params);
ImportBinding* codegen_find_main_import(CodegenContext *ctx, const char *name);

// ========== SHARED ENVIRONMENT SUPPORT ==========

// Add a variable to the shared environment (returns index)
int shared_env_add_var(CodegenContext *ctx, const char *var);

// Get index of a variable in shared environment (-1 if not found)
int shared_env_get_index(CodegenContext *ctx, const char *var);

// Clear shared environment state
void shared_env_clear(CodegenContext *ctx);

// Count required parameters (those without defaults)
int count_required_params(Expr **param_defaults, int num_params);

// ========== CLOSURE SCANNING ==========

// Scan expression for closures and collect captured variables
void scan_closures_expr(CodegenContext *ctx, Expr *expr, Scope *local_scope);

// Scan statement for closures
void scan_closures_stmt(CodegenContext *ctx, Stmt *stmt, Scope *local_scope);

// ========== MODULE FUNCTIONS ==========

// Find an import binding in a module
ImportBinding* module_find_import(CompiledModule *module, const char *name);

// Add an import to a module
void module_add_import(CompiledModule *module, const char *local_name,
                       const char *original_name, const char *module_prefix,
                       int is_function, int num_params);

// Check if a name is an extern function in the given module
int module_is_extern_fn(CompiledModule *module, const char *name);

// ========== CLOSURE GENERATION ==========

// Generate a closure function implementation
void codegen_closure_impl(CodegenContext *ctx, ClosureInfo *closure);

// Generate wrapper function for closure
void codegen_closure_wrapper(CodegenContext *ctx, ClosureInfo *closure);

// ========== FUNCTION GENERATION ==========

// Generate a top-level function declaration
void codegen_function_decl(CodegenContext *ctx, Expr *func, const char *name);

// Check if a statement is a function definition
int is_function_def(Stmt *stmt, char **name_out, Expr **func_out);

// Generate module init function
void codegen_module_init(CodegenContext *ctx, CompiledModule *module);

// Generate function declarations for a module
void codegen_module_funcs(CodegenContext *ctx, CompiledModule *module,
                          MemBuffer *decl_buffer, MemBuffer *impl_buffer);

// ========== MODULE COMPILATION ==========

// Parse a module file
Stmt** parse_module_file(const char *path, int *stmt_count);

// ========== TYPE MAPPING HELPERS ==========

// Convert TypeKind to HML_VAL_* string (e.g., TYPE_I8 -> "HML_VAL_I8")
// Returns NULL for types that don't have a direct HML_VAL mapping
const char* type_kind_to_hml_val(TypeKind kind);

// Convert TypeKind to HML_FFI_* string (e.g., TYPE_I8 -> "HML_FFI_I8")
// Returns "HML_FFI_VOID" for unknown types
const char* type_kind_to_ffi_type(TypeKind kind);

#endif // HEMLOCK_CODEGEN_INTERNAL_H
