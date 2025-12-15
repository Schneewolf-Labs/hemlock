/*
 * Type Inference for Hemlock Compiler
 *
 * Performs static type analysis to enable optimized code generation.
 * When types are known at compile time, we can emit direct C operations
 * instead of runtime type dispatch.
 */

#ifndef HEMLOCK_TYPE_INFER_H
#define HEMLOCK_TYPE_INFER_H

#include "../../include/ast.h"

// Inferred type kinds
typedef enum {
    INFER_UNKNOWN,      // Type not yet inferred / can be anything
    INFER_I32,          // Known to be i32
    INFER_I64,          // Known to be i64
    INFER_F64,          // Known to be f64
    INFER_BOOL,         // Known to be bool
    INFER_STRING,       // Known to be string
    INFER_NULL,         // Known to be null
    INFER_ARRAY,        // Known to be array (element type may be known)
    INFER_OBJECT,       // Known to be object
    INFER_FUNCTION,     // Known to be function
    INFER_NUMERIC,      // Known to be numeric (i32, i64, or f64) but not which
    INFER_INTEGER,      // Known to be integer (i32 or i64) but not which
} InferredTypeKind;

// Inferred type with optional element type for arrays
typedef struct InferredType {
    InferredTypeKind kind;
    struct InferredType *element_type;  // For arrays
} InferredType;

// Variable type binding
typedef struct TypeBinding {
    char *name;
    InferredType type;
    struct TypeBinding *next;
} TypeBinding;

// Type environment (scoped type bindings)
typedef struct TypeEnv {
    TypeBinding *bindings;
    struct TypeEnv *parent;
} TypeEnv;

// Function return type tracking
typedef struct FuncReturnType {
    char *name;              // Function name
    InferredType return_type;
    struct FuncReturnType *next;
} FuncReturnType;

// Type inference context
typedef struct {
    TypeEnv *current_env;
    FuncReturnType *func_returns;  // Registry of function return types
    int changed;  // Set to 1 if any type was refined this pass
} TypeInferContext;

// ========== TYPE CONSTRUCTORS ==========

InferredType infer_unknown(void);
InferredType infer_i32(void);
InferredType infer_i64(void);
InferredType infer_f64(void);
InferredType infer_bool(void);
InferredType infer_string(void);
InferredType infer_null(void);
InferredType infer_numeric(void);
InferredType infer_integer(void);

// ========== TYPE OPERATIONS ==========

// Check if type is known (not UNKNOWN)
int infer_is_known(InferredType t);

// Check if type is a specific kind
int infer_is_i32(InferredType t);
int infer_is_i64(InferredType t);
int infer_is_f64(InferredType t);
int infer_is_integer(InferredType t);  // i32, i64, or INTEGER
int infer_is_numeric(InferredType t);  // any numeric type

// Meet operation: find common type (for control flow merge)
InferredType infer_meet(InferredType a, InferredType b);

// Result type of binary operation
InferredType infer_binary_result(BinaryOp op, InferredType left, InferredType right);

// Result type of unary operation
InferredType infer_unary_result(UnaryOp op, InferredType operand);

// ========== ENVIRONMENT OPERATIONS ==========

TypeInferContext* type_infer_new(void);
void type_infer_free(TypeInferContext *ctx);

void type_env_push(TypeInferContext *ctx);
void type_env_pop(TypeInferContext *ctx);

void type_env_bind(TypeInferContext *ctx, const char *name, InferredType type);
InferredType type_env_lookup(TypeInferContext *ctx, const char *name);

// Refine a variable's type (if new type is more specific)
void type_env_refine(TypeInferContext *ctx, const char *name, InferredType type);

// ========== FUNCTION RETURN TYPE TRACKING ==========

// Register a function's return type
void type_register_func_return(TypeInferContext *ctx, const char *name, InferredType ret_type);

// Look up a function's return type
InferredType type_lookup_func_return(TypeInferContext *ctx, const char *name);

// ========== INFERENCE ==========

// Infer type of an expression given current environment
InferredType infer_expr(TypeInferContext *ctx, Expr *expr);

// Analyze a statement (updates environment with bindings)
void infer_stmt(TypeInferContext *ctx, Stmt *stmt);

// Analyze a function and infer parameter/return types
void infer_function(TypeInferContext *ctx, Expr *func_expr);

// ========== DEBUG ==========

const char* infer_type_name(InferredType t);

#endif // HEMLOCK_TYPE_INFER_H
