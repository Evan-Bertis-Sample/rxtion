#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <stdbool.h>
#include <gs/gs.h>
#include <rxcore/transform.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/mesh.h>

#define MAX_SCENE_DEPTH 16

// forward declaration
typedef struct rxcore_scene_node_t rxcore_scene_node_t;
typedef struct rxcore_scene_graph_t rxcore_scene_graph_t;
typedef void (*rxcore_scene_graph_traveral_fn)(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data);

typedef struct rxcore_scene_node_t
{
    rxcore_transform_t transform;
    rxcore_mesh_t mesh;          // it is okay to have the mesh as a value here, because the mesh type is just a fat pointer really
    rxcore_material_t *material; // it is not okay to have a material as a value here, because the material is pretty big
    gs_dyn_array(rxcore_scene_node_t *) children;
    rxcore_scene_node_t *parent;
    rxcore_scene_graph_t *graph;
} rxcore_scene_node_t;

typedef struct rxcore_scene_graph_t
{
    rxcore_scene_node_t *root;
    uint32_t node_count;
    // caches for the stacks using in the traversal
    // these are used during the traversal to avoid having to allocate memory on the stack
    // as traversals of the scene graph are likely to be frequent
    // and the stack size is not known at compile time
    // so we can't use a fixed size array
    // this is meant for internal use only -- this contains garbage data
    gs_mat4 *matrix_stack;
    rxcore_scene_node_t **node_stack;
    uint32_t *depth_stack;
    uint32_t stack_size;
    bool is_dirty;
} rxcore_scene_graph_t;

rxcore_scene_node_t *rxcore_scene_node_create(rxcore_transform_t transform, rxcore_mesh_t mesh, rxcore_material_t *material);
rxcore_scene_node_t *rxcore_scene_node_copy(rxcore_scene_node_t *node, bool deep_copy);
void rxcore_scene_node_add_child(rxcore_scene_node_t *node, rxcore_scene_node_t *child);
void rxcore_scene_node_remove_child(rxcore_scene_node_t *node, rxcore_scene_node_t *child);
void rxcore_scene_node_destroy(rxcore_scene_node_t *node);

rxcore_scene_graph_t *rxcore_scene_graph_create();
rxcore_scene_graph_t *rxcore_scene_graph_create_from_node(rxcore_scene_node_t *node);
void rxcore_scene_graph_add_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t *node);
void rxcore_scene_graph_remove_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t *node);
void rxcore_scene_graph_traverse(rxcore_scene_graph_t *graph, rxcore_scene_graph_traveral_fn fn, void *user_data);
void rxcore_scene_graph_print(rxcore_scene_graph_t *graph, void (*print_fn)(const char *str, ...));
void rxcore_scene_graph_destroy(rxcore_scene_graph_t *graph);

void _rxcore_scene_graph_regen_stacks(rxcore_scene_graph_t *graph);
void _rxcore_scene_graph_free_node(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth,void *user_data);
void _rxcore_scene_graph_print_node(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data);

#endif // __SCENE_GRAPH_H__