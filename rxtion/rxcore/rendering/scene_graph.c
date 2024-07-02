// scene_graph.c

#include <rxcore/rendering/scene_graph.h>
#include <rxcore/transform.h>

rxcore_scene_node_t *rxcore_scene_node_create(rxcore_transform_t transform, rxcore_mesh_t mesh, rxcore_material_t *material)
{
    rxcore_scene_node_t *node = malloc(sizeof(rxcore_scene_node_t));
    node->transform = transform;
    node->mesh = mesh;
    node->material = material;
    node->children = gs_dyn_array_new(rxcore_scene_node_t *);
    node->parent = NULL;
    node->graph = NULL;
    return node;
}

void rxcore_scene_node_add_child(rxcore_scene_node_t *node, rxcore_scene_node_t *child)
{
    assert(node != NULL);
    assert(child != NULL);

    gs_dyn_array_push(node->children, child);
    child->parent = node;
    child->graph = node->graph;

    // update the graph's node count
    if (child->graph)
    {
        child->graph->node_count++;
    }
}

rxcore_scene_node_t *rxcore_scene_node_copy(rxcore_scene_node_t *node, bool deep_copy)
{
    assert(node != NULL);

    rxcore_scene_node_t *copy = rxcore_scene_node_create(
        node->transform,
        node->mesh,
        node->material);

    copy->parent = NULL;
    copy->graph = NULL;

    if (deep_copy)
    {
        for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
        {
            rxcore_scene_node_t *child = node->children[i];
            rxcore_scene_node_t *child_copy = rxcore_scene_node_copy(child, deep_copy);
            rxcore_scene_node_add_child(copy, child_copy);
        }
    }
    return copy;
}

void rxcore_scene_node_remove_child(rxcore_scene_node_t *node, rxcore_scene_node_t *child)
{
    assert(node != NULL);
    assert(child != NULL);

    for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
    {
        if (node->children[i] == child)
        {
            // turn this into a scene_graph real quick lol
            rxcore_scene_graph_t *graph = rxcore_scene_graph_create_from_node(child);
            // free this graph to destroy all the children
            rxcore_scene_graph_destroy(graph);
            node->children[i] = NULL;
            // update the graph's node count
            if (node->graph)
            {
                node->graph->node_count--;
            }

            // create a new array, and copy all the children except the one we are removing
            gs_dyn_array(rxcore_scene_node_t *) new_children = gs_dyn_array_new(rxcore_scene_node_t *);
            for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
            {
                if (node->children[i] != NULL)
                {
                    gs_dyn_array_push(new_children, node->children[i]);
                }
            }

            gs_dyn_array_free(node->children);
            node->children = new_children;

            return;
        }
    }
}

void rxcore_scene_node_destroy(rxcore_scene_node_t *node)
{
    assert(node != NULL);

    for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
    {
        rxcore_scene_node_destroy(node->children[i]);
    }

    gs_dyn_array_free(node->children);
    free(node);
}

rxcore_scene_graph_t *rxcore_scene_graph_create()
{
    rxcore_scene_graph_t *graph = malloc(sizeof(rxcore_scene_graph_t));
    graph->node_count = 1;
    graph->is_dirty = false;

    // malloc the stacks
    uint32_t stack_size = 16;
    graph->matrix_stack = malloc(sizeof(gs_mat4) * stack_size);
    graph->node_stack = malloc(sizeof(rxcore_scene_node_t *) * stack_size);
    graph->depth_stack = malloc(sizeof(uint32_t) * stack_size);
    graph->stack_size = stack_size;

    // create the root node
    rxcore_transform_t t = rxcore_transform_create(
        gs_v3(0.0f, 0.0f, 0.0f),
        gs_v3(1.0f, 1.0f, 1.0f),
        gs_quat_default());

    graph->root = rxcore_scene_node_create(
        t,
        rxcore_mesh_empty(),
        NULL);

    graph->root->graph = graph;
    return graph;
}

rxcore_scene_graph_t *rxcore_scene_graph_create_from_node(rxcore_scene_node_t *node)
{
    rxcore_scene_graph_t *graph = rxcore_scene_graph_create();
    rxcore_scene_node_destroy(graph->root);
    graph->root = node;
    return graph;
}

