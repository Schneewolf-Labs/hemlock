/*
 * Type Inference Implementation
 */

#include "type_infer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== TYPE CONSTRUCTORS ==========

InferredType infer_unknown(void) {
    return (InferredType){ .kind = INFER_UNKNOWN, .element_type = NULL };
}

InferredType infer_i32(void) {
    return (InferredType){ .kind = INFER_I32, .element_type = NULL };
}

InferredType infer_i64(void) {
    return (InferredType){ .kind = INFER_I64, .element_type = NULL };
}

InferredType infer_f64(void) {
    return (InferredType){ .kind = INFER_F64, .element_type = NULL };
}

InferredType infer_bool(void) {
    return (InferredType){ .kind = INFER_BOOL, .element_type = NULL };
}

InferredType infer_string(void) {
    return (InferredType){ .kind = INFER_STRING, .element_type = NULL };
}

InferredType infer_null(void) {
    return (InferredType){ .kind = INFER_NULL, .element_type = NULL };
}

InferredType infer_numeric(void) {
    return (InferredType){ .kind = INFER_NUMERIC, .element_type = NULL };
}

InferredType infer_integer(void) {
    return (InferredType){ .kind = INFER_INTEGER, .element_type = NULL };
}

// ========== TYPE OPERATIONS ==========

int infer_is_known(InferredType t) {
    return t.kind != INFER_UNKNOWN;
}

int infer_is_i32(InferredType t) {
    return t.kind == INFER_I32;
}

int infer_is_i64(InferredType t) {
    return t.kind == INFER_I64;
}

int infer_is_f64(InferredType t) {
    return t.kind == INFER_F64;
}

int infer_is_integer(InferredType t) {
    return t.kind == INFER_I32 || t.kind == INFER_I64 || t.kind == INFER_INTEGER;
}

int infer_is_numeric(InferredType t) {
    return t.kind == INFER_I32 || t.kind == INFER_I64 || t.kind == INFER_F64 ||
           t.kind == INFER_NUMERIC || t.kind == INFER_INTEGER;
}

// Meet: find common type (for merging control flow paths)
InferredType infer_meet(InferredType a, InferredType b) {
    // Same type -> keep it
    if (a.kind == b.kind) return a;

    // Unknown dominates
    if (a.kind == INFER_UNKNOWN) return a;
    if (b.kind == INFER_UNKNOWN) return b;

    // Both integers -> INTEGER
    if (infer_is_integer(a) && infer_is_integer(b)) {
        if (a.kind == INFER_I32 && b.kind == INFER_I32) return infer_i32();
        if (a.kind == INFER_I64 && b.kind == INFER_I64) return infer_i64();
        return infer_integer();
    }

    // Both numeric -> NUMERIC
    if (infer_is_numeric(a) && infer_is_numeric(b)) {
        return infer_numeric();
    }

    // Otherwise unknown
    return infer_unknown();
}

// Result type of binary operation
InferredType infer_binary_result(BinaryOp op, InferredType left, InferredType right) {
    switch (op) {
        // Arithmetic: result type depends on operands
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
            if (infer_is_f64(left) || infer_is_f64(right)) return infer_f64();
            if (infer_is_i64(left) || infer_is_i64(right)) return infer_i64();
            if (infer_is_i32(left) && infer_is_i32(right)) return infer_i32();
            if (infer_is_integer(left) && infer_is_integer(right)) return infer_integer();
            if (infer_is_numeric(left) && infer_is_numeric(right)) return infer_numeric();
            // String + anything = string
            if (op == OP_ADD && (left.kind == INFER_STRING || right.kind == INFER_STRING)) {
                return infer_string();
            }
            return infer_unknown();

        case OP_DIV:
            // Division always returns f64 in Hemlock
            return infer_f64();

        case OP_MOD:
            // Modulo: like other arithmetic but stays integer
            if (infer_is_i64(left) || infer_is_i64(right)) return infer_i64();
            if (infer_is_i32(left) && infer_is_i32(right)) return infer_i32();
            if (infer_is_integer(left) && infer_is_integer(right)) return infer_integer();
            return infer_numeric();

        // Comparison: always bool
        case OP_EQUAL:
        case OP_NOT_EQUAL:
        case OP_LESS:
        case OP_LESS_EQUAL:
        case OP_GREATER:
        case OP_GREATER_EQUAL:
            return infer_bool();

        // Logical: always bool
        case OP_AND:
        case OP_OR:
            return infer_bool();

        // Bitwise: result is integer type
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_BIT_LSHIFT:
        case OP_BIT_RSHIFT:
            if (infer_is_i64(left) || infer_is_i64(right)) return infer_i64();
            if (infer_is_i32(left) && infer_is_i32(right)) return infer_i32();
            return infer_integer();

        default:
            return infer_unknown();
    }
}

