// render_group.c

#include <rxcore/rendering/render_group.h>

void _rxcore_material_group_destroy(rxcore_material_group_t *group)
{
    gs_dyn_array_free(group->draw_items);
    free(group);
}

rxcore_render_group_t *_rxcore_render_group_create_empty()
{
    rxcore_render_group_t *group = malloc(sizeof(rxcore_render_group_t));
    group->items = gs_dyn_array_new(rxcore_render_item_t);
    return group;
}

void _rxcore_render_group_traversal(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data)
{
    gs_dyn_array(rxcore_material_group_t) *material_groups = (gs_dyn_array(rxcore_material_group_t) *)user_data;
                 
    if (node->material == NULL || rxcore_mesh_is_empty(&node->mesh))
    {
        return;
    }


    // check if we have a material group for this material
    rxcore_material_group_t *material_group = NULL;
    for (uint32_t i = 0; i < gs_dyn_array_size(*material_groups); i++)
    {
        if ((*material_groups)[i].material == node->material)
        {
            // gs_println("Found material group for material %p", node->material);
            material_group = &(*material_groups)[i];
            break;
        }
    }

    if (material_group == NULL)
    {
        gs_println("Creating new material group for material %p", node->material);
        rxcore_material_group_t new_group = {0};
        new_group.material = node->material;
        new_group.draw_items = gs_dyn_array_new(rxcore_draw_item_t);
        gs_dyn_array_push(*material_groups, new_group);
        material_group = &(*material_groups)[gs_dyn_array_size(*material_groups) - 1];
    }

    // gs_println("Adding draw item to material group");
    // gs_println("Material group: %p", material_group);

    rxcore_draw_item_t draw_item = {0};
    draw_item.model_matrix = model_matrix;
    draw_item.node = node;
    gs_dyn_array_push(material_group->draw_items, draw_item);
}

bool rxcore_material_group_compare(const void *a, const void *b)
{
    rxcore_material_t *ma = ((rxcore_material_group_t *)a)->material;
    rxcore_material_t *mb = ((rxcore_material_group_t *)b)->material;

    // sort by shader set
    // this doesn't really matter, just that all the materials with the same shader set are together
    return (
        (size_t)ma->shader_set.fragment_shader + (size_t)ma->shader_set.vertex_shader) >
        ((size_t)mb->shader_set.fragment_shader + (size_t)mb->shader_set.vertex_shader
    );
}

rxcore_render_group_t *rxcore_render_group_create(rxcore_scene_graph_t *graph)
{
    gs_dyn_array(rxcore_material_group_t) material_groups = gs_dyn_array_new(rxcore_material_group_t);
    rxcore_scene_graph_traverse(graph, _rxcore_render_group_traversal, &material_groups);

    // now we need to sort the material groups by material
    // so that we can swap materials efficiently
    uint32_t num_groups = gs_dyn_array_size(material_groups);
    gs_println("Number of material groups: %d", num_groups);
    // qsort(material_groups, num_groups, sizeof(rxcore_material_group_t), rxcore_material_group_compare);

    rxcore_render_group_t *res = _rxcore_render_group_create_empty();

    for (uint32_t i = 0; i < num_groups; i++)
    {
        gs_println("Adding material group %d", i);
        gs_println("Num draw items: %d", gs_dyn_array_size(material_groups[i].draw_items));
        rxcore_material_group_t group = material_groups[i];
        rxcore_render_item_t item = {0};
        item.type = RXCORE_SWAP_ITEM;
        item.swap_item.material = group.material;
        gs_dyn_array_push(res->items, item);

        for (uint32_t j = 0; j < gs_dyn_array_size(group.draw_items); j++)
        {
            rxcore_draw_item_t draw_item = group.draw_items[j];
            rxcore_render_item_t item = {0};
            item.type = RXCORE_DRAW_ITEM;
            item.draw_item = draw_item;
            gs_dyn_array_push(res->items, item);
        }
    
    }

    gs_println("Number of items in render group: %d", gs_dyn_array_size(res->items));
    gs_println("Number of material groups: %d", gs_dyn_array_size(material_groups));

    rxcore_render_group_print(res, printf);

    for (uint32_t i = 0; i < gs_dyn_array_size(material_groups); i++)
    {
        _rxcore_material_group_destroy(&material_groups[i]);
    }
    gs_dyn_array_free(material_groups);

    gs_println("Render group created");

    return res;
}

void rxcore_render_group_print(rxcore_render_group_t *group, void (*print_fn)(const char *str, ...))
{
    print_fn("Render Group:\n");
    for (uint32_t i = 0; i < gs_dyn_array_size(group->items); i++)
    {
        rxcore_render_item_t item = group->items[i];
        switch (item.type)
        {
        case RXCORE_DRAW_ITEM:
            print_fn("  Draw Item\n");
            print_fn("    Model Matrix: %f %f %f %f\n", item.draw_item.model_matrix.elements[0], item.draw_item.model_matrix.elements[1], item.draw_item.model_matrix.elements[2], item.draw_item.model_matrix.elements[3]);
            print_fn("    Node: %p\n", item.draw_item.node);
            break;
        case RXCORE_SWAP_ITEM:
            print_fn("  Swap Item\n");
            print_fn("    Material: %p\n", item.swap_item.material);
            break;
        }
    }
}

void rxcore_render_group_destroy(rxcore_render_group_t *group)
{
    gs_dyn_array_free(group->items);
    free(group);
}