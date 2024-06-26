// shader.c
#include <rx_core/rendering/shader.h>
#include <gs/util/gs_asset.h>

// SHADER DESC

rxcore_shader_desc_t _rxcore_shader_desc_create(const char *shader_name, const char *shader_path, const char **shader_dependencies, uint16_t shader_dependency_count)
{
    rxcore_shader_desc_t desc = {0};
    desc.shader_name = shader_name;
    desc.shader_path = shader_path;
    desc.shader_dependencies = shader_dependencies;
    desc.shader_dependency_count = shader_dependency_count;
    return desc;
}

// SHADER

rxcore_shader_t *_rxcore_shader_create(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc)
{
    // check that the shader path exists
    if (!gs_platform_file_exists(desc.shader_path))
    {
        gs_println("Shader path does not exist: %s", desc.shader_path);
        return NULL;
    }

    rxcore_shader_t shader = {0};
    const char* shader_name_buf = malloc(strlen(desc.shader_name) + 1);
    strcpy(shader_name_buf, desc.shader_name);
    shader.shader_name = shader_name_buf;

    // get the shader source from the file
    const char *this_shader_src = gs_read_file_contents_into_string_null_term(desc.shader_path, 'r', SRC_MAX_LENGTH);

    if (desc.shader_dependency_count > 0)
    {
        // get all the depdencies of the shader, and concatenate them into a single string
        const char *shader_src = malloc(SRC_MAX_LENGTH * desc.shader_dependency_count + 1);

        int offset;
        for (uint32_t i = 0; i < desc.shader_dependency_count; i++)
        {
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, desc.shader_dependencies[i]);
            strcpy(shader_src + offset, dep->shader_src);
            offset += strlen(dep->shader_src);
        }

        // now append the source of the shader itself
        strcpy(shader_src + offset, this_shader_src);
        int full_length = strlen(shader_src) + 1;
        shader_src = realloc(shader_src, full_length);

        shader.shader_src = shader_src;
    }
    else
    {
        // malloc the size of the shader source, and copy the source into the shader
        const char *shader_src = malloc(strlen(this_shader_src) + 1);
        strcpy(shader_src, this_shader_src);
        shader.shader_src = shader_src;
    }
}

void _rxcore_shader_destroy(rxcore_shader_t *shader)
{
    free(shader->shader_name);
    free(shader->shader_src);
}

// SHADER REGISTRY

rxcore_shader_registry_t *rx_shader_registry_create()
{
    rxcore_shader_registry_t *reg = malloc(sizeof(rxcore_shader_registry_t));
    return reg;
}

void rx_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char *dep_name, const char *dep_path)
{
    rxcore_shader_desc_t desc = _rxcore_shader_desc_create(dep_name, dep_path, NULL, 0);
    rxcore_shader_t *dep = _rxcore_shader_create(reg, desc);
    gs_dyn_array_push(reg->dependencies, *dep);
}

uint32_t rx_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc)
{
    rxcore_shader_t *shader = _rxcore_shader_create(reg, desc);
    gs_dyn_array_push(reg->shaders, *shader);
    uint32_t id = gs_array_size(reg->shaders) - 1;
    return id;
}

rxcore_shader_set_t rx_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name)
{
    rxcore_shader_set_t set = {0};

    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = &reg->shaders[i];
        if (strcmp(shader->shader_name, vertex_shader_name) == 0)
        {
            set.vertex_shader_id = i;
        }
        else if (strcmp(shader->shader_name, fragment_shader_name) == 0)
        {
            set.fragment_shader_id = i;
        }
    }

    return set;
}

void rx_shader_registry_destroy(rxcore_shader_registry_t *reg)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        _rxcore_shader_destroy(&reg->shaders[i]);
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        _rxcore_shader_destroy(&reg->dependencies[i]);
    }

    gs_dyn_array_free(reg->shaders);
    gs_dyn_array_free(reg->dependencies);
    free(reg);
}

uint16_t _rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = &reg->dependencies[i];
        if (strcmp(dep->shader_name, shader_name) == 0)
        {
            return i;
        }
    }

    return -1;
}

rxcore_shader_t *_rxcore_shader_registry_find_shader(rxcore_shader_registry_t *reg, const char *shader_name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = &reg->shaders[i];
        if (strcmp(shader->shader_name, shader_name) == 0)
        {
            return shader;
        }
    }

    return NULL;
}