// Result type of unary operation
InferredType infer_unary_result(UnaryOp op, InferredType operand) {
    switch (op) {
        case UNARY_NEGATE:
            // Negation preserves numeric type
            return operand;

        case UNARY_NOT:
            // Logical not always produces bool
            return infer_bool();

        case UNARY_BIT_NOT:
            // Bitwise not preserves integer type
            return operand;

        default:
            return infer_unknown();
    }
}

// ========== ENVIRONMENT OPERATIONS ==========

TypeInferContext* type_infer_new(void) {
    TypeInferContext *ctx = malloc(sizeof(TypeInferContext));
    ctx->current_env = malloc(sizeof(TypeEnv));
    ctx->current_env->bindings = NULL;
    ctx->current_env->parent = NULL;
    ctx->func_returns = NULL;
    ctx->changed = 0;
    return ctx;
}

static void free_bindings(TypeBinding *b) {
    while (b) {
        TypeBinding *next = b->next;
        free(b->name);
        if (b->type.element_type) free(b->type.element_type);
        free(b);
        b = next;
    }
}

void type_infer_free(TypeInferContext *ctx) {
    while (ctx->current_env) {
        TypeEnv *parent = ctx->current_env->parent;
        free_bindings(ctx->current_env->bindings);
        free(ctx->current_env);
        ctx->current_env = parent;
    }
    // Free function return type registry
    FuncReturnType *f = ctx->func_returns;
    while (f) {
        FuncReturnType *next = f->next;
        free(f->name);
        free(f);
        f = next;
    }
    free(ctx);
}

void type_env_push(TypeInferContext *ctx) {
    TypeEnv *env = malloc(sizeof(TypeEnv));
    env->bindings = NULL;
    env->parent = ctx->current_env;
    ctx->current_env = env;
}

void type_env_pop(TypeInferContext *ctx) {
    TypeEnv *old = ctx->current_env;
    ctx->current_env = old->parent;
    free_bindings(old->bindings);
    free(old);
}

void type_env_bind(TypeInferContext *ctx, const char *name, InferredType type) {
    TypeBinding *b = malloc(sizeof(TypeBinding));
    b->name = strdup(name);
    b->type = type;
    b->next = ctx->current_env->bindings;
    ctx->current_env->bindings = b;
}

InferredType type_env_lookup(TypeInferContext *ctx, const char *name) {
    for (TypeEnv *env = ctx->current_env; env; env = env->parent) {
        for (TypeBinding *b = env->bindings; b; b = b->next) {
            if (strcmp(b->name, name) == 0) {
                return b->type;
            }
        }
    }
    return infer_unknown();
}

void type_env_refine(TypeInferContext *ctx, const char *name, InferredType type) {
    // Find and update existing binding if new type is more specific
    for (TypeEnv *env = ctx->current_env; env; env = env->parent) {
        for (TypeBinding *b = env->bindings; b; b = b->next) {
            if (strcmp(b->name, name) == 0) {
                // Only refine if going from less specific to more specific
                if (b->type.kind == INFER_UNKNOWN && type.kind != INFER_UNKNOWN) {
                    b->type = type;
                    ctx->changed = 1;
                } else if (b->type.kind == INFER_NUMERIC && infer_is_integer(type)) {
                    b->type = type;
                    ctx->changed = 1;
                } else if (b->type.kind == INFER_INTEGER &&
                           (type.kind == INFER_I32 || type.kind == INFER_I64)) {
                    b->type = type;
                    ctx->changed = 1;
                }
                return;
            }
        }
    }
}

// ========== FUNCTION RETURN TYPE TRACKING ==========

void type_register_func_return(TypeInferContext *ctx, const char *name, InferredType ret_type) {
    // Check if already registered
    for (FuncReturnType *f = ctx->func_returns; f; f = f->next) {
        if (strcmp(f->name, name) == 0) {
            f->return_type = ret_type;  // Update existing
            return;
        }
    }
    // Add new entry
    FuncReturnType *f = malloc(sizeof(FuncReturnType));
    f->name = strdup(name);
    f->return_type = ret_type;
    f->next = ctx->func_returns;
    ctx->func_returns = f;
}

