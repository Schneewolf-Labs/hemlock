#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== STRING OPERATIONS ==========

void string_free(String *str) {
    if (str) {
        free(str->data);
        free(str);
    }
}

String* string_new(const char *cstr) {
    int len = strlen(cstr);
    String *str = malloc(sizeof(String));
    str->length = len;
    str->capacity = len + 1;
    str->data = malloc(str->capacity);
    memcpy(str->data, cstr, len);
    str->data[len] = '\0';
    return str;
}

String* string_copy(String *str) {
    String *copy = malloc(sizeof(String));
    copy->length = str->length;
    copy->capacity = str->capacity;
    copy->data = malloc(copy->capacity);
    memcpy(copy->data, str->data, str->length + 1);
    return copy;
}

String* string_concat(String *a, String *b) {
    int new_len = a->length + b->length;
    String *result = malloc(sizeof(String));
    result->length = new_len;
    result->capacity = new_len + 1;
    result->data = malloc(result->capacity);

    memcpy(result->data, a->data, a->length);
    memcpy(result->data + a->length, b->data, b->length);
    result->data[new_len] = '\0';

    return result;
}

Value val_string(const char *str) {
    Value v;
    v.type = VAL_STRING;
    v.as.as_string = string_new(str);
    return v;
}

Value val_string_take(char *data, int length, int capacity) {
    Value v;
    v.type = VAL_STRING;
    String *str = malloc(sizeof(String));
    str->data = data;
    str->length = length;
    str->capacity = capacity;
    v.as.as_string = str;
    return v;
}

// ========== BUFFER OPERATIONS ==========

