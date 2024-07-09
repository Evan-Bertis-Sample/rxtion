// shader.c

#include <gs/gs.h>
#include <rxcore/rendering/shader.h>
#include <gs/util/gs_asset.h>
#include <sds/sds.h>

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
        RXCORE_SHADER_DEBUG_PRINTF("Shader path does not exist: %s", desc.shader_path);
        return NULL;
    }

    rxcore_shader_t *shader = malloc(sizeof(rxcore_shader_t));
    const char *shader_name_buf = malloc(strlen(desc.shader_name) + 1);
    strcpy(shader_name_buf, desc.shader_name);
    shader->shader_name = shader_name_buf;
    shader->stage = desc.stage;

    RXCORE_SHADER_DEBUG_PRINTF("Creating shader: %s", shader->shader_name);
    RXCORE_SHADER_DEBUG_PRINTF("Shader path: %s", desc.shader_path);
    // get the shader source from the file
    size_t this_src_len = 0;
    // the buffer is created on the heap, so it must be freed later!
    char *this_shader_src = gs_platform_read_file_contents(desc.shader_path, "r", &this_src_len);
    // RXCORE_SHADER_DEBUG_PRINTF("Shader source length: %d", this_src_len);
    RXCORE_SHADER_DEBUG_PRINTF("Shader source: %s", this_shader_src);

    if (desc.shader_dependency_count > 0)
    {
        size_t full_shader_len = 0;
        for (int i = 0; i < desc.shader_dependency_count; i++)
        {
            const char *dep_name = desc.shader_dependencies[i];
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, dep_name);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Dependency not found: %s", dep_name);
                continue;
            }

            full_shader_len += strlen(dep->shader_src);
        }

        char *full_shader_src = malloc(full_shader_len + this_src_len + 1 + desc.shader_dependency_count);
        
        for (int i = 0; i < desc.shader_dependency_count; i++)
        {
            const char *dep_name = desc.shader_dependencies[i];
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, dep_name);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Dependency not found: %s", dep_name);
                continue;
            }

            strcat(full_shader_src, dep->shader_src);
            strcat(full_shader_src, "\n");
        }

        strcat(full_shader_src, this_shader_src);
        // free this_shader_src, as it is no longer needed
        full_shader_src[full_shader_len + this_src_len] = '\0'; // idk why this is needed, but it is
        shader->shader_src = full_shader_src;
    }
    else
    {
        shader->shader_src = this_shader_src;
    }

    shader->shader_src = _rxcore_shader_resolve_includes(reg, shader->shader_src);

    RXCORE_SHADER_DEBUG_PRINTF("Shader source:%s\n%s", shader->shader_name, shader->shader_src);

    return shader;
}

void _rxcore_shader_destroy(rxcore_shader_t *shader)
{
    free(shader->shader_name);
    free(shader->shader_src);
}

char *_rxcore_shader_resolve_includes(rxcore_shader_registry_t *reg, const char *src)
{
    // find all of the #includes, and replace them with the source of the included shader
    sds s = sdsnew(src);
    uint32_t line_count = 0;
    sds *lines = sdssplitlen(s, sdslen(s), "\n", 1, &line_count);
    sds resolved_src = sdsempty();

    for (uint32_t i = 0; i < line_count; i++)
    {
        sds line = lines[i];
        // RXCORE_SHADER_DEBUG_PRINTF("Line: %s", line);
        // skip all the whitespace
        while (isspace(*line))
        {
            line++;
        }

        if (strstr(line, "#include") != NULL)
        {
            RXCORE_SHADER_DEBUG_PRINTF("Found include directive: %s", line);
            // skip the #include, and find the "
            char *include_start = strstr(line, "\"");
            if (include_start == NULL)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Invalid include directive: %s", line);
                continue;
            }

            include_start++;
            char *include_end = strstr(include_start, "\"");
            if (include_end == NULL)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Invalid include directive: %s", line);
                continue;
            }

            uint32_t include_len = include_end - include_start;
            char *include = malloc(include_len + 1);
            strncpy(include, include_start, include_len);
            include[include_len] = '\0';

            // find the include in the registry
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, include);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Dependency not found: %s", include);
                // replace this line with an empty string
                resolved_src = sdscat(resolved_src, "\n");
                continue;
            }

            // append the source of the dependency to the resolved source
            resolved_src = sdscat(resolved_src, dep->shader_src);
            resolved_src = sdscat(resolved_src, "\n");
        }
        else
        {
            resolved_src = sdscat(resolved_src, line);
            resolved_src = sdscat(resolved_src, "\n");
        }
    }

    sdsfreesplitres(lines, line_count);
    sdsfree(s);

    return resolved_src;
}

rxcore_shader_registry_t *rxcore_shader_registry_create()
{
    rxcore_shader_registry_t *reg = malloc(sizeof(rxcore_shader_registry_t));
    reg->shaders = gs_dyn_array_new(rxcore_shader_t *);
    reg->dependencies = gs_dyn_array_new(rxcore_shader_t *);
    return reg;
}


void rxcore_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char *dep_name, const char *dep_path)
{
    RXCORE_SHADER_DEBUG_PRINTF("Adding dependency %s", dep_name);
    rxcore_shader_desc_t desc = rxcore_shader_desc_create(dep_name, dep_path, RXCORE_SHADER_STAGE_DEPENDENCY, NULL, 0);
    rxcore_shader_t *dep = _rxcore_shader_create(reg, desc);
    if (!dep)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Failed to create dependency: %s", dep_name);
        return;
    }
    gs_dyn_array_push(reg->dependencies, dep);
}

