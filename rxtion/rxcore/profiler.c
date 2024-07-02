// profiler.c

#include <rxcore/profiler.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gs/gs.h>

// Initialize the global profiler
static rxcore_profiler_t g_profiler = {0};

void rxcore_profiling_system_init() {
    g_profiler = rxcore_profiler_create();
}

void rxcore_profiling_system_update() {
    // For now, do nothing here
}

void rxcore_profiling_system_shutdown() {
    rxcore_profiler_destroy(&g_profiler);
}

rxcore_profiling_task_t *rxcore_profiling_task_create(const char *name) {
    rxcore_profiling_task_t *task = (rxcore_profiling_task_t *)std_malloc(sizeof(rxcore_profiling_task_t));
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

rxcore_profiling_task_t *rxcore_profiling_task_get_child(rxcore_profiling_task_t *task, const char *name) {
    for (int i = 0; i < gs_dyn_array_size(task->children); ++i) {
        if (strcmp(task->children[i]->name, name) == 0) {
            return task->children[i];
        }
    }
    return NULL;
}

inline bool rxcore_profiling_task_is_done(rxcore_profiling_task_t *task) {
    return task->end != 0;
}

void rxcore_profiling_task_destroy(rxcore_profiling_task_t *task) {
    for (int i = 0; i < gs_dyn_array_size(task->children); ++i) {
        rxcore_profiling_task_destroy(task->children[i]);
    }
    gs_dyn_array_free(task->children);
    free((void *)task->name);
    free(task);
}

rxcore_profiler_t rxcore_profiler_create() {
    rxcore_profiler_t profiler = {0};
    profiler.tasks = gs_dyn_array_new(rxcore_profiling_task_t *);
    profiler.stack = gs_dyn_array_new(rxcore_profiling_task_t *);
    profiler.stack_index = 0;
    profiler.current_task = NULL;
    return profiler;
}

void rxcore_profiler_get_task(rxcore_profiler_t *profiler, const char *name) {
    for (int i = 0; i < gs_dyn_array_size(profiler->tasks); ++i) {
        if (strcmp(profiler->tasks[i]->name, name) == 0) {
            profiler->current_task = profiler->tasks[i];
            return;
        }
    }
    profiler->current_task = NULL;
}

void rxcore_profiler_get_current_task(rxcore_profiler_t *profiler) {
    if (profiler->stack_index > 0) {
        profiler->current_task = profiler->stack[profiler->stack_index - 1];
    } else {
        profiler->current_task = NULL;
    }
}

void rxcore_profiler_begin_task(rxcore_profiler_t *profiler, const char *name) {
    rxcore_profiling_task_t *task = rxcore_profiling_task_create(name);
    if (profiler->current_task) {
        gs_dyn_array_push(profiler->current_task->children, task);
    } else {
        gs_dyn_array_push(profiler->tasks, task);
    }
    gs_dyn_array_push(profiler->stack, task);
    profiler->stack_index++;
    profiler->current_task = task;
}

void rxcore_profiler_end_task(rxcore_profiler_t *profiler) {
    if (profiler->current_task) {
        profiler->current_task->end = (double)clock() / CLOCKS_PER_SEC;
        profiler->stack_index--;
        gs_dyn_array_pop(profiler->stack);
        rxcore_profiler_get_current_task(profiler);
    }
}

void rxcore_profiler_report(rxcore_profiler_t *profiler) {
    printf("Profiling Report:\n");
    for (int i = 0; i < gs_dyn_array_size(profiler->tasks); ++i) {
        rxcore_profiling_task_t *task = profiler->tasks[i];
        printf("Task %s: Start: %.3f, End: %.3f, Duration: %.3f\n", task->name, task->start, task->end, task->end - task->start);
        printf("Mallocs: %u, Frees: %u, Reallocs: %u, Bytes Allocated: %u, Bytes Freed: %u\n",
               task->num_mallocs, task->num_frees, task->num_reallocs, task->bytes_allocated, task->bytes_freed);
    }
}

void rxcore_profiler_destroy(rxcore_profiler_t *profiler) {
    for (int i = 0; i < gs_dyn_array_size(profiler->tasks); ++i) {
        rxcore_profiling_task_destroy(profiler->tasks[i]);
    }
    gs_dyn_array_free(profiler->tasks);
    gs_dyn_array_free(profiler->stack);
}

void* rxcore_profiler_malloc(size_t size) {
    void *ptr = std_malloc(size + sizeof(size_t));
    *((size_t *)ptr) = size;
    ptr = (void *)((size_t)ptr + 1);

    if (g_profiler.current_task) {
        g_profiler.current_task->num_mallocs++;
        g_profiler.current_task->bytes_allocated += size;
    }
    return ptr;
}

void rxcore_profiler_free(void *ptr) {
    if (g_profiler.current_task) {
        g_profiler.current_task->num_frees++;
        g_profiler.current_task->bytes_freed += *((size_t *)((size_t)ptr - 1));
    }
    std_free(ptr);
}
