#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <stdbool.h>
#include <gs/gs.h>
#include <rx_core/transform.h>
#include <rx_core/rendering/material.h>
#include <rx_core/rendering/shader.h>
#include <rx_core/rendering/mesh.h>

typedef void (*rxcore_scene_graph_traveral_fn)(rxcore_scene_node_t *node, gs_mat4 model_matrix);

typedef struct rxcore_scene_node_t
{
    rxcore_transform_t transform;
    rxcore_mesh_t mesh; // it is okay to have the mesh as a value here, because the mesh type is just a fat pointer really
    rxcore_material_t *material; // it is not okay to have a material as a value here, because the material is pretty big
    gs_dyn_array(rxcore_scene_node_t) children;
    rxcore_scene_node_t *parent;
    rxcore_scene_graph_t *graph;
} rxcore_scene_node_t;

typedef struct rxcore_scene_graph_t
{
    rxcore_scene_node_t *root;
} rxcore_scene_graph_t;

// typedef struct rxcore_scene_node_flattened_t
// {
//     // double ptrs are used here to avoid copying the data
//     // also for the flexibility of being able to change the data from the scene_node, and not rebuild the flattened tree
//     // all of these ptrs should be in a row in memory, so that the cache can be utilized
//     rxcore_transform_t *transform;
//     rxcore_mesh_t *mesh;
//     rxcore_material_t **material;
// } rxcore_scene_node_flattened_t;

// typedef struct rxcore_scene_graph_flattened_t
// {
//     // the reason we flatten the scene graph is to optimize cache performance
//     // although there still is some indirection here (namely the double ptrs in the scene_node_flattened_t struct)
//     // the scene graph is still much more cache friendly than the original scene graph, as we don't have to jump around in memory as much while rendering
//     rxcore_scene_node_flattened_t *nodes;
//     uint32_t node_count;
// } rxcore_scene_graph_flattened_t;

rxcore_scene_node_t rxcore_scene_node_create(rxcore_transform_t transform, rxcore_mesh_t mesh, rxcore_material_t *material);
void rxcore_scene_node_add_child(rxcore_scene_node_t *node, rxcore_scene_node_t child);

// flatten the scene graph into a single array of nodes, to optimize cache performance when rendering
void rxcore_scene_graph_add_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t node);
void rxcore_scene_graph_traverse(rxcore_scene_graph_t *graph, rxcore_scene_graph_traveral_fn fn);
void rxcore_scene_graph_destroy(rxcore_scene_graph_t *graph);
// rxcore_scene_graph_flattened_t rxcore_scene_graph_flatten(rxcore_scene_graph_t *graph);



#endif // __SCENE_GRAPH_H__