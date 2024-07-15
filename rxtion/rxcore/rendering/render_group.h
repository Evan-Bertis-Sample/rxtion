#ifndef __RENDER_GROUP_H__
#define __RENDER_GROUP_H__

#include <gs/gs.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/scene_graph.h>


typedef struct rxcore_draw_item_t
{
    gs_mat4 model_matrix;
    rxcore_scene_node_t *node;
} rxcore_draw_item_t;

typedef struct rxcore_swap_item_t
{
    rxcore_material_t *material;
} rxcore_swap_item_t;

typedef struct rxcore_material_group_t 
{
    rxcore_material_t *material;
    gs_dyn_array(rxcore_draw_item_t) draw_items;
} rxcore_material_group_t;

enum rxcore_render_item_type_t
{
    RXCORE_DRAW_ITEM,
    RXCORE_SWAP_ITEM,
};

typedef struct rxcore_render_item_t
{
    enum rxcore_render_item_type_t type;
    union {
        rxcore_draw_item_t draw_item;
        rxcore_swap_item_t swap_item;
    };
} rxcore_render_item_t;

typedef struct rxcore_render_group_t
{
    gs_dyn_array(rxcore_render_item_t) items;
} rxcore_render_group_t;


rxcore_render_group_t *_rxcore_render_group_create_empty();
void _rxcore_render_group_traversal(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data);
rxcore_render_group_t *rxcore_render_group_create(rxcore_scene_graph_t *graph);
void rxcore_render_group_destroy(rxcore_render_group_t *group);


#endif // __RENDER_GROUP_H__