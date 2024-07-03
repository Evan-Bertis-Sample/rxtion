#ifndef __SCENE_H__
#define __SCENE_H__

#include <gs/gs.h>
#include <rxcore/profiler.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering/scene_graph.h>
#include <rxcore/rendering/shader.h>

typedef struct rxcore_scene_t
{
    // will probably add more stuff here later...
    rxcore_scene_graph_t *graph;
    rxcore_mesh_registry_t *mesh_registry;
} rxcore_scene_t;

rxcore_scene_t rxcore_scene_create();
void rxcore_scene_destroy(rxcore_scene_t *scene);

#endif // __SCENE_H__