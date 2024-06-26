// shader.c
#include <rx_core/rendering/shader.h>
#include <gs/util/gs_asset.h>

// SHADER DESC

rxcore_shader_desc_t rxcore_shader_desc_create(const char *shader_name, const char *shader_path, rxcore_shader_stage_t stage, const char **shader_dependencies, uint16_t shader_dependency_count)
{
    rxcore_shader_desc_t desc = {0};
    desc.stage = stage;
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
        RXCORE_SHADER_DEBUG_PRINT("Shader path does not exist: %s", desc.shader_path);
        return NULL;
    }

    rxcore_shader_t *shader = malloc(sizeof(rxcore_shader_t));
    const char* shader_name_buf = malloc(strlen(desc.shader_name) + 1);
    strcpy(shader_name_buf, desc.shader_name);
    shader->shader_name = shader_name_buf;

    shader->stage = desc.stage;

    RXCORE_SHADER_DEBUG_PRINT("Creating shader: %s", shader->shader_name);
    RXCORE_SHADER_DEBUG_PRINT("Shader path: %s", desc.shader_path);
    // get the shader source from the file
    size_t this_src_len = 0;
    // the buffer is created on the heap, so it must be freed later!
    char *this_shader_src = gs_platform_read_file_contents(desc.shader_path, "r", &this_src_len);
    RXCORE_SHADER_DEBUG_PRINT("Shader source length: %d", this_src_len);
    RXCORE_SHADER_DEBUG_PRINT("Shader source: %s", this_shader_src);

    if (desc.shader_dependency_count > 0)
    {
        size_t full_shader_len = 0;
        for (int i = 0; i < desc.shader_dependency_count; i++)
        {
            const char *dep_name = desc.shader_dependencies[i];
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, dep_name);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINT("Dependency not found: %s", dep_name);
                continue;
            }

            full_shader_len += strlen(dep->shader_src);
        }

        RXCORE_SHADER_DEBUG_PRINT("Full shader length: %d", full_shader_len);

        char *full_shader_src = malloc(full_shader_len + this_src_len + 1 + desc.shader_dependency_count);
        full_shader_src[0] = '\0';
        for (int i = 0; i < desc.shader_dependency_count; i++)
        {
            const char *dep_name = desc.shader_dependencies[i];
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, dep_name);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINT("Dependency not found: %s", dep_name);
                continue;
            }

            strcat(full_shader_src, dep->shader_src);
            strcat(full_shader_src, "\n");
        }

        strcat(full_shader_src, this_shader_src);

        RXCORE_SHADER_DEBUG_PRINT("Full shader source: %s", full_shader_src);
        // free this_shader_src, as it is no longer needed
        shader->shader_src = full_shader_src;
    }
    else
    {
        RXCORE_SHADER_DEBUG_PRINT("No dependencies for shader: %s", shader->shader_name);
        shader->shader_src = this_shader_src;
    }

    return shader;
}

void _rxcore_shader_destroy(rxcore_shader_t *shader)
{
    free(shader->shader_name);
    free(shader->shader_src);
}

rxcore_shader_registry_t *rxcore_shader_registry_create()
{
    rxcore_shader_registry_t *reg = malloc(sizeof(rxcore_shader_registry_t));
    reg->shaders = gs_hash_table_new(const char *, rxcore_shader_t*);
    reg->dependencies = gs_hash_table_new(const char *, rxcore_shader_t*);
    return reg;
}

void rxcore_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char *dep_name, const char *dep_path)
{
    rxcore_shader_desc_t desc = rxcore_shader_desc_create(dep_name, dep_path, RXCORE_SHADER_STAGE_DEPENDENCY, NULL, 0);
    rxcore_shader_t *dep = _rxcore_shader_create(reg, desc);
    if (!dep)
    {
        RXCORE_SHADER_DEBUG_PRINT("Failed to create dependency: %s", dep_name);
        return;
    }
    gs_hash_table_insert(reg->dependencies, dep_name, dep);
}

rxcore_shader_t *rxcore_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc)
{
    rxcore_shader_t *shader = _rxcore_shader_create(reg, desc);
    if (!shader)
    {
        RXCORE_SHADER_DEBUG_PRINT("Failed to create shader: %s", desc.shader_name);
        return NULL;
    }
    
    gs_hash_table_insert(reg->shaders, desc.shader_name, shader);
    return shader;
}

rxcore_shader_set_t rxcore_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name)
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

void rxcore_shader_registry_destroy(rxcore_shader_registry_t *reg)
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

rxcore_shader_t *_rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name)
{
    rxcore_shader_t *dep = gs_hash_table_get(reg->dependencies, shader_name);
    if (!dep)
    {
        RXCORE_SHADER_DEBUG_PRINT("Dependency not found: %s", shader_name);
        return NULL;
    }

    return dep;
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

    RXCORE_SHADER_DEBUG_PRINT("Shader not found: %s", shader_name);
    return NULL;
}