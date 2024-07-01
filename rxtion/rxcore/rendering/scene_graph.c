// scene_graph.c

#include <rxcore/rendering/scene_graph.h>
#include <rxcore/transform.h>

rxcore_scene_node_t *rxcore_scene_node_create(rxcore_transform_t transform, rxcore_mesh_t mesh, rxcore_material_t *material)
{
    rxcore_scene_node_t *node = malloc(sizeof(rxcore_scene_node_t));
    node->transform = transform;
    node->mesh = mesh;
    node->material = material;
    node->children = gs_dyn_array_new(rxcore_scene_node_t);
    node->parent = NULL;
    node->graph = NULL;
    return node;
}

void rxcore_scene_node_add_child(rxcore_scene_node_t *node, rxcore_scene_node_t *child)
{
    gs_dyn_array_push(node->children, child);
    child->parent = node;
    child->graph = node->graph;

    // update the graph's node count
    if (child->graph)
    {
        child->graph->node_count++;
    }
}

rxcore_scene_graph_t *rxcore_scene_graph_create()
{
    rxcore_scene_graph_t *graph = malloc(sizeof(rxcore_scene_graph_t));
    rxcore_transform_t t = transform_create(
        gs_v3(0.0f, 0.0f, 0.0f),
        gs_v3(1.0f, 1.0f, 1.0f),
        gs_quat_default());

    graph->root = rxcore_scene_node_create(
        t,
        rxcore_mesh_empty(),
        NULL
    );
    return graph;
}

void rxcore_scene_graph_add_child(rxcore_scene_graph_t *graph, rxcore_scene_node_t *node)
{
    rxcore_scene_node_add_child(graph->root, node);
}

void rxcore_scene_graph_traverse(rxcore_scene_graph_t *graph, rxcore_scene_graph_traveral_fn fn)
{
    if (graph->is_dirty)
    {
        _rxcore_scene_graph_regen_stacks(graph);
    }

    gs_mat4 model_matrix = gs_mat4_identity();
    gs_mat4 *matrix_stack = graph->matrix_stack;
    uint32_t model_stack_ptr = 0;

    rxcore_scene_node_t **node_stack = graph->node_stack;
    uint32_t node_stack_ptr = 0;

    // traverse the graph in a depth first manner
    node_stack[node_stack_ptr++] = graph->root;
    matrix_stack[model_stack_ptr++] = model_matrix;

    while (node_stack_ptr > 0)
    {
        rxcore_scene_node_t *node = node_stack[--node_stack_ptr];
        model_matrix = matrix_stack[--model_stack_ptr];

        // call the traversal function
        fn(node, model_matrix);

        // push children onto the stack
        for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
        {
            rxcore_scene_node_t *child = node->children[i];
            gs_mat4 child_model_matrix = gs_mat4_mul(model_matrix, rxcore_transform_to_mat4(child->transform));
            node_stack[node_stack_ptr++] = child;
            matrix_stack[model_stack_ptr++] = child_model_matrix;
        }
    }
}

void rxcore_scene_graph_destroy(rxcore_scene_graph_t *graph)
{
    rxcore_scene_graph_traverse(graph, _rxcore_scene_graph_free_node);
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
    graph->matrix_stack = malloc(sizeof(gs_mat4) * graph->node_count);
    graph->node_stack = malloc(sizeof(rxcore_scene_node_t *) * graph->node_count);
    graph->stack_size = new_size;
    graph->is_dirty = false;
}

void _rxcore_scene_graph_free_node(rxcore_scene_node_t *node, void *user_data)
{
    free(node);
}

