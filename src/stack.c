#include <stdlib.h>
#include "stack.h"
#include "position.h"

#define REALLOC_SIZE 128

struct MS_Stack {
    size_t top;
    size_t num_items;
    
    struct Move_state *arr;
};

MS_Stack* stk_create(size_t initial_size) {
    MS_Stack *new_stk = malloc(sizeof(MS_Stack));
    if (new_stk == NULL) {
        free(new_stk);
        return NULL;
    }
    new_stk->top = 0;
    new_stk->num_items = initial_size;
    new_stk->arr = malloc(sizeof(struct Move_state) * initial_size);

    return new_stk;
}

void stk_destroy(MS_Stack *stack) {
    free(stack->arr);
    free(stack);
}

int stk_push(MS_Stack *stack, struct Move_state ms) {
    if (stack->top >= stack->num_items) {
        stack->arr = realloc(stack->arr, sizeof(struct Move_state) * REALLOC_SIZE);
        
        if (stack->arr == NULL)
            return -1;
        
        stack->num_items += REALLOC_SIZE;
    }

    stack->arr[stack->top++] = ms;
    return 0;
}

struct Move_state stk_pop(MS_Stack *stack) {
    return stack->arr[--stack->top];
}

bool stk_empty(MS_Stack *stack) {
    return stack->top == 0;
}