InferredType type_lookup_func_return(TypeInferContext *ctx, const char *name) {
    for (FuncReturnType *f = ctx->func_returns; f; f = f->next) {
        if (strcmp(f->name, name) == 0) {
            return f->return_type;
        }
    }
    return infer_unknown();
}

// ========== INFERENCE ==========

InferredType infer_expr(TypeInferContext *ctx, Expr *expr) {
    if (!expr) return infer_unknown();

    switch (expr->type) {
        case EXPR_NUMBER:
            // Check if it's a float
            if (expr->as.number.is_float) {
                return infer_f64();
            }
            // Check if value fits in i32
            if (expr->as.number.int_value >= -2147483648LL &&
                expr->as.number.int_value <= 2147483647LL) {
                return infer_i32();
            }
            return infer_i64();

        case EXPR_BOOL:
            return infer_bool();

        case EXPR_STRING:
        case EXPR_STRING_INTERPOLATION:
            return infer_string();

        case EXPR_NULL:
            return infer_null();

        case EXPR_IDENT:
            return type_env_lookup(ctx, expr->as.ident.name);

        case EXPR_BINARY: {
            InferredType left = infer_expr(ctx, expr->as.binary.left);
            InferredType right = infer_expr(ctx, expr->as.binary.right);
            return infer_binary_result(expr->as.binary.op, left, right);
        }

        case EXPR_UNARY: {
            InferredType operand = infer_expr(ctx, expr->as.unary.operand);
            return infer_unary_result(expr->as.unary.op, operand);
        }

        case EXPR_ASSIGN: {
            InferredType value = infer_expr(ctx, expr->as.assign.value);
            // Refine the variable's type
            type_env_refine(ctx, expr->as.assign.name, value);
            return value;
        }

        case EXPR_TERNARY: {
            InferredType then_t = infer_expr(ctx, expr->as.ternary.true_expr);
            InferredType else_t = infer_expr(ctx, expr->as.ternary.false_expr);
            return infer_meet(then_t, else_t);
        }

        case EXPR_CALL:
            // Look up function return type if it's a direct call to a known function
            if (expr->as.call.func && expr->as.call.func->type == EXPR_IDENT) {
                return type_lookup_func_return(ctx, expr->as.call.func->as.ident.name);
            }
            return infer_unknown();

        case EXPR_ARRAY_LITERAL:
            return (InferredType){ .kind = INFER_ARRAY, .element_type = NULL };

        case EXPR_OBJECT_LITERAL:
            return (InferredType){ .kind = INFER_OBJECT, .element_type = NULL };

        case EXPR_FUNCTION:
            return (InferredType){ .kind = INFER_FUNCTION, .element_type = NULL };

        case EXPR_INDEX:
            // Array indexing - would need element type tracking
            return infer_unknown();

        case EXPR_GET_PROPERTY:
            // Object property access - would need object type tracking
            return infer_unknown();

        case EXPR_PREFIX_INC:
        case EXPR_PREFIX_DEC:
            return infer_expr(ctx, expr->as.prefix_inc.operand);

        case EXPR_POSTFIX_INC:
        case EXPR_POSTFIX_DEC:
            return infer_expr(ctx, expr->as.postfix_inc.operand);

        case EXPR_RUNE:
            // Rune is an integer type (unicode codepoint)
            return infer_i32();

        case EXPR_AWAIT:
            // Await returns unknown (depends on task return type)
            return infer_unknown();

        case EXPR_NULL_COALESCE: {
            InferredType left = infer_expr(ctx, expr->as.null_coalesce.left);
            InferredType right = infer_expr(ctx, expr->as.null_coalesce.right);
            // If left is null, return right's type, otherwise unknown
            if (left.kind == INFER_NULL) return right;
            return infer_meet(left, right);
        }

        default:
            return infer_unknown();
    }
}

