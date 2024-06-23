#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

Stack* stack_init() {
    Stack* stack = malloc(sizeof(Stack));
    if (!stack) {
        fprintf(stderr, "Error allocating memory for stack\n");
        exit(EXIT_FAILURE);
    }

    stack->items = (int32_t *)malloc(sizeof(int32_t) * 1);
    if (!stack->items) {
        fprintf(stderr, "Error allocating memory for stack items\n");
        free(stack);
        exit(EXIT_FAILURE);
    }
    
    stack->size = 0;
    stack->capacity = 1;

    return stack;
}

void stack_push(Stack *stack, int32_t item) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->items = (int32_t *)realloc(stack->items, sizeof(int32_t) * stack->capacity);
        if (!stack->items) {
            fprintf(stderr, "Error reallocating memory for stack items\n");
            exit(EXIT_FAILURE);
        }
    }
    stack->items[stack->size++] = item;
}

int32_t stack_pop(Stack *stack) {
    if (stack_is_empty(stack)) {
        fprintf(stderr, "Error: Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return stack->items[--stack->size];
}

int32_t stack_peek(Stack *stack) {
    if (stack_is_empty(stack)) {
        fprintf(stderr, "Error: Stack is empty\n");
        exit(EXIT_FAILURE);
    }
    return stack->items[stack->size - 1];
}

int stack_is_empty(Stack *stack) {
    return stack->size == 0;
}

void stack_free(Stack *stack) {
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
    free(stack);
}
