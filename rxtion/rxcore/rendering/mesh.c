// mesh.c

#include <rxcore/rendering/mesh.h>
#include <gs/gs.h>
#include <stdbool.h>

rxcore_mesh_buffer_t *rxcore_mesh_buffer_create()
{
    rxcore_mesh_buffer_t *buffer = malloc(sizeof(rxcore_mesh_buffer_t));
    buffer->vertices = gs_dyn_array_new(rxcore_vertex_t);
    buffer->indices = gs_dyn_array_new(uint32_t);
    buffer->vertex_dirty = true;
    buffer->index_dirty = true;
    buffer->vertex_generated = false;
    buffer->index_generated = false;
    return buffer;
}

rxcore_mesh_t rxcore_mesh_buffer_add_mesh(rxcore_mesh_buffer_t *buffer, rxcore_vertex_t *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t index_count)
{
    rxcore_mesh_t mesh = {0};
    mesh.buffer = buffer;
    mesh.starting_index = gs_dyn_array_size(buffer->indices);
    mesh.index_count = index_count;
    mesh.base_vertex = gs_dyn_array_size(buffer->vertices);

    for (uint32_t i = 0; i < vertex_count; i++)
    {
        gs_dyn_array_push(buffer->vertices, vertices[i]);
    }

    for (uint32_t i = 0; i < index_count; i++)
    {
        gs_dyn_array_push(buffer->indices, indices[i]);
    }

    buffer->vertex_dirty = true;
    buffer->index_dirty = true;

    return mesh;
}

rxcore_mesh_t rxcore_mesh_buffer_add_mesh_from_file(rxcore_mesh_buffer_t *buffer, const char *file_path)
{
    rxcore_vertex_t *vertices = NULL;
    uint32_t vertex_count = 0;
    uint32_t *indices = NULL;
    uint32_t index_count = 0;

    if (!rxcore_mesh_load_from_file(file_path, vertices, &vertex_count, indices, &index_count))
    {
        RXCORE_MESH_DEBUG_PRINTF("Failed to load mesh from file %s", file_path);
        return (rxcore_mesh_t){0};
    }

    return rxcore_mesh_buffer_add_mesh(buffer, vertices, vertex_count, indices, index_count);
}

gs_handle(gs_graphics_vertex_buffer_t) rxcore_mesh_buffer_get_vertex_buffer(rxcore_mesh_buffer_t *buffer)
{
    if (buffer->vertex_dirty)
    {
        // free the old buffer, if there was one
        if (buffer->vertex_generated)
        {
            gs_graphics_vertex_buffer_destroy(buffer->vertex_buffer);
        }

        // update the vertex buffer
        gs_graphics_vertex_buffer_desc_t desc = {
            .data = buffer->vertices,
            .size = sizeof(rxcore_vertex_t) * gs_dyn_array_size(buffer->vertices),
            .usage = GS_GRAPHICS_BUFFER_USAGE_STATIC};
        buffer->vertex_buffer = gs_graphics_vertex_buffer_create(&desc);

        buffer->vertex_dirty = false;

        gs_println("Generated vertex buffer with %d vertices", gs_dyn_array_size(buffer->vertices));
    }

    return buffer->vertex_buffer;
}

gs_handle(gs_graphics_index_buffer_t) rxcore_mesh_buffer_get_index_buffer(rxcore_mesh_buffer_t *buffer)
{
    if (buffer->index_dirty)
    {
        // free the old buffer, if there was one
        if (buffer->index_generated)
        {
            gs_graphics_index_buffer_destroy(buffer->index_buffer);
        }

        // update the index buffer
        gs_graphics_index_buffer_desc_t desc = {
            .data = buffer->indices,
            .size = sizeof(uint32_t) * gs_dyn_array_size(buffer->indices),
            .usage = GS_GRAPHICS_BUFFER_USAGE_STATIC};
        buffer->index_buffer = gs_graphics_index_buffer_create(&desc);

        buffer->index_dirty = false;
    }

    return buffer->index_buffer;
}

void rxcore_mesh_buffer_apply_bindings(rxcore_mesh_buffer_t *buffer, gs_command_buffer_t *cb)
{
    gs_handle(gs_graphics_vertex_buffer_t) vb = rxcore_mesh_buffer_get_vertex_buffer(buffer);
    gs_handle(gs_graphics_index_buffer_t) ib = rxcore_mesh_buffer_get_index_buffer(buffer);

    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vb}},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = ib}},
    };

    gs_graphics_apply_bindings(cb, &binds);
}

void rxcore_mesh_buffer_destroy(rxcore_mesh_buffer_t *buffer)
{
    gs_dyn_array_free(buffer->vertices);
    gs_dyn_array_free(buffer->indices);
    gs_graphics_vertex_buffer_destroy(buffer->vertex_buffer);
    gs_graphics_index_buffer_destroy(buffer->index_buffer);
    free(buffer);
}

rxcore_mesh_t rxcore_mesh_empty()
{
    return (rxcore_mesh_t){
        .buffer = NULL,
        .starting_index = 0,
        .base_vertex = 0,
        .index_count = 0};
}

