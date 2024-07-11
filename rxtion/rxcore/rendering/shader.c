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
    // RXCORE_SHADER_DEBUG_PRINTF("Shader path: %s", desc.shader_path);

    size_t this_src_len = 0;
    char *this_shader_src = gs_platform_read_file_contents(desc.shader_path, "rb", &this_src_len);
    if (!this_shader_src)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Failed to read shader source: %s", desc.shader_path);
        return NULL;
    }

    // if the description entails that we should prepend some dependencies to the shader, do so
    if (desc.shader_dependency_count > 0)
    {
        RXCORE_SHADER_DEBUG_PRINTF("Shader has dependencies: %s", shader->shader_name);
        // calculate the length of the new shader_src string, to malloc latger
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

        // add the original length to the new length, as well as room to add \n between each dependencies
        full_shader_len += this_src_len + desc.shader_dependency_count;
        char *full_shader_src = malloc(full_shader_len + 1);
        // make the string empty for strcat to work
        full_shader_src[0] = '\0';

        // find each dependency, and append it to the new source
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
        free(this_shader_src);
        shader->shader_src = full_shader_src;
    }
    else
    {
        // there was no dependencies in the description
        shader->shader_src = this_shader_src;
    }

    // now manually expand the #include statements
    shader->shader_src = _rxcore_shader_resolve_includes(reg, shader->shader_src);
    return shader;
}

// Utility method to check if a line is an include directive and extract the filename
int is_include_directive(const char *line, char **filename)
{
    char *include_pos = strstr(line, "#include");
    if (!include_pos)
        return 0;

    include_pos += 8; // move past "#include"
    while (*include_pos == ' ' || *include_pos == '\t')
    {
        include_pos++;
    }

    if (*include_pos != '\"')
    {
        RXCORE_SHADER_DEBUG_PRINT("Error: Malformed include directive\n");
        return -1;
    }

    include_pos++; // move past the opening quote
    char *end_quote = strchr(include_pos, '\"');
    if (!end_quote)
    {
        RXCORE_SHADER_DEBUG_PRINT("Error: Malformed include directive\n");
        return -1;
    }

    size_t filename_len = end_quote - include_pos;
    *filename = (char *)malloc(filename_len + 1);
    strncpy(*filename, include_pos, filename_len);
    (*filename)[filename_len] = '\0';

    return 1;
}

char *_rxcore_shader_resolve_includes(rxcore_shader_registry_t *reg, const char *src)
{
    // tokenize the source by newlines using sds
    sds resolved_src = sdsempty();
    int line_count = 0;
    sds src_copy = sdsnew(src);
    sds *lines = sdssplitlen(src_copy, sdslen(src_copy), "\n", 1, &line_count);

    for (int i = 0; i < line_count; i++)
    {
        char *line = lines[i];
        char *filename = NULL;
        int is_include = is_include_directive(line, &filename);
        if (is_include == 1)
        {
            rxcore_shader_t *dep = _rxcore_shader_registry_find_dependency(reg, filename);
            if (!dep)
            {
                RXCORE_SHADER_DEBUG_PRINTF("Error: Failed to resolve include: %s\n", filename);
                continue;
            }

            resolved_src = sdscat(resolved_src, dep->shader_src);
            resolved_src = sdscat(resolved_src, "\n");
        }
        else if (is_include == -1)
        {
            RXCORE_SHADER_DEBUG_PRINT("Error: Failed to resolve include directive\n");
        }
        else
        {
            resolved_src = sdscat(resolved_src, line);
        }

        free(filename);
    }
    sdsfreesplitres(lines, line_count);
    return resolved_src;
}

void _rxcore_shader_destroy(rxcore_shader_t *shader)
{
    free(shader->shader_name);
    free(shader->shader_src);
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

    const char *dep_output_dir = malloc(strlen(output_dir) + 5);
    strcpy(dep_output_dir, output_dir);
    strcat(dep_output_dir, "/dep");

    if (!gs_platform_dir_exists(dep_output_dir))
    {
        gs_platform_mkdir(dep_output_dir, 0);
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = reg->dependencies[i];
        char *output_path = malloc(strlen(dep_output_dir) + strlen(dep->shader_name) + 5);
        strcpy(output_path, dep_output_dir);
        strcat(output_path, "/");
        strcat(output_path, dep->shader_name);
        strcat(output_path, ".glsl");

        FILE *file = fopen(output_path, "w");
        if (!file)
        {
            RXCORE_SHADER_DEBUG_PRINTF("Failed to open file for writing: %s", output_path);
            continue;
        }

        fwrite(dep->shader_src, 1, strlen(dep->shader_src), file);
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

bool rxcore_shader_set_equals(rxcore_shader_set_t a, rxcore_shader_set_t b)
{
    return a.vertex_shader == b.vertex_shader && a.fragment_shader == b.fragment_shader;
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
        return NULL;
    }

    size_t vert_src_size = strlen(set.vertex_shader->shader_name);
    size_t frag_src_size = strlen(set.fragment_shader->shader_name);

    char program_name[64];
    program_name[0] = '\0';
    // combine the names of the shaders
    strncat(program_name, set.fragment_shader->shader_name,
            gs_clamp(strlen(set.fragment_shader->shader_name), 1, 31));
    strcat(program_name, "/");
    strncat(program_name, set.vertex_shader->shader_name,
            gs_clamp(strlen(set.vertex_shader->shader_name), 1, 31));
            
    // gs_println("Creating shader program: %s", program_name);

    gs_graphics_shader_desc_t shader_desc =
    {
        .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
        .sources = (gs_graphics_shader_source_desc_t[]){
            { .source = set.vertex_shader->shader_src, .type = GS_GRAPHICS_SHADER_STAGE_VERTEX },
            { .source = set.fragment_shader->shader_src, .type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT },
        },
    };

    strncpy(shader_desc.name, program_name, 63);
    shader_desc.name[64] = '\0';

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
