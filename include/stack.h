#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>

typedef struct Stack {
    int32_t *items;
    size_t size;
    size_t capacity;
} Stack;

Stack* stack_init();
void stack_push(Stack* stack, int32_t item);
int32_t stack_pop(Stack* stack);
int32_t stack_peek(Stack* stack);
int stack_is_empty(Stack* stack);
void stack_free(Stack* stack);

#endif // STACK_H
