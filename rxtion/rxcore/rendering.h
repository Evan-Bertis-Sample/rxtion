#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <rxcore/system.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering/scene_graph.h>

#define CORE_ASSET(ASSET_NAME) "rxtion/rxcore/" ASSET_NAME
#define APP_ASSET(ASSET_NAME) "rxtion/rxapp/assets/" ASSET_NAME

// default dependency names
#define RXCORE_SHADER_VERT_UTIL_HANDLE "rxcore_shader_vert_util"
#define RXCORE_SHADER_FRAG_LIT_HANDLE "rxcore_shader_frag_lit"
#define RXCORE_SHADER_FRAG_UNLIT_HANDLE "rxcore_shader_frag_unlit"

// default shader names
#define RXCORE_SHADER_VERTEX_DEFAULT "rxcore_shader_vertex_default"
#define RXCORE_SHADER_FRAGMENT_UNLIT_DEFAULT "rxcore_shader_fragment_unlit"
#define RXCORE_SHADER_FRAGMENT_LIT_DEFAULT "rxcore_shader_fragment_lit"

// used to pass into to get default shader sets
#define RXCORE_SHADER_SET_UNLIT_DEFAULT RXCORE_SHADER_VERTEX_DEFAULT, RXCORE_SHADER_FRAGMENT_UNLIT_DEFAULT
#define RXCORE_SHADER_SET_LIT_DEFAULT RXCORE_SHADER_VERTEX_DEFAULT, RXCORE_SHADER_FRAGMENT_LIT_DEFAULT

typedef struct rxcore_rendering_context_t
{
    rxcore_shader_registry_t *shader_registry;
    rxcore_material_registry_t *material_registry;
    rxcore_mesh_registry_t *mesh_registry;
    rxcore_scene_graph_t *scene_graph;
    // probably will add more stuff here later...
} rxcore_rendering_context_t;

static rxcore_rendering_context_t g_rendering_context;

void rxcore_rendering_init();
void rxcore_rendering_update();
void rxcore_rendering_shutdown();

rxcore_rendering_context_t rxcore_rendering_context_create();
void rxcore_rendering_context_destroy(rxcore_rendering_context_t *context);

// private methods for rendering
static void _rxcore_rendering_load_core_shader_dependencies(rxcore_shader_registry_t *reg);
static void _rxcore_rendering_load_core_shaders(rxcore_shader_registry_t *reg);

#define rxcore_rendering_system RXCORE_SYSTEM(rxcore_rendering_init, rxcore_rendering_update, rxcore_rendering_shutdown)

#endif // __RENDERING_H__