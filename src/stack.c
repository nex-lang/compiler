#include "stack.h"

Stack* stack_init() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->capacity = 8;
    stack->size = 0;
    stack->data = (__uint128_t*)malloc(stack->capacity * sizeof(__uint128_t));
    return stack;
}

void stack_push(Stack* stack, __uint128_t value) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->data = (__uint128_t*)realloc(stack->data, stack->capacity * sizeof(__uint128_t));
    }
    stack->data[stack->size++] = value;
}

__uint128_t stack_pop(Stack* stack) {
    if (stack->size == 0) {
        return 0;
    }
    return stack->data[--stack->size];
}

__uint128_t stack_peek(Stack* stack) {
    if (stack->size == 0) {
        return 0;
    }
    return stack->data[stack->size - 1];
}

bool stack_is_empty(Stack* stack) {
    return stack->size == 0;
}

void stack_free(Stack* stack) {
    free(stack->data);
    free(stack);
}