void infer_stmt(TypeInferContext *ctx, Stmt *stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_LET: {
            InferredType init_type = infer_unknown();
            if (stmt->as.let.value) {
                init_type = infer_expr(ctx, stmt->as.let.value);
            }
            // If there's a type annotation, use it
            if (stmt->as.let.type_annotation) {
                switch (stmt->as.let.type_annotation->kind) {
                    case TYPE_I32: init_type = infer_i32(); break;
                    case TYPE_I64: init_type = infer_i64(); break;
                    case TYPE_F32:
                    case TYPE_F64: init_type = infer_f64(); break;
                    case TYPE_BOOL: init_type = infer_bool(); break;
                    case TYPE_STRING: init_type = infer_string(); break;
                    default: break;
                }
            }
            type_env_bind(ctx, stmt->as.let.name, init_type);
            break;
        }

        case STMT_CONST: {
            InferredType init_type = infer_unknown();
            if (stmt->as.const_stmt.value) {
                init_type = infer_expr(ctx, stmt->as.const_stmt.value);
            }
            if (stmt->as.const_stmt.type_annotation) {
                switch (stmt->as.const_stmt.type_annotation->kind) {
                    case TYPE_I32: init_type = infer_i32(); break;
                    case TYPE_I64: init_type = infer_i64(); break;
                    case TYPE_F32:
                    case TYPE_F64: init_type = infer_f64(); break;
                    case TYPE_BOOL: init_type = infer_bool(); break;
                    case TYPE_STRING: init_type = infer_string(); break;
                    default: break;
                }
            }
            type_env_bind(ctx, stmt->as.const_stmt.name, init_type);
            break;
        }

        case STMT_BLOCK: {
            type_env_push(ctx);
            for (int i = 0; i < stmt->as.block.count; i++) {
                infer_stmt(ctx, stmt->as.block.statements[i]);
            }
            type_env_pop(ctx);
            break;
        }

        case STMT_IF: {
            infer_expr(ctx, stmt->as.if_stmt.condition);
            infer_stmt(ctx, stmt->as.if_stmt.then_branch);
            if (stmt->as.if_stmt.else_branch) {
                infer_stmt(ctx, stmt->as.if_stmt.else_branch);
            }
            break;
        }

        case STMT_WHILE: {
            infer_expr(ctx, stmt->as.while_stmt.condition);
            infer_stmt(ctx, stmt->as.while_stmt.body);
            break;
        }

        case STMT_FOR: {
            type_env_push(ctx);
            if (stmt->as.for_loop.initializer) {
                infer_stmt(ctx, stmt->as.for_loop.initializer);
            }
            if (stmt->as.for_loop.condition) {
                infer_expr(ctx, stmt->as.for_loop.condition);
            }
            if (stmt->as.for_loop.increment) {
                infer_expr(ctx, stmt->as.for_loop.increment);
            }
            infer_stmt(ctx, stmt->as.for_loop.body);
            type_env_pop(ctx);
            break;
        }

        case STMT_EXPR:
            infer_expr(ctx, stmt->as.expr);
            break;

        case STMT_RETURN:
            if (stmt->as.return_stmt.value) {
                infer_expr(ctx, stmt->as.return_stmt.value);
            }
            break;

        default:
            break;
    }
}

void infer_function(TypeInferContext *ctx, Expr *func_expr) {
    if (!func_expr || func_expr->type != EXPR_FUNCTION) return;

    // Analyze function body in new scope
    type_env_push(ctx);
    for (int i = 0; i < func_expr->as.function.num_params; i++) {
        InferredType param_type = infer_unknown();
        if (func_expr->as.function.param_types && func_expr->as.function.param_types[i]) {
            switch (func_expr->as.function.param_types[i]->kind) {
                case TYPE_I32: param_type = infer_i32(); break;
                case TYPE_I64: param_type = infer_i64(); break;
                case TYPE_F32:
                case TYPE_F64: param_type = infer_f64(); break;
                case TYPE_BOOL: param_type = infer_bool(); break;
                case TYPE_STRING: param_type = infer_string(); break;
                default: break;
            }
        }
        type_env_bind(ctx, func_expr->as.function.param_names[i], param_type);
    }
    infer_stmt(ctx, func_expr->as.function.body);
    type_env_pop(ctx);
}

// ========== DEBUG ==========

const char* infer_type_name(InferredType t) {
    switch (t.kind) {
        case INFER_UNKNOWN: return "unknown";
        case INFER_I32: return "i32";
        case INFER_I64: return "i64";
        case INFER_F64: return "f64";
        case INFER_BOOL: return "bool";
        case INFER_STRING: return "string";
        case INFER_NULL: return "null";
        case INFER_ARRAY: return "array";
        case INFER_OBJECT: return "object";
        case INFER_FUNCTION: return "function";
        case INFER_NUMERIC: return "numeric";
        case INFER_INTEGER: return "integer";
        default: return "?";
    }
}
