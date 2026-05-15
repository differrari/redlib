#pragma once
#include "chunk_array.h"

typedef chunk_array_t arr_stack_t;

#define stack_create_alloc chunk_array_create_alloc

#define stack_create chunk_array_create

#define stack_push chunk_array_push

#define stack_destroy chunk_array_destroy

#define stack_reset chunk_array_reset

#define stack_remove chunk_array_remove

#define stack_get chunk_array_get

#define stack_count chunk_array_count

#define stack_find chunk_array_find

#define stack_test chunk_array_test

#define STACK_GET(type,array,index) (*(type*)stack_get(array,index))