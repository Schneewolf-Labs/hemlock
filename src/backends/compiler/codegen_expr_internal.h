/*
 * Hemlock Code Generator - Expression Internal Declarations
 *
 * Internal declarations for modular expression code generation.
 * These functions are called from the main codegen_expr() dispatcher.
 */

#ifndef HEMLOCK_CODEGEN_EXPR_INTERNAL_H
#define HEMLOCK_CODEGEN_EXPR_INTERNAL_H

#include "codegen_internal.h"

// ========== EXPRESSION HANDLER FUNCTIONS ==========

// Handle EXPR_IDENT - variable references and builtin constants/functions
// Returns the temp variable name containing the result
char* codegen_expr_ident(CodegenContext *ctx, Expr *expr, char *result);

// Handle EXPR_CALL - function calls, method calls, and builtin invocations
// Returns the temp variable name containing the result
char* codegen_expr_call(CodegenContext *ctx, Expr *expr, char *result);

#endif // HEMLOCK_CODEGEN_EXPR_INTERNAL_H
