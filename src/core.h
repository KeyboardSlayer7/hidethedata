#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <string.h>

typedef void (*Destructor)(void*);

#define ARRAY(T)                    \
typedef struct T##_dynamic_array_t  \
{                                   \
    size_t size;                    \
    size_t capacity;                \
    size_t element_size;            \
    T* data;                        \
    Destructor destructor;          \
} T##_dynamic_array;                \
                                    \
void init_dynamic_array_##T(T##_dynamic_array* array, Destructor destructor);   \
T* get_##T(T##_dynamic_array* array, size_t index);                             \
int resize_dynamic_array_##T(T##_dynamic_array* array);                         \
void append_##T(T##_dynamic_array* array, T* value);                            \
void free_dynamic_array_##T(T##_dynamic_array* array);

#define INIT_ARRAY(T)                                                           \
void init_dynamic_array_##T(T##_dynamic_array* array, Destructor destructor)    \
{                                                                               \
    array->size = 0;                                                            \
    array->capacity = 0;                                                        \
    array->element_size = sizeof(T);                                            \
    array->data = NULL;                                                         \
    array->destructor = destructor;                                             \
}

#define GET(T)                                      \
T* get_##T(T##_dynamic_array* array, size_t index)  \
{                                                   \
    if (index >= array->size)                       \
        return NULL;                                \
                                                    \
    return &array->data[index];                     \
}

#define RESIZE(T)                                                           \
int resize_dynamic_array_##T(T##_dynamic_array* array)                      \
{                                                                           \
    if (array->data == NULL)                                                \
    {                                                                       \
        array->capacity = 1;                                                \
                                                                            \
        void* temp = malloc(array->capacity * sizeof(T));                   \
                                                                            \
        if (temp == NULL)                                                   \
            return 1;                                                       \
                                                                            \
        array->data = (T*)temp;                                             \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        array->capacity *= 2;                                               \
                                                                            \
        void* temp = realloc(array->data, array->capacity * sizeof(T));     \
                                                                            \
        if (temp == NULL)                                                   \
            return -1;                                                      \
                                                                            \
        array->data = (T*)temp;                                             \
    }                                                                       \
                                                                            \
    return 0;                                                               \
}

#define APPEND(T)                                           \
void append_##T(T##_dynamic_array* array, T* value)         \
{                                                           \
    if (array->size >= array->capacity)                     \
        resize_dynamic_array_##T(array);                    \
                                                            \
    memcpy(&array->data[array->size], value, sizeof(T));    \
    array->size++;                                          \
}

#define FREE(T)                                         \
void free_dynamic_array_##T(T##_dynamic_array* array)   \
{                                                       \
    if (array->destructor != NULL)                      \
    {                                                   \
        for (int i = 0; i < array->size; ++i)           \
        {                                               \
            array->destructor(get_##T(array, i));       \
        }                                               \
    }                                                   \
                                                        \
    free(array->data);                                  \
}

#define NEW_ARRAY(T)    \
    INIT_ARRAY(T)       \
    GET(T)              \
    RESIZE(T)           \
    APPEND(T)           \
    FREE(T)

#endif // CORE_H
