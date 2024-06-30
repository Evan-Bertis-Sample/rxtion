#ifndef __MESH_H__
#define __MESH_H__

#include <gs/gs.h>

typedef struct rxcore_vertex_t
{
    gs_vec3 position;
    gs_vec3 normal;
    gs_vec2 uv;
} rxcore_vertex_t;

typedef struct rxcore_mesh_buffer_t
{
    gs_dyn_array(rxcore_vertex_t) vertices;
    gs_dyn_array(uint32_t) indices;
} rxcore_mesh_buffer_t;

typedef struct rxcore_mesh_t
{
    rxcore_mesh_buffer_t *buffer;
    uint32_t starting_index;
    uint32_t index_count;
} rxcore_mesh_t;

typedef struct rxcore_mesh_registry_t
{
    rxcore_mesh_buffer_t buffer;
    gs_dyn_array(rxcore_mesh_t) meshes;
    gs_dyn_array(const char *) mesh_names;
} rxcore_mesh_registry_t;

rxcore_mesh_buffer_t *rxcore_mesh_buffer_create();
rxcore_mesh_t rxcore_mesh_buffer_add_mesh(rxcore_mesh_buffer_t *buffer, rxcore_vertex_t *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t index_count);
rxcore_mesh_t rxcore_mesh_buffer_add_mesh_from_file(rxcore_mesh_buffer_t *buffer, const char *file_path);
void rxcore_mesh_buffer_destroy(rxcore_mesh_buffer_t *buffer);

rxcore_vertex_t *rxcore_mesh_get_vertices(rxcore_mesh_t *mesh);
uint32_t *rxcore_mesh_get_indices(rxcore_mesh_t *mesh);

rxcore_mesh_registry_t *rxcore_mesh_registry_create();
rxcore_mesh_t *rxcore_mesh_registry_add_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name, rxcore_vertex_t *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t index_count);
rxcore_mesh_t *rxcore_mesh_registry_add_mesh_from_file(rxcore_mesh_registry_t *reg, const char *mesh_name, const char *file_path);
rxcore_mesh_t rxcore_mesh_registry_get_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name);
void rxcore_mesh_registry_destroy(rxcore_mesh_registry_t *reg);

#endif // __MESH_H__