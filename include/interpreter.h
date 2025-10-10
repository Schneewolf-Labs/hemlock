#ifndef HEMLOCK_INTERPRETER_H
#define HEMLOCK_INTERPRETER_H

#include "ast.h"

// Value types that can exist at runtime
typedef enum {
    VAL_INT,
    VAL_BOOL,
    VAL_NULL,
} ValueType;

// Runtime value
typedef struct {
    ValueType type;
    union {
        int as_int;
        int as_bool;
    } as;
} Value;

// Environment (symbol table for variables)
typedef struct Environment {
    char **names;
    Value *values;
    int count;
    int capacity;
    struct Environment *parent;  // for nested scopes later
} Environment;

// Public interface
Environment* env_new(Environment *parent);
void env_free(Environment *env);
void env_set(Environment *env, const char *name, Value value);
Value env_get(Environment *env, const char *name);

Value eval_expr(Expr *expr, Environment *env);
void eval_stmt(Stmt *stmt, Environment *env);
void eval_program(Stmt **stmts, int count, Environment *env);

// Value constructors
Value val_int(int value);
Value val_bool(int value);
Value val_null(void);

// Value operations
void print_value(Value val);

#endif // HEMLOCK_INTERPRETER_H