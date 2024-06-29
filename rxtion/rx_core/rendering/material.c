// material.c

#include <rx_core/rendering/material.h>
#include <gs/gs.h>

rxcore_material_prototype_t rxcore_material_prototype_create_impl(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms)
{
    rxcore_material_prototype_t proto = {0};
    proto.shader_set = set;
    proto.uniform_descs = uniform_descs;
    proto.num_uniforms = num_uniforms;
    return proto;
}

rxcore_material_uniforms_t rxcore_material_uniforms_create(rxcore_shader_stage_t stage, gs_handle(gs_graphics_uniform_t) *uniform_handles, gs_graphics_bind_uniform_desc_t *uniform_bindings, uint32_t num_uniforms)
{
    rxcore_material_uniforms_t uniforms = {0};
    uniforms.stage = stage;
    uniforms.uniform_handles = uniform_handles;
    uniforms.uniform_bindings = uniform_bindings;
    uniforms.num_uniforms = num_uniforms;
    return uniforms;
}

rxcore_material_t *rxcore_material_create(const rxcore_material_prototype_t *prototype, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms)
{
    rxcore_material_t *material = malloc(sizeof(rxcore_material_t));
    material->shader_set = prototype->shader_set;
    material->vertex_uniforms = malloc(sizeof(rxcore_material_uniforms_t));
    material->fragment_uniforms = malloc(sizeof(rxcore_material_uniforms_t));

    for (uint32_t i = 0; i < prototype->num_uniforms; i++)
    {
        gs_graphics_uniform_desc_t desc = prototype->uniform_descs[i];
        if (desc.stage == RXCORE_SHADER_STAGE_VERTEX)
        {
            material->vertex_uniforms->num_uniforms++;
        }
        else if (desc.stage == RXCORE_SHADER_STAGE_FRAGMENT)
        {
            material->fragment_uniforms->num_uniforms++;
        }
    }

    material->vertex_uniforms->uniform_handles = malloc(sizeof(gs_handle(gs_graphics_uniform_t)) * material->vertex_uniforms->num_uniforms);
    material->vertex_uniforms->uniform_bindings = malloc(sizeof(gs_graphics_bind_uniform_desc_t) * material->vertex_uniforms->num_uniforms);

    material->fragment_uniforms->uniform_handles = malloc(sizeof(gs_handle(gs_graphics_uniform_t)) * material->fragment_uniforms->num_uniforms);
    material->fragment_uniforms->uniform_bindings = malloc(sizeof(gs_graphics_bind_uniform_desc_t) * material->fragment_uniforms->num_uniforms);

    for (uint32_t i = 0; i < prototype->num_uniforms; i++)
    {
        gs_graphics_uniform_desc_t desc = prototype->uniform_descs[i];
        if (desc.stage == RXCORE_SHADER_STAGE_VERTEX)
        {
            material->vertex_uniforms->uniform_handles[material->vertex_uniforms->num_uniforms] = gs_graphics_uniform_create(desc.layout);
            material->vertex_uniforms->uniform_bindings[material->vertex_uniforms->num_uniforms] = (gs_graphics_bind_uniform_desc_t){
                .uniform = material->vertex_uniforms->uniform_handles[material->vertex_uniforms->num_uniforms],
                .data = NULL,
            };
        }
        else if (desc.stage == RXCORE_SHADER_STAGE_FRAGMENT)
        {
            material->fragment_uniforms->uniform_handles[material->fragment_uniforms->num_uniforms] = gs_graphics_uniform_create(desc.layout);
            material->fragment_uniforms->uniform_bindings[material->fragment_uniforms->num_uniforms] = (gs_graphics_bind_uniform_desc_t){
                .uniform = material->fragment_uniforms->uniform_handles[material->fragment_uniforms->num_uniforms],
                .data = NULL,
            };
        }
    }
}

rxcore_material_t *rxcore_material_create(const rxcore_material_prototype_t *prototype, gs_graphics_uniform_desc_t *override_uniform_descs, uint32_t num_overrides)
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
    rxcore_material_t *material = rxcore_material_create(prototype, uniforms, num_uniforms);
    return material;
}

void rxcore_material_add_binding(rxcore_material_t *material, rxcore_shader_stage_t stage, const char *uniform_name, void *data, uint32_t size)
{
    if (stage != RXCORE_SHADER_STAGE_VERTEX && stage != RXCORE_SHADER_STAGE_FRAGMENT)
    {
        gs_println("Invalid shader stage: %d", stage);
        return;
    }

    rxcore_material_uniforms_t *uniforms = stage == RXCORE_SHADER_STAGE_VERTEX ? material->vertex_uniforms : material->fragment_uniforms;
    uint32_t num_uniforms = stage == RXCORE_SHADER_STAGE_VERTEX ? material->vertex_uniforms->num_uniforms : material->fragment_uniforms->num_uniforms;

    for (uint32_t i = 0; i < num_uniforms; i++)
    {
        gs_graphics_bind_uniform_desc_t *binding = &(uniforms->uniform_bindings[i]);
        char *name = binding->uniform->desc.name;
        if (strcmp(name, uniform_name) == 0)
        {
            binding->data = data;
            binding->size = size;
            return;
        }
    }

    gs_println("Uniform not found: %s, cannot add binding.", uniform_name);
}

void rxcore_material_bind(rxcore_material_t *material, gs_command_buffer_t *cb)
{
    for (uint32_t i = 0; i < material->vertex_uniforms->num_uniforms; i++)
    {
        gs_graphics_bind_uniform_desc_t binding = material->vertex_uniforms->uniform_bindings[i];
        gs_command_bind_uniform(cb, &binding);
    }

    for (uint32_t i = 0; i < material->fragment_uniforms->num_uniforms; i++)
    {
        gs_graphics_bind_uniform_desc_t binding = material->fragment_uniforms->uniform_bindings[i];
        gs_command_bind_uniform(cb, &binding);
    }
}