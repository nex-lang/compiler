#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct Stack {
    __uint128_t* data;
    size_t size;
    size_t capacity;
} Stack;

Stack* stack_init();
void stack_push(Stack* stack, __uint128_t value);
__uint128_t stack_pop(Stack* stack);
__uint128_t stack_peek(Stack* stack);
bool stack_is_empty(Stack* stack);
void stack_free(Stack* stack);

#endif // STACK_H