rxcore_shader_t *rxcore_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc)
{
    RXCORE_SHADER_DEBUG_PRINTF("Adding shader %s", desc.shader_name);
    rxcore_shader_t *shader = _rxcore_shader_create(reg, desc);
    if (!shader)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Failed to create shader: %s", desc.shader_name);
        return NULL;
    }

    gs_dyn_array_push(reg->shaders, shader);
    return shader;
}

rxcore_shader_set_t rxcore_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name)
{
    rxcore_shader_set_t set = {0};
    // find the shaders
    set.vertex_shader = _rxcore_shader_registry_find_shader(reg, vertex_shader_name);
    if (!set.vertex_shader)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Warning! Vertex Shader %s not found! Failing to create full set.", vertex_shader_name);
    }

    // check that this is a vertex shader
    if (set.vertex_shader->stage != RXCORE_SHADER_STAGE_VERTEX)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Warning! Shader %s is not a vertex shader! Undefined behavior expected!", vertex_shader_name);
    }

    set.fragment_shader = _rxcore_shader_registry_find_shader(reg, fragment_shader_name);
    if (!set.fragment_shader)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Warning! Vertex Shader %s not found! Failing to create full set.", fragment_shader_name);
    }

    // check that this is a fragment shader
    if (set.fragment_shader->stage != RXCORE_SHADER_STAGE_FRAGMENT)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Warning! Shader %s is not a fragment shader! Undefined behavior expected!", fragment_shader_name);
    }

    return set;
}

void rxcore_shader_registry_write_compiled_shaders_to_file(rxcore_shader_registry_t *reg, const char *output_dir)
{
    if (!gs_platform_dir_exists(output_dir))
    {
        gs_platform_mkdir(output_dir, 0);
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = reg->shaders[i];
        char *output_path = malloc(strlen(output_dir) + strlen(shader->shader_name) + 5);
        strcpy(output_path, output_dir);
        strcat(output_path, "/");
        strcat(output_path, shader->shader_name);
        strcat(output_path, ".glsl");

        FILE *file = fopen(output_path, "w");
        if (!file)
        {
            RXCORE_SHADER_DEBUG_PRINTF("Failed to open file for writing: %s", output_path);
            continue;
        }

        fwrite(shader->shader_src, 1, strlen(shader->shader_src), file);
        fclose(file);
        free(output_path);
    }
}

void rxcore_shader_registry_destroy(rxcore_shader_registry_t *reg)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = reg->shaders[i];
        _rxcore_shader_destroy(shader);
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = reg->dependencies[i];
        _rxcore_shader_destroy(dep);
    }

    gs_dyn_array_free(reg->shaders);
    gs_dyn_array_free(reg->dependencies);
    free(reg);
}

rxcore_shader_t *_rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = reg->dependencies[i];
        if (strcmp(dep->shader_name, shader_name) == 0)
        {
            return dep;
        }
    }

    RXCORE_SHADER_DEBUG_PRINTF("Dependency not found: %s", shader_name);
    return NULL;
}

rxcore_shader_t *_rxcore_shader_registry_find_shader(rxcore_shader_registry_t *reg, const char *shader_name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = reg->shaders[i];
        if (strcmp(shader->shader_name, shader_name) == 0)
        {
            return shader;
        }
    }

    RXCORE_SHADER_DEBUG_PRINTF("Shader not found: %s", shader_name);
    return NULL;
}

rxcore_shader_program_t *rxcore_shader_program_set(rxcore_shader_set_t set)
{
    if (set.vertex_shader == NULL || set.vertex_shader->stage != RXCORE_SHADER_STAGE_VERTEX)
    {
        RXCORE_SHADER_DEBUG_PRINT("Invalid vertex shader in set!");
        return NULL;
    }

    if (set.fragment_shader == NULL || set.fragment_shader->stage != RXCORE_SHADER_STAGE_FRAGMENT)
    {
        RXCORE_SHADER_DEBUG_PRINT("Invalid fragment shader in set!");
        return 0;
    }

    size_t vert_src_size = strlen(set.vertex_shader->shader_name);
    size_t frag_src_size = strlen(set.fragment_shader->shader_name);

    gs_graphics_shader_source_desc_t vert_src_desc = {
        .source = set.vertex_shader->shader_src,
        .type = GS_GRAPHICS_SHADER_STAGE_VERTEX};

    gs_graphics_shader_source_desc_t frag_shader_src = {
        .source = set.fragment_shader->shader_src,
        .type = GS_GRAPHICS_SHADER_STAGE_VERTEX};

    gs_graphics_shader_source_desc_t sources[] = {vert_src_desc, frag_shader_src};

    char *program_name = malloc(64); // 64 cause that is the max expected size of the program name
    program_name[0] = '\0';
    // combine the names of the shaders
    strncat(program_name, set.fragment_shader->shader_name,
            gs_clamp(strlen(set.fragment_shader->shader_name), 1, 31));
    strcat(program_name, "_");
    strncat(program_name, set.vertex_shader->shader_name,
            gs_clamp(strlen(set.vertex_shader->shader_name), 1, 31));

    gs_graphics_shader_desc_t shader_desc =
        {
            .name = set.vertex_shader->shader_name,
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .sources = sources,
        };

    gs_handle(gs_graphics_shader_t) shader = gs_graphics_shader_create(&shader_desc);

    rxcore_shader_program_t *program = malloc(sizeof(rxcore_shader_program_t));
    program->program = shader;
    program->program_name = program_name;

    return program;
}

void rxcore_shader_program_destroy(rxcore_shader_program_t *program)
{
    gs_graphics_shader_destroy(program->program);
    free(program->program_name);
    free(program);
}
