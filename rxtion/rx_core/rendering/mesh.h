#ifndef __MESH_H__
#define __MESH_H__

#include <gs/gs.h>
#include <stdlib.h> 

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
    gs_handle(gs_graphics_vertex_buffer_t) vertex_buffer;
    gs_handle(gs_graphics_index_buffer_t) index_buffer;
    bool is_dirty; // if true, the vertex and index buffers need to be updated
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
gs_handle(gs_graphics_vertex_buffer_t) rxcore_mesh_buffer_get_vertex_buffer(rxcore_mesh_buffer_t *buffer);
gs_handle(gs_graphics_index_buffer_t) rxcore_mesh_buffer_get_index_buffer(rxcore_mesh_buffer_t *buffer);
void rxcore_mesh_buffer_destroy(rxcore_mesh_buffer_t *buffer);

rxcore_vertex_t *rxcore_mesh_get_vertices(rxcore_mesh_t *mesh);
uint32_t *rxcore_mesh_get_indices(rxcore_mesh_t *mesh);
void rxcore_mesh_draw(rxcore_mesh_t *mesh, gs_command_buffer_t *cb);
bool rxcore_mesh_is_empty(rxcore_mesh_t *mesh);

rxcore_mesh_registry_t *rxcore_mesh_registry_create();
rxcore_mesh_t *rxcore_mesh_registry_add_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name, rxcore_vertex_t *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t index_count);
rxcore_mesh_t *rxcore_mesh_registry_add_mesh_from_file(rxcore_mesh_registry_t *reg, const char *mesh_name, const char *file_path);
rxcore_mesh_t rxcore_mesh_registry_get_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name);
bool rxcore_mesh_registry_get_mesh_index(rxcore_mesh_registry_t *reg, const char *mesh_name, uint32_t *out_index);
void rxcore_mesh_registry_destroy(rxcore_mesh_registry_t *reg);

#endif // __MESH_H__