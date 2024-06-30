#ifndef __SHADER_H__
#define __SHADER_H__

#include <gs/gs.h>

#define SRC_MAX_LENGTH 1024
#define RXCORE_SHADER_DEBUG

#ifdef RXCORE_SHADER_DEBUG
#define RXCORE_SHADER_DEBUG_PRINT(str) gs_println("RXCORE::rendering::shader::" str)
#define RXCORE_SHADER_DEBUG_PRINTF(str, ...) gs_println("RXCORE::rendering::shader::" str, __VA_ARGS__)
#else
#define RXCORE_SHADER_DEBUG_PRINT(...) ((void)0)
#endif

typedef enum rxcore_shader_stage_t
{
    RXCORE_SHADER_STAGE_VERTEX = GS_GRAPHICS_SHADER_STAGE_VERTEX,
    RXCORE_SHADER_STAGE_FRAGMENT = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
    RXCORE_SHADER_STAGE_COMPUTE = GS_GRAPHICS_SHADER_STAGE_COMPUTE,
    RXCORE_SHADER_STAGE_DEPENDENCY = 4,
    RXCORE_SHADER_STAGE_COUNT 
} rxcore_shader_stage_t;


typedef struct rxcore_shader_desc_t
{
    rxcore_shader_stage_t stage;
    const char **shader_dependencies;
    uint16_t shader_dependency_count;
    const char *shader_name;
    const char *shader_path;
} rxcore_shader_desc_t;

typedef struct rxcore_shader_t
{
    rxcore_shader_stage_t stage;
    const char *shader_name;
    const char *shader_src;
} rxcore_shader_t;

typedef struct rxcore_shader_registry_t
{
    gs_dyn_array(rxcore_shader_t *) shaders;
    gs_dyn_array(rxcore_shader_t *) dependencies;

} rxcore_shader_registry_t;

typedef struct rxcore_shader_set_t
{
    rxcore_shader_t *vertex_shader;
    rxcore_shader_t *fragment_shader;
} rxcore_shader_set_t;

typedef struct rxcore_shader_program_t
{
    const char *program_name;
    gs_handle(gs_graphics_shader_t) program;
} rxcore_shader_program_t;

// RXCORE_SHADER public methods
rxcore_shader_desc_t rxcore_shader_desc_create(const char *shader_name, const char *shader_path, rxcore_shader_stage_t stage, const char **shader_dependencies, uint16_t shader_dependency_count);
#define RXCORE_SHADER_DEPENDENCIES(...) (const char *[]){__VA_ARGS__}, sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *)
#define RXCORE_SHADER_DESC_WITH_INCLUDE(shader_name, shader_path, stage, ...) rxcore_shader_desc_create(shader_name, shader_path, stage, RXCORE_SHADER_DEPENDENCIES(__VA_ARGS__))

// RXCORE_SHADER private methods
rxcore_shader_t *_rxcore_shader_create(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);
void _rxcore_shader_destroy(rxcore_shader_t *shader);

// RXCORE_SHADER_REGISTRY public methods
rxcore_shader_registry_t *rxcore_shader_registry_create();
void rxcore_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char *dep_name, const char *dep_path);
rxcore_shader_t *rxcore_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);
rxcore_shader_set_t rxcore_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name);
void rxcore_shader_registry_destroy(rxcore_shader_registry_t *reg);

// RXCORE_SHADER_REGISTRY private methods
rxcore_shader_t *_rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name);
rxcore_shader_t *_rxcore_shader_registry_find_shader(rxcore_shader_registry_t *reg, const char *shader_name);

// RXCORE_SHADER_SET public methods
rxcore_shader_program_t *rxcore_shader_program_set(rxcore_shader_set_t set);
void rxcore_shader_program_destroy(rxcore_shader_program_t *program);


#endif // __SHADER_H__