rxcore_vertex_t *rxcore_mesh_get_vertices(rxcore_mesh_t *mesh)
{
    uint32_t *indices = rxcore_mesh_get_indices(mesh);
    rxcore_vertex_t *vertices = malloc(sizeof(rxcore_vertex_t) * mesh->index_count);

    for (uint32_t i = 0; i < mesh->index_count; i++)
    {
        vertices[i] = mesh->buffer->vertices[indices[i] + mesh->base_vertex];
    }

    return vertices;
}

uint32_t *rxcore_mesh_get_indices(rxcore_mesh_t *mesh)
{
    return mesh->buffer->indices + mesh->starting_index;
}

void rxcore_mesh_draw(rxcore_mesh_t *mesh, gs_command_buffer_t *cb)
{
    // gs_println("Drawing mesh starting at %d for %d verts", mesh->starting_index, mesh->index_count);
    gs_graphics_draw(cb, &(gs_graphics_draw_desc_t){
                            .start = 0,
                            .count = mesh->index_count,
                             .range = {
                                 .start = mesh->starting_index,
                                 .end = mesh->starting_index + mesh->index_count,
                             },
                             .base_vertex = mesh->base_vertex,
                             .instances = 1});
}

bool rxcore_mesh_load_from_file(const char *file_path, rxcore_vertex_t *vertex_out, uint32_t *vertex_count_out, uint32_t *indices_out, uint32_t *index_count_out)
{
    RXCORE_MESH_DEBUG_PRINTF("Loading mesh from file: %s", file_path);
    RXCORE_MESH_DEBUG_PRINT("Not implemented yet");
    return false;
}

bool rxcore_mesh_is_empty(rxcore_mesh_t *mesh)
{
    return mesh->index_count == 0;
}

void rxcore_mesh_print(rxcore_mesh_t *mesh, void (*print_func)(const char *, ...), bool add_newline)
{
    print_func("Mesh: %p ", mesh);
    print_func("Starting index: %d ", mesh->starting_index);
    print_func("Index count: %d ", mesh->index_count);
    print_func("Buffer: %p ", mesh->buffer);
    print_func("Vertices: %p\n");
    rxcore_vertex_t *vertices = rxcore_mesh_get_vertices(mesh);
    for (uint32_t i = 0; i < mesh->index_count; i++)
    {
        print_func("  Vertex: P %f, %f, %f, N %f, %f, %f, U %f, %f",
                   vertices[i].position.x, vertices[i].position.y, vertices[i].position.z,
                   vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z,
                   vertices[i].uv.x, vertices[i].uv.y);

        if (add_newline)
        {
            print_func("\n");
        }
    }

    print_func("Indices: %p\n");
    uint32_t *indices = rxcore_mesh_get_indices(mesh);
    for (uint32_t i = 0; i < mesh->index_count; i++)
    {
        print_func("  Index: %d", indices[i]);
        if (add_newline)
        {
            print_func("\n");
        }
    }
}

rxcore_mesh_registry_t *rxcore_mesh_registry_create()
{
    rxcore_mesh_registry_t *reg = malloc(sizeof(rxcore_mesh_registry_t));
    reg->buffer = rxcore_mesh_buffer_create();
    reg->meshes = gs_dyn_array_new(rxcore_mesh_t);
    reg->mesh_names = gs_dyn_array_new(const char *);
    return reg;
}

rxcore_mesh_t *rxcore_mesh_registry_add_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name, rxcore_vertex_t *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t index_count)
{
    rxcore_mesh_t mesh = rxcore_mesh_buffer_add_mesh(reg->buffer, vertices, vertex_count, indices, index_count);
    gs_dyn_array_push(reg->meshes, mesh);
    gs_dyn_array_push(reg->mesh_names, mesh_name);
    return &reg->meshes[gs_dyn_array_size(reg->meshes) - 1];
}

rxcore_mesh_t *rxcore_mesh_registry_add_mesh_from_file(rxcore_mesh_registry_t *reg, const char *mesh_name, const char *file_path)
{
    rxcore_mesh_t mesh = rxcore_mesh_buffer_add_mesh_from_file(reg->buffer, file_path);
    gs_dyn_array_push(reg->meshes, mesh);
    gs_dyn_array_push(reg->mesh_names, mesh_name);
    return &reg->meshes[gs_dyn_array_size(reg->meshes) - 1];
}

rxcore_mesh_t rxcore_mesh_registry_get_mesh(rxcore_mesh_registry_t *reg, const char *mesh_name)
{
    uint32_t index = 0;
    if (!rxcore_mesh_registry_get_mesh_index(reg, mesh_name, &index))
    {
        RXCORE_MESH_DEBUG_PRINTF("Failed to find mesh: %s", mesh_name);
        return (rxcore_mesh_t){0};
    }

    return reg->meshes[index];
}

bool rxcore_mesh_registry_get_mesh_index(rxcore_mesh_registry_t *reg, const char *mesh_name, uint32_t *index_out)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->mesh_names); i++)
    {
        if (strcmp(reg->mesh_names[i], mesh_name) == 0)
        {
            *index_out = i;
            return true;
        }
    }

    return false;
}

void rxcore_mesh_registry_destroy(rxcore_mesh_registry_t *reg)
{
    rxcore_mesh_buffer_destroy(reg->buffer);
    gs_dyn_array_free(reg->meshes);
    gs_dyn_array_free(reg->mesh_names);
    free(reg);
}