void rxcore_scene_graph_add_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t *node)
{
    assert(node != NULL);
    assert(node->parent == NULL);

    rxcore_scene_node_add_child(graph->root, node);
}

void rxcore_scene_graph_remove_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t *node)
{
    assert(node != NULL);
    assert(node->parent != NULL);

    rxcore_scene_node_remove_child(graph->root, node);
}

void rxcore_scene_graph_traverse(rxcore_scene_graph_t *graph, rxcore_scene_graph_traveral_fn fn, void *user_data)
{
    assert(fn != NULL);

    if (graph->is_dirty)
    {
        _rxcore_scene_graph_regen_stacks(graph);
    }

    gs_mat4 model_matrix = gs_mat4_identity();
    gs_mat4 *matrix_stack = graph->matrix_stack;
    uint32_t model_stack_ptr = 0;

    rxcore_scene_node_t *node = NULL;
    rxcore_scene_node_t **node_stack = graph->node_stack;
    uint32_t node_stack_ptr = 0;

    uint32_t depth = 0;
    uint32_t *depth_stack = graph->depth_stack;
    uint32_t depth_stack_ptr = 0;

    // traverse the graph in a depth first manner
    node_stack[node_stack_ptr++] = graph->root;
    matrix_stack[model_stack_ptr++] = model_matrix;
    depth_stack[depth_stack_ptr++] = depth;

    while (node_stack_ptr > 0)
    {
        node = node_stack[--node_stack_ptr];
        model_matrix = matrix_stack[--model_stack_ptr];
        depth = depth_stack[--depth_stack_ptr];
        // call the traversal function
        fn(node, model_matrix, depth, user_data);

        // push children onto the stack
        for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
        {
            rxcore_scene_node_t *child = node->children[i];
            gs_mat4 child_model_matrix = gs_mat4_mul(model_matrix, rxcore_transform_to_mat4(&(child->transform)));
            node_stack[node_stack_ptr++] = child;
            matrix_stack[model_stack_ptr++] = child_model_matrix;
            depth_stack[depth_stack_ptr++] = depth + 1;
        }
    }
}

void rxcore_scene_graph_print(rxcore_scene_graph_t *graph, void (*print_fn)(const char *str, ...))
{
    rxcore_scene_graph_traverse(graph, _rxcore_scene_graph_print_node, print_fn);
}

void rxcore_scene_graph_destroy(rxcore_scene_graph_t *graph)
{
    rxcore_scene_graph_traverse(graph, _rxcore_scene_graph_free_node, NULL);
    free(graph->matrix_stack);
    free(graph->node_stack);
    free(graph);
}

void _rxcore_scene_graph_regen_stacks(rxcore_scene_graph_t *graph)
{
    // can we support the new nodes?
    if (graph->node_count <= graph->stack_size)
    {
        graph->is_dirty = false;
        return;
    }

    // we don't care about the old data
    free(graph->matrix_stack);
    free(graph->node_stack);
    // we will either double the stack size or use the node count, whichever is larger
    // the reason being that we want to avoid reallocating the stack too often
    uint32_t new_size = graph->node_count < graph->stack_size * 2 ? graph->stack_size * 2 : graph->node_count;
    graph->matrix_stack = malloc(sizeof(gs_mat4) * new_size);
    graph->node_stack = malloc(sizeof(rxcore_scene_node_t *) * new_size);
    graph->depth_stack = malloc(sizeof(uint32_t) * new_size);
    graph->stack_size = new_size;
    graph->is_dirty = false;
}

void _rxcore_scene_graph_free_node(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data)
{
    rxcore_scene_node_destroy(node);
}

void _rxcore_scene_graph_print_node(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data)
{
    void (*print_fn)(const char *str, ...) = user_data;
    for (int i = 0; i < depth; i++)
    {
        print_fn("  ");
    }
    print_fn("Node: %p, depth: %d, mesh: %d-%d, parent: %p\n", node, depth, node->mesh.starting_index, node->mesh.index_count + node->mesh.starting_index, node->parent);
}
