// material.c

#include <rx_core/rendering/material.h>
#include <gs/gs.h>

rxcore_material_prototype_t rxcore_material_prototype_create(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms)
{
    rxcore_material_prototype_t proto = {0};
    proto.shader_set = set;
    proto.uniform_descs = uniform_descs;
    proto.num_uniforms = num_uniforms;
    return proto;
}

rxcore_material_t *rxcore_material_create_base(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms)
{
    RXCORE_SHADER_DEBUG_PRINTF("Creating material with %d uniforms", num_uniforms);

    rxcore_material_t *material = malloc(sizeof(rxcore_material_t));
    material->shader_set = set;
    material->num_uniforms = num_uniforms;

    // create the uniform handles
    material->uniform_handles = malloc(sizeof(gs_handle(gs_graphics_uniform_t)) * num_uniforms);
    for (uint32_t i = 0; i < num_uniforms; i++)
    {
        gs_handle(gs_graphics_uniform_t) uniform = gs_graphics_uniform_create(&uniform_descs[i]);
        material->uniform_handles[i] = uniform;
    }

    // create the uniform bindings
    material->uniform_bindings = malloc(sizeof(gs_graphics_bind_uniform_desc_t) * num_uniforms);
    for (uint32_t i = 0; i < num_uniforms; i++)
    {
        gs_graphics_bind_uniform_desc_t binding = {
            .uniform = material->uniform_handles[i],
            .data = NULL,
        };
        material->uniform_bindings[i] = binding;
    }

    // create the uniform name to index map
    material->uniform_name_to_index = malloc(num_uniforms * sizeof(const char *));
    for (uint32_t i = 0; i < num_uniforms; i++)
    {
        const char *name_cpy = malloc(strlen(uniform_descs[i].name) + 1);
        strcpy(name_cpy, uniform_descs[i].name);
        RXCORE_MATERIAL_DEBUG_PRINTF("Adding uniform %s to index %d", name_cpy, i);
        material->uniform_name_to_index[i] = name_cpy;
    }

    return material;
}

rxcore_material_t *rxcore_material_create_from_prototype(const rxcore_material_prototype_t *prototype, gs_graphics_uniform_desc_t *override_uniform_descs, uint32_t num_overrides)
{
    // copy all the uniforms from the prototype, but override any that are in the override list
    // determine how large the new uniform list will be
    // at a minimum, it will be the size of the prototype uniforms
    // but we may need more if there are overrides that aren't in the prototype
    uint32_t num_uniforms = prototype->num_uniforms;
    for (uint32_t i = 0; i < num_overrides; i++)
    {
        gs_graphics_uniform_desc_t override = override_uniform_descs[i];
        bool found = false;
        for (uint32_t j = 0; j < prototype->num_uniforms; j++)
        {
            gs_graphics_uniform_desc_t proto = prototype->uniform_descs[j];
            if (strcmp(override.name, proto.name) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            num_uniforms++;
        }
    }

    // allocate the new uniform list
    gs_graphics_uniform_desc_t *uniforms = malloc(sizeof(gs_graphics_uniform_desc_t) * num_uniforms);

    // copy the prototype uniforms into the new list
    for (uint32_t i = 0; i < prototype->num_uniforms; i++)
    {
        uniforms[i] = prototype->uniform_descs[i];
    }

    // copy the overrides into the new list
    uint32_t uniform_index = prototype->num_uniforms;
    for (uint32_t i = 0; i < num_overrides; i++)
    {
        gs_graphics_uniform_desc_t override = override_uniform_descs[i];
        bool found = false;
        for (uint32_t j = 0; j < prototype->num_uniforms; j++)
        {
            gs_graphics_uniform_desc_t proto = prototype->uniform_descs[j];
            if (strcmp(override.name, proto.name) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            uniforms[uniform_index] = override;
            uniform_index++;
        }
    }

    // create the material
    rxcore_material_t *material = rxcore_material_create_base(prototype->shader_set, uniforms, num_uniforms);
    return material;
}

void rxcore_material_add_binding(rxcore_material_t *material, const char *uniform_name, void *data, uint32_t size)
{
    if (material->uniform_name_to_index == NULL)
    {
        RXCORE_MATERIAL_DEBUG_PRINT("Material has no uniforms, cannot add binding.");
        return;
    }

    uint32_t index = 0;
    bool exists = rxcore_material_get_uniform_index(material, uniform_name, &index); 
    if (!exists)
    {
        RXCORE_MATERIAL_DEBUG_PRINTF("Uniform not found: %s, cannot add binding.", uniform_name);
        return;   
    }

    RXCORE_MATERIAL_DEBUG_PRINTF("Binding %s to index %d", uniform_name, index);
    material->uniform_bindings[index].data = data;
    material->uniform_bindings[index].binding = size;
}

void rxcore_material_bind(rxcore_material_t *material, gs_command_buffer_t *cb)
{
    gs_graphics_apply_bindings(cb, material->uniform_bindings);
}

void rxcore_material_print(rxcore_material_t *material)
{
    RXCORE_MATERIAL_DEBUG_PRINT("Material:");
    RXCORE_MATERIAL_DEBUG_PRINTF("  Vertex Shader: %s", material->shader_set.vertex_shader->shader_name);
    RXCORE_MATERIAL_DEBUG_PRINTF("  Fragment Shader: %s", material->shader_set.fragment_shader->shader_name);
    RXCORE_MATERIAL_DEBUG_PRINT("  Uniforms:");
    for (uint32_t i = 0; i < material->num_uniforms; i++)
    {
        RXCORE_MATERIAL_DEBUG_PRINTF("    %s", material->uniform_name_to_index[i]);
    }

    RXCORE_MATERIAL_DEBUG_PRINT("End Material");
}

bool rxcore_material_uniform_exists(rxcore_material_t *material, const char *uniform_name)
{
    for (uint32_t i = 0; i < material->num_uniforms; i++)
    {
        if (strcmp(material->uniform_name_to_index[i], uniform_name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool rxcore_material_get_uniform_index(rxcore_material_t *material, const char *uniform_name, uint32_t *out_index)
{
    for (uint32_t i = 0; i < material->num_uniforms; i++)
    {
        if (strcmp(material->uniform_name_to_index[i], uniform_name) == 0)
        {
            *out_index = i;
            return true;
        }
    }
    return false;
}

void rxcore_material_destroy(rxcore_material_t *material)
{
    for (uint32_t i = 0; i < material->num_uniforms; i++)
    {
        gs_graphics_uniform_destroy(material->uniform_handles[i]);
    }

    free(material->uniform_handles);
    free(material->uniform_bindings);

    for (uint32_t i = 0; i < material->num_uniforms; i++)
    {
        free(material->uniform_name_to_index[i]);
    }

    free(material->uniform_name_to_index);
    free(material);
}
