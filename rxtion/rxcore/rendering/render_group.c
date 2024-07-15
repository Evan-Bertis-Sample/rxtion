// render_group.c

#include <rxcore/rendering/render_group.h>

rxcore_render_group_t *_rxcore_render_group_create_empty()
{
    rxcore_render_group_t *group = malloc(sizeof(rxcore_render_group_t));
    group->items = gs_dyn_array_new(rxcore_render_item_t);
    return group;
}

void _rxcore_render_group_traversal(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data)
{
    gs_hash_table(rxcore_material_t *, gs_dyn_array(rxcore_draw_item_t)) *materials_to_draw_items = (gs_hash_table(rxcore_material_t *, gs_dyn_array(rxcore_draw_item_t)) *)user_data;

    if (node->material == NULL || rxcore_mesh_is_empty(&node->mesh))
    {
        return;
    }

    rxcore_draw_item_t draw_item = {
        .model_matrix = model_matrix,
        .node = node,
    };

    if (gs_hash_table_exists(*materials_to_draw_items, node->material))
    {
        gs_println("Material already exists in hash table, adding draw item");
        gs_dyn_array(rxcore_draw_item_t) *draw_items = gs_hash_table_get(*materials_to_draw_items, node->material);
        gs_dyn_array_push(*draw_items, draw_item);
    }
    else
    {
        gs_println("Material does not exist in hash table, creating new draw item array");
        gs_dyn_array(rxcore_draw_item_t) draw_items = gs_dyn_array_new(rxcore_draw_item_t);
        gs_dyn_array_push(draw_items, draw_item);
        gs_hash_table_insert(*materials_to_draw_items, node->material, draw_items);
    }
}

bool rxcore_material_compare(const void *a, const void *b)
{
    rxcore_material_t *ma = *(rxcore_material_t **)a;
    rxcore_material_t *mb = *(rxcore_material_t **)b;

    // sort by shader set
    // this doesn't really matter, just that all the materials with the same shader set are together
    return (
        (size_t)ma->shader_set.fragment_shader + (size_t)ma->shader_set.vertex_shader) >
        ((size_t)mb->shader_set.fragment_shader + (size_t)mb->shader_set.vertex_shader
    );
}

rxcore_render_group_t *rxcore_render_group_create(rxcore_scene_graph_t *graph)
{
    rxcore_render_group_t *group = _rxcore_render_group_create_empty();
    gs_hash_table(rxcore_material_t *, gs_dyn_array(rxcore_draw_item_t)) materials_to_draw_items = gs_hash_table_new(rxcore_material_t *, gs_dyn_array(rxcore_draw_item_t));
    rxcore_scene_graph_traverse(graph, _rxcore_render_group_traversal, &materials_to_draw_items);

    // get all the materials, then sort them by shader set
    // that way we can minimize the number of shader swaps
    // and draw all the meshes that use the same material

    gs_dyn_array(rxcore_material_t *) materials = gs_dyn_array_new(rxcore_material_t *);

    // iterate through the hash table
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(materials_to_draw_items);
        gs_hash_table_iter_valid(materials_to_draw_items, it);
        gs_hash_table_iter_advance(materials_to_draw_items, it))
    {
        rxcore_material_t *material = gs_hash_table_iter_getk(materials_to_draw_items, it);
        gs_println("Material: %p", material);
        gs_dyn_array(rxcore_draw_item_t) *draw_items = gs_hash_table_iter_get(materials_to_draw_items, it);
        gs_dyn_array_push(materials, material);
    }

    // sort the materials by shader set
    size_t material_count = gs_dyn_array_size(materials);
    qsort(materials, material_count, sizeof(rxcore_material_t *), rxcore_material_compare);

    // now we can create the render items
    for (size_t i = 0; i < material_count; i++)
    {
        rxcore_material_t *material = materials[i];
        gs_dyn_array(rxcore_draw_item_t) *draw_items = gs_hash_table_get(materials_to_draw_items, material);

        rxcore_render_item_t render_item = {
            .type = RXCORE_SWAP_ITEM,
            .swap_item = {
                .material = *material,
            },
        };

        gs_dyn_array_push(group->items, render_item);

        for (size_t j = 0; j < gs_dyn_array_size(*draw_items); j++)
        {
            rxcore_draw_item_t draw_item = (*draw_items)[j];
            render_item = (rxcore_render_item_t){
                .type = RXCORE_DRAW_ITEM,
                .draw_item = draw_item,
            };

            gs_dyn_array_push(group->items, render_item);
        }
    }

    // now free the hash table and all of their draw items
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(materials_to_draw_items);
        gs_hash_table_iter_valid(materials_to_draw_items, it);
        gs_hash_table_iter_advance(materials_to_draw_items, it))
    {
        gs_dyn_array(rxcore_draw_item_t) *draw_items = gs_hash_table_iter_get(materials_to_draw_items, it);
        gs_dyn_array_free(*draw_items);
    }

    // now print out the render group
    gs_println("Render group:");
    for (size_t i = 0; i < gs_dyn_array_size(group->items); i++)
    {
        rxcore_render_item_t item = group->items[i];
        if (item.type == RXCORE_SWAP_ITEM)
        {
            rxcore_material_print(&item.swap_item.material);
        }
        else
        {
            rxcore_draw_item_t draw_item = item.draw_item;
            rxcore_scene_node_t *node = draw_item.node;
            rxcore_material_t *material = node->material;
            rxcore_mesh_t mesh = node->mesh;
            rxcore_material_print(material);
            rxcore_mesh_print(&mesh, printf, true);
        }
    }

    gs_hash_table_free(materials_to_draw_items);

    return group;
}

void rxcore_render_group_destroy(rxcore_render_group_t *group)
{
    gs_dyn_array_free(group->items);
    free(group);
}