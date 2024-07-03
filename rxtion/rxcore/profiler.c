// profiler.c

#include <rxcore/profiler.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gs/gs.h>

// undefine malloc and free redefined in rxcore_profiler.h
// this is to use the system malloc and free functions
#undef malloc
#undef free

void rxcore_profiling_system_init()
{
    g_profiler = rxcore_profiler_create();
}

void rxcore_profiling_system_update()
{
    // For now, do nothing here
}

void rxcore_profiling_system_shutdown()
{
    rxcore_profiler_destroy(&g_profiler);
}

rxcore_profiling_task_t *rxcore_profiling_task_create(const char *name)
{
    rxcore_profiling_task_t *task = (rxcore_profiling_task_t *)malloc(sizeof(rxcore_profiling_task_t));
    task->name = strdup(name);
    task->start = (double)clock() / CLOCKS_PER_SEC;
    task->end = 0;
    task->num_mallocs = 0;
    task->num_frees = 0;
    task->num_reallocs = 0;
    task->bytes_allocated = 0;
    task->bytes_freed = 0;
    task->children = gs_dyn_array_new(rxcore_profiling_task_t *);
    return task;
}


inline bool rxcore_profiling_task_is_done(rxcore_profiling_task_t *task)
{
    return task->end != 0;
}

void rxcore_profiling_task_traverse(rxcore_profiling_task_t *task, rxcore_profiling_task_traversal_fn fn, uint32_t depth, void *user_data)
{
    // dfs of task tree
    fn(task, depth, user_data);
    for (int i = 0; i < gs_dyn_array_size(task->children); ++i)
    {
        rxcore_profiling_task_traverse(task->children[i], fn, depth + 1, user_data);
    }
}

void rxcore_profiling_task_traversal_print(rxcore_profiling_task_t *task, uint32_t depth, void* user_data)
{
    char *indent = malloc(depth + 1);
    memset(indent, ' ', depth);
    indent[depth] = '\0';

    void (*print_fn)(const char *str, ...) = (void (*)(const char *str, ...))user_data;

    print_fn("%s%s: Start: %.3f, End: %.3f, Duration: %.3f\n", indent, task->name, task->start, task->end, task->end - task->start);
    print_fn("%s%s%s: Mallocs: %d, Frees: %d, Bytes Allocated: %d, Bytes Freed %d, Bytes Unfreed %d\n", indent, indent, task->name, task->bytes_allocated, task->bytes_freed, task->bytes_allocated - task->bytes_freed);
    print_fn("\n");

    free(indent);
}

void rxcore_profiling_task_destroy(rxcore_profiling_task_t *task)
{
    for (int i = 0; i < gs_dyn_array_size(task->children); ++i)
    {
        rxcore_profiling_task_destroy(task->children[i]);
    }
    gs_dyn_array_free(task->children);
    free((void *)task->name);
    free(task);
}

rxcore_profiler_t rxcore_profiler_create()
{
    rxcore_profiler_t profiler = {0};
    profiler.completed_tasks = gs_dyn_array_new(rxcore_profiling_task_t *);
    profiler.stack = gs_dyn_array_new(rxcore_profiling_task_t *);
    profiler.stack_index = 0;
    return profiler;
}

rxcore_profiling_task_t *rxcore_profiler_get_current_task(rxcore_profiler_t *profiler)
{
    if (rxcore_profiler_any_tasks(profiler))
    {
        return profiler->stack[profiler->stack_index - 1];
    }

    return NULL;
}

void rxcore_profiler_begin_task(rxcore_profiler_t *profiler, const char *name)
{
    rxcore_profiling_task_t *task = rxcore_profiling_task_create(name);
    if (rxcore_profiler_any_tasks(profiler))
    {
        rxcore_profiling_task_t *parent = profiler->stack[profiler->stack_index - 1];
        gs_dyn_array_push(parent->children, task);
    }

    gs_dyn_array_push(profiler->stack, task);
    profiler->stack_index++;
}

void rxcore_profiler_end_task(rxcore_profiler_t *profiler)
{
    if (!rxcore_profiler_any_tasks(profiler))
    {
        return;
    }

    rxcore_profiling_task_t *task = profiler->stack[profiler->stack_index - 1];

    task->end = (double)clock() / CLOCKS_PER_SEC;
    profiler->stack_index--;

    // now update all the data of the parents of the tasks
    for (int i = profiler->stack_index - 1; i >= 0; --i)
    {
        rxcore_profiling_task_t *parent = profiler->stack[i];
        parent->bytes_allocated += task->bytes_allocated;
        parent->bytes_freed += task->bytes_freed;
        parent->num_mallocs += task->num_mallocs;
        parent->num_frees += task->num_frees;
        parent->num_reallocs += task->num_reallocs;
    }

    // is this task complete?
    // we know a task is complete if this is the last thing in the stack
    if (profiler->stack_index == 0)
    {
        // it is complete, add it to the completed tasks
        gs_println("Completeing task: %s", task->name);
        gs_dyn_array_push(profiler->completed_tasks, task);
    }
}

bool rxcore_profiler_any_tasks(rxcore_profiler_t *profiler)
{
    return profiler->stack_index > 0;
}

void rxcore_profiler_report(rxcore_profiler_t *profiler)
{
    printf("Profiling Report:\n");
    for (int i = 0; i < gs_dyn_array_size(profiler->completed_tasks); ++i)
    {
        rxcore_profiling_task_t *task = profiler->completed_tasks[i];
        printf("Task %s: Start: %.3f, End: %.3f, Duration: %.3f\n", task->name, task->start, task->end, task->end - task->start);
        printf("Mallocs: %u, Frees: %u, Reallocs: %u, Bytes Allocated: %u, Bytes Freed: %u\n",
               task->num_mallocs, task->num_frees, task->num_reallocs, task->bytes_allocated, task->bytes_freed);
    }
}

void rxcore_profiler_destroy(rxcore_profiler_t *profiler)
{
    for (int i = 0; i < gs_dyn_array_size(profiler->completed_tasks); ++i)
    {
        rxcore_profiling_task_destroy(profiler->completed_tasks[i]);
    }
    gs_dyn_array_free(profiler->completed_tasks);
    gs_dyn_array_free(profiler->stack);
}

void *rxcore_profiler_malloc(size_t size)
{
    gs_println("custom malloc called");
    void *ptr = malloc(size + sizeof(size_t));
    *((size_t *)ptr) = size;
    ptr = (void *)((size_t)ptr + 1);
    if (rxcore_profiler_any_tasks(&g_profiler))
    {
        rxcore_profiling_task_t *current_task = rxcore_profiler_get_current_task(&g_profiler);
        current_task->num_mallocs++;
        current_task->bytes_allocated += size;
    }
    return ptr;
}

void rxcore_profiler_free(void *ptr)
{
    gs_println("custom free called");
    if (!ptr)
    {
        return;
    }

    ptr = (void *)((size_t)ptr - 1);
    size_t size = *((size_t *)ptr);
    if (rxcore_profiler_any_tasks(&g_profiler))
    {
        rxcore_profiling_task_t *current_task = rxcore_profiler_get_current_task(&g_profiler);
        current_task->num_frees++;
        current_task->bytes_freed += size;
    }

    free(ptr);
}
