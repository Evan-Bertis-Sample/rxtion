#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <stdint.h>
#include <gs/gs.h>

#define RXCORE_PROFILING_ENABLED

/**
 * Example Usage
 * RXCORE_PROFILER_BEGIN_TASK("Task 1");
 * do some stuff...
 * RXCORE_PROFILER_BEGIN_TASK("Task 1 Subprocess");
 * RXCORE_PROFILER_END_TASK("Task 1 Subprocess");
 * RXCORE_PROFILER_END_TASK("Task 1");
 */

typedef struct rxcore_profiling_task_t rxcore_profiling_task_t;

typedef struct rxcore_profiling_task_t
{
    const char *name;
    double start;
    double end;
    uint32_t num_mallocs;
    uint32_t num_frees;
    uint32_t num_reallocs;
    uint32_t bytes_allocated;
    uint32_t bytes_freed;
    gs_dyn_array(rxcore_profiling_task_t *) children;
} rxcore_profiling_task_t;

typedef struct rxcore_profiler_t
{
    gs_dyn_array(rxcore_profiling_task_t *) tasks;
    gs_dyn_array(rxcore_profiling_task_t *) stack;
    uint32_t stack_index;
    rxcore_profiling_task_t *current_task;
} rxcore_profiler_t;

static rxcore_profiler_t g_profiler;

void rxcore_profiling_system_init();
void rxcore_profiling_system_update();
void rxcore_profiling_system_shutdown();

rxcore_profiling_task_t *rxcore_profiling_task_create(const char *name);
rxcore_profiling_task_t *rxcore_profiling_task_get_child(rxcore_profiling_task_t *task, const char *name);
inline bool rxcore_profiling_task_is_done(rxcore_profiling_task_t *task);
void rxcore_profiling_task_destroy(rxcore_profiling_task_t *task);

rxcore_profiler_t rxcore_profiler_create();
void rxcore_profiler_get_task(rxcore_profiler_t *profiler, const char *name);
void rxcore_profiler_get_current_task(rxcore_profiler_t *profiler);
void rxcore_profiler_begin_task(rxcore_profiler_t *profiler, const char *name);
void rxcore_profiler_end_task(rxcore_profiler_t *profiler);
void rxcore_profiler_report(rxcore_profiler_t *profiler);
void rxcore_profiler_destroy(rxcore_profiler_t *profiler);

void* rxcore_profiler_malloc(size_t size);
void rxcore_profiler_free(void *ptr);

// For internal use
#define std_malloc malloc
#define std_free free

#ifdef RXCORE_PROFILING_ENABLED
#define RXCORE_PROFILER_BEGIN_TASK(name) rxcore_profiler_begin_task(&g_profiler, name)
#define RXCORE_PROFILER_END_TASK(name) rxcore_profiler_end_task(&g_profiler)
#define RXCORE_PROFILER_REPORT() rxcore_profiler_report(&g_profiler)

// redefine malloc, free, realloc
#define malloc(size) rxcore_profiler_malloc(size)
#define free(ptr) rxcore_profiler_free(ptr)

#else
#define RXCORE_PROFILER_BEGIN_TASK(name) ((void)0)
#define RXCORE_PROFILER_END_TASK(name) ((void)0)
#define RXCORE_PROFILER_REPORT() ((void)0)
#endif

#define rxcore_profiling_system RXCORE_SYSTEM(rxcore_profiling_system_init, rxcore_profiling_system_update, rxcore_profiling_system_shutdown)

#endif // __PROFILER_H__