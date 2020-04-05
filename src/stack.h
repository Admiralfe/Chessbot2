#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct Move_state;

typedef struct MS_Stack MS_Stack;

MS_Stack* stk_create(size_t initial_size);
void stk_destroy(MS_Stack *stack);
struct Move_state stk_pop(MS_Stack *stack);
int stk_push(MS_Stack *stack, const struct Move_state *ms);
bool stk_empty(MS_Stack *stack);

#endif