void buffer_free(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

Value val_buffer(int size) {
    if (size <= 0) {
        fprintf(stderr, "Runtime error: buffer size must be positive\n");
        exit(1);
    }

    Value v;
    v.type = VAL_BUFFER;
    Buffer *buf = malloc(sizeof(Buffer));
    buf->data = malloc(size);
    if (buf->data == NULL) {
        fprintf(stderr, "Runtime error: Failed to allocate buffer\n");
        exit(1);
    }
    buf->length = size;
    buf->capacity = size;
    v.as.as_buffer = buf;
    return v;
}

Value val_file(FileHandle *file) {
    Value v;
    v.type = VAL_FILE;
    v.as.as_file = file;
    return v;
}

// ========== ARRAY OPERATIONS ==========

Array* array_new(void) {
    Array *arr = malloc(sizeof(Array));
    arr->capacity = 8;
    arr->length = 0;
    arr->elements = malloc(sizeof(Value) * arr->capacity);
    return arr;
}

void array_free(Array *arr) {
    if (arr) {
        free(arr->elements);
        free(arr);
    }
}

static void array_grow(Array *arr) {
    arr->capacity *= 2;
    arr->elements = realloc(arr->elements, sizeof(Value) * arr->capacity);
}

void array_push(Array *arr, Value val) {
    if (arr->length >= arr->capacity) {
        array_grow(arr);
    }
    arr->elements[arr->length++] = val;
}

Value array_pop(Array *arr) {
    if (arr->length == 0) {
        return val_null();
    }
    return arr->elements[--arr->length];
}

Value array_get(Array *arr, int index) {
    if (index < 0 || index >= arr->length) {
        fprintf(stderr, "Runtime error: Array index %d out of bounds (length %d)\n",
                index, arr->length);
        exit(1);
    }
    return arr->elements[index];
}

void array_set(Array *arr, int index, Value val) {
    if (index < 0) {
        fprintf(stderr, "Runtime error: Negative array index not supported\n");
        exit(1);
    }

    // Extend array if needed, filling with nulls
    while (index >= arr->length) {
        array_push(arr, val_null());
    }

    arr->elements[index] = val;
}

Value val_array(Array *arr) {
    Value v;
    v.type = VAL_ARRAY;
    v.as.as_array = arr;
    return v;
}

// ========== FILE OPERATIONS ==========

void file_free(FileHandle *file) {
    if (file) {
        if (file->fp && !file->closed) {
            fclose(file->fp);
        }
        if (file->path) free(file->path);
        if (file->mode) free(file->mode);
        free(file);
    }
}

// ========== OBJECT OPERATIONS ==========

void object_free(Object *obj) {
    if (obj) {
        if (obj->type_name) free(obj->type_name);
        for (int i = 0; i < obj->num_fields; i++) {
            free(obj->field_names[i]);
            // Note: field values are NOT freed (memory leak in v0.1)
        }
        free(obj->field_names);
        free(obj->field_values);
        free(obj);
    }
}

Object* object_new(char *type_name, int initial_capacity) {
    Object *obj = malloc(sizeof(Object));
    obj->type_name = type_name ? strdup(type_name) : NULL;
    obj->field_names = malloc(sizeof(char*) * initial_capacity);
    obj->field_values = malloc(sizeof(Value) * initial_capacity);
    obj->num_fields = 0;
    obj->capacity = initial_capacity;
    return obj;
}

Value val_object(Object *obj) {
    Value v;
    v.type = VAL_OBJECT;
    v.as.as_object = obj;
    return v;
}

// ========== VALUE OPERATIONS ==========

Value val_i8(int8_t value) {
    Value v;
    v.type = VAL_I8;
    v.as.as_i8 = value;
    return v;
}

Value val_i16(int16_t value) {
    Value v;
    v.type = VAL_I16;
    v.as.as_i16 = value;
    return v;
}

Value val_i32(int32_t value) {
    Value v;
    v.type = VAL_I32;
    v.as.as_i32 = value;
    return v;
}

Value val_u8(uint8_t value) {
    Value v;
    v.type = VAL_U8;
    v.as.as_u8 = value;
    return v;
}

Value val_u16(uint16_t value) {
    Value v;
    v.type = VAL_U16;
    v.as.as_u16 = value;
    return v;
}

Value val_u32(uint32_t value) {
    Value v;
    v.type = VAL_U32;
    v.as.as_u32 = value;
    return v;
}

Value val_f32(float value) {
    Value v;
    v.type = VAL_F32;
    v.as.as_f32 = value;
    return v;
}

Value val_f64(double value) {
    Value v;
    v.type = VAL_F64;
    v.as.as_f64 = value;
    return v;
}

Value val_int(int value) {
    return val_i32((int32_t)value);
}

Value val_float(double value) {
    return val_f64(value);
}

Value val_bool(int value) {
    Value v;
    v.type = VAL_BOOL;
    v.as.as_bool = value ? 1 : 0;
    return v;
}

Value val_ptr(void *ptr) {
    Value v;
    v.type = VAL_PTR;
    v.as.as_ptr = ptr;
    return v;
}

Value val_type(TypeKind kind) {
    Value v;
    v.type = VAL_TYPE;
    v.as.as_type = kind;
    return v;
}

Value val_function(Function *fn) {
    Value v;
    v.type = VAL_FUNCTION;
    v.as.as_function = fn;
    return v;
}

Value val_null(void) {
    Value v;
    v.type = VAL_NULL;
    return v;
}

void print_value(Value val) {
    switch (val.type) {
        case VAL_I8:
            printf("%d", val.as.as_i8);
            break;
        case VAL_I16:
            printf("%d", val.as.as_i16);
            break;
        case VAL_I32:
            printf("%d", val.as.as_i32);
            break;
        case VAL_U8:
            printf("%u", val.as.as_u8);
            break;
        case VAL_U16:
            printf("%u", val.as.as_u16);
            break;
        case VAL_U32:
            printf("%u", val.as.as_u32);
            break;
        case VAL_F32:
            printf("%g", val.as.as_f32);
            break;
        case VAL_F64:
            printf("%g", val.as.as_f64);
            break;
        case VAL_BOOL:
            printf("%s", val.as.as_bool ? "true" : "false");
            break;
        case VAL_STRING:
            printf("%s", val.as.as_string->data);
            break;
        case VAL_PTR:
            printf("%p", val.as.as_ptr);
            break;
        case VAL_BUFFER:
            printf("<buffer %p length=%d capacity=%d>",
                   val.as.as_buffer->data,
                   val.as.as_buffer->length,
                   val.as.as_buffer->capacity);
            break;
        case VAL_ARRAY: {
            Array *arr = val.as.as_array;
            printf("[");
            for (int i = 0; i < arr->length; i++) {
                if (i > 0) printf(", ");
                print_value(arr->elements[i]);
            }
            printf("]");
            break;
        }
        case VAL_FILE: {
            FileHandle *file = val.as.as_file;
            if (file->closed) {
                printf("<file (closed)>");
            } else {
                printf("<file '%s' mode='%s'>", file->path, file->mode);
            }
            break;
        }
        case VAL_OBJECT:
            if (val.as.as_object->type_name) {
                printf("<object:%s>", val.as.as_object->type_name);
            } else {
                printf("<object>");
            }
            break;
        case VAL_TYPE:
            printf("<type>");
            break;
        case VAL_BUILTIN_FN:
            printf("<builtin function>");
            break;
        case VAL_FUNCTION:
            printf("<function>");
            break;
        case VAL_NULL:
            printf("null");
            break;
    }
}
