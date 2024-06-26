#ifndef __SHADER_H__
#define __SHADER_H__

#include <gs/gs.h>

#define SRC_MAX_LENGTH 1024

typedef struct rxcore_shader_desc_t
{
    const char **shader_dependencies;
    uint16_t shader_dependency_count;
    const char *shader_name;
    const char *shader_path;
} rxcore_shader_desc_t;

typedef struct rxcore_shader_t
{
    const char *shader_name;
    const char *shader_src;
} rxcore_shader_t;

typedef struct rxcore_shader_registry_t
{
    gs_dyn_array(rxcore_shader_t) shaders;
    gs_dyn_array(rxcore_shader_t) dependencies;
} rxcore_shader_registry_t;

typedef struct rxcore_shader_set_t
{
    uint16_t vertex_shader_id;
    uint16_t fragment_shader_id;
} rxcore_shader_set_t;

// RXCORE_SHADER public methods
rxcore_shader_desc_t rxcore_shader_desc_create(const char *shader_name, const char *shader_path, const char **shader_dependencies, uint16_t shader_dependency_count);
#define RXCORE_SHADER_DEPENDENCIES(...) (const char *[]){__VA_ARGS__}, sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *)
#define RXCORE_SHADER_DESC_WITH_INCLUDE(shader_name, shader_path, ...) rxcore_shader_desc_create(shader_name, shader_path, RXCORE_SHADER_DEPENDENCIES(__VA_ARGS__))

// RXCORE_SHADER private methods
rxcore_shader_t *_rxcore_shader_create(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);
void _rxcore_shader_destroy(rxcore_shader_t *shader);

// RXCORE_SHADER_REGISTRY public methods
rxcore_shader_registry_t *rxcore_shader_registry_create();
void rxcore_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char* dep_name, const char *dep_path);
uint32_t rxcore_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);
rxcore_shader_set_t rxcore_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name);
void rxcore_shader_registry_destroy(rxcore_shader_registry_t *reg);

// RXCORE_SHADER_REGISTRY private methods
uint16_t _rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name);
rxcore_shader_t *_rxcore_shader_registry_find_shader(rxcore_shader_registry_t *reg, const char *shader_name);

#endif // __SHADER_H__