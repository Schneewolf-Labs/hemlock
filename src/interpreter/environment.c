#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== ENVIRONMENT ==========

Environment* env_new(Environment *parent) {
    Environment *env = malloc(sizeof(Environment));
    env->capacity = 16;
    env->count = 0;
    env->names = malloc(sizeof(char*) * env->capacity);
    env->values = malloc(sizeof(Value) * env->capacity);
    env->is_const = malloc(sizeof(int) * env->capacity);
    env->parent = parent;
    return env;
}

void env_free(Environment *env) {
    for (int i = 0; i < env->count; i++) {
        free(env->names[i]);
    }
    free(env->names);
    free(env->values);
    free(env->is_const);
    free(env);
}

static void env_grow(Environment *env) {
    env->capacity *= 2;
    env->names = realloc(env->names, sizeof(char*) * env->capacity);
    env->values = realloc(env->values, sizeof(Value) * env->capacity);
    env->is_const = realloc(env->is_const, sizeof(int) * env->capacity);
}

// Define a new variable (for let/const declarations)
void env_define(Environment *env, const char *name, Value value, int is_const) {
    // Check if variable already exists in current scope
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            fprintf(stderr, "Runtime error: Variable '%s' already defined in this scope\n", name);
            exit(1);
        }
    }

    // New variable
    if (env->count >= env->capacity) {
        env_grow(env);
    }

    env->names[env->count] = strdup(name);
    env->values[env->count] = value;
    env->is_const[env->count] = is_const;
    env->count++;
}

// Set a variable (for reassignment or implicit definition in loops/functions)
void env_set(Environment *env, const char *name, Value value) {
    // Check current scope
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            // Check if variable is const
            if (env->is_const[i]) {
                fprintf(stderr, "Runtime error: Cannot assign to const variable '%s'\n", name);
                exit(1);
            }
            env->values[i] = value;
            return;
        }
    }

    // Check parent scope
    if (env->parent != NULL) {
        // Look for variable in parent scopes
        Environment *search_env = env->parent;
        while (search_env != NULL) {
            for (int i = 0; i < search_env->count; i++) {
                if (strcmp(search_env->names[i], name) == 0) {
                    // Found in parent scope - check if const
                    if (search_env->is_const[i]) {
                        fprintf(stderr, "Runtime error: Cannot assign to const variable '%s'\n", name);
                        exit(1);
                    }
                    // Update parent scope variable
                    search_env->values[i] = value;
                    return;
                }
            }
            search_env = search_env->parent;
        }
    }

    // Variable not found anywhere - create new mutable variable in current scope
    // This handles implicit variable creation in loops and function calls
    if (env->count >= env->capacity) {
        env_grow(env);
    }

    env->names[env->count] = strdup(name);
    env->values[env->count] = value;
    env->is_const[env->count] = 0;  // Always mutable for implicit variables
    env->count++;
}

Value env_get(Environment *env, const char *name) {
    // Search current scope
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            return env->values[i];
        }
    }

    // Search parent scope
    if (env->parent != NULL) {
        return env_get(env->parent, name);
    }

    // Variable not found
    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
    exit(1);
}
