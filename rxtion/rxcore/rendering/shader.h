#ifndef __SHADER_H__
#define __SHADER_H__

#include <gs/gs.h>
#include <rxcore/profiler.h>

#define SRC_MAX_LENGTH 1024
#define RXCORE_SHADER_DEBUG

#ifdef RXCORE_SHADER_DEBUG
#define RXCORE_SHADER_DEBUG_PRINT(str) gs_println("RXCORE::rendering::shader::" str)
#define RXCORE_SHADER_DEBUG_PRINTF(str, ...) gs_println("RXCORE::rendering::shader::" str, __VA_ARGS__)
#else
#define RXCORE_SHADER_DEBUG_PRINT(...) ((void)0)
#define RXCORE_SHADER_DEBUG_PRINTF(...) ((void)0)
#endif

/// @brief The stage of a shader in the rendering pipeline
typedef enum rxcore_shader_stage_t
{
    RXCORE_SHADER_STAGE_VERTEX = GS_GRAPHICS_SHADER_STAGE_VERTEX,
    RXCORE_SHADER_STAGE_FRAGMENT = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
    RXCORE_SHADER_STAGE_COMPUTE = GS_GRAPHICS_SHADER_STAGE_COMPUTE,
    RXCORE_SHADER_STAGE_DEPENDENCY = 4,
    RXCORE_SHADER_STAGE_COUNT 
} rxcore_shader_stage_t;

/// @brief A description of a shader, which can be used to create a shader
typedef struct rxcore_shader_desc_t
{
    rxcore_shader_stage_t stage;
    const char **shader_dependencies;
    uint16_t shader_dependency_count;
    const char *shader_name;
    const char *shader_path;
} rxcore_shader_desc_t;

/// @brief A shader, which can be used to create a shader program, should be used with a shader registry
typedef struct rxcore_shader_t
{
    rxcore_shader_stage_t stage;
    const char *shader_name;
    const char *shader_src;
} rxcore_shader_t;

/// @brief A registry of shaders, which can be used to create shader sets
typedef struct rxcore_shader_registry_t
{
    gs_dyn_array(rxcore_shader_t *) shaders;
    gs_dyn_array(rxcore_shader_t *) dependencies;

} rxcore_shader_registry_t;

/// @brief A set of shaders, which can be used to create a shader program
typedef struct rxcore_shader_set_t
{
    rxcore_shader_t *vertex_shader;
    rxcore_shader_t *fragment_shader;
} rxcore_shader_set_t;

/// @brief A shader program, which can be used to render objects
typedef struct rxcore_shader_program_t
{
    const char *program_name;
    gs_handle(gs_graphics_shader_t) program;
} rxcore_shader_program_t;

/// @brief Creates a shader description, allocated on the stack, which can be used to create a shader.
/// @param shader_name The name of the shader
/// @param shader_path The path to the shader file
/// @param stage The stage of the shader in the rendering pipeline
/// @param shader_dependencies // A list of the name of the dependencies of the shader, will be prepended to the shader source 
/// @param shader_dependency_count // The number of dependencies in the list
/// @return 
rxcore_shader_desc_t rxcore_shader_desc_create(const char *shader_name, const char *shader_path, rxcore_shader_stage_t stage, const char **shader_dependencies, uint16_t shader_dependency_count);
#define RXCORE_SHADER_DEPENDENCIES(...) (const char *[]){__VA_ARGS__}, sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *)
#define RXCORE_SHADER_DESC_WITH_INCLUDE(shader_name, shader_path, stage, ...) rxcore_shader_desc_create(shader_name, shader_path, stage, RXCORE_SHADER_DEPENDENCIES(__VA_ARGS__))
#define RXCORE_SHADER_DESC(shader_name, shader_path, stage) rxcore_shader_desc_create(shader_name, shader_path, stage, NULL, 0)

// RXCORE_SHADER private methods

/// @brief Creates a shader from a shader description -- this is a private method
/// @param reg The shader registry used to grab dependencies
/// @param desc The shader description
/// @return A pointer to the created shader, allocated on the heap
rxcore_shader_t *_rxcore_shader_create(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);
void _rxcore_shader_destroy(rxcore_shader_t *shader);

// RXCORE_SHADER_REGISTRY public methods

/// @brief Creates a shader registry, allocated on the heap
/// @return A pointer to the created shader registry
rxcore_shader_registry_t *rxcore_shader_registry_create();

/// @brief Adds a dependency to the shader registry, which can be used to create shaders
/// @param reg The shader registry to add the dependency to
/// @param dep_name The name of the dependency
/// @param dep_path The path to the dependency file
void rxcore_shader_registry_add_dependency(rxcore_shader_registry_t *reg, const char *dep_name, const char *dep_path);

/// @brief Adds a shader to the shader registry, which can be used to create shader sets
/// @param reg The shader registry to add the shader to
/// @param desc The shader description
/// @return A pointer to the created shader, allocated on the heap, or NULL if the shader could not be created. Does not need to be freed, as it is managed by the registry
rxcore_shader_t *rxcore_shader_registry_add_shader(rxcore_shader_registry_t *reg, rxcore_shader_desc_t desc);

/// @brief Gets a shader set from the shader registry, which can be used to create shader programs
/// @param reg A pointer to the shader registry to get the shader set from
/// @param vertex_shader_name The name of the vertex shader
/// @param fragment_shader_name The name of the fragment shader
/// @return The shader set, which contains the vertex and fragment shaders
rxcore_shader_set_t rxcore_shader_registry_get_shader_set(rxcore_shader_registry_t *reg, const char *vertex_shader_name, const char *fragment_shader_name);

/// @brief Frees all memory associated with the shader registry, will break shader sets, and shader ptrs created from the registry
/// @param reg The shader registry to destroy
void rxcore_shader_registry_destroy(rxcore_shader_registry_t *reg);

// RXCORE_SHADER_REGISTRY private methods

/// @brief Finds a dependency in the shader registry
/// @param reg The shader registry to search
/// @param shader_name The name of the dependency to find
/// @return A pointer to the dependency, or NULL if the dependency could not be found
rxcore_shader_t *_rxcore_shader_registry_find_dependency(rxcore_shader_registry_t *reg, const char *shader_name);

/// @brief Finds a shader in the shader registry
/// @param reg The shader registry to search
/// @param shader_name The name of the shader to find
/// @return A pointer to the shader, or NULL if the shader could not be found
rxcore_shader_t *_rxcore_shader_registry_find_shader(rxcore_shader_registry_t *reg, const char *shader_name);

// RXCORE_SHADER_SET public methods

/// @brief Creates a shader program from a shader set
/// @param set The shader set to create the program from
/// @return A pointer to the created shader program, allocated on the heap, or NULL if the program could not be created. Ownership is transferred to the caller
rxcore_shader_program_t *rxcore_shader_program_set(rxcore_shader_set_t set);

/// @brief Frees all memory associated with the shader program
/// @param program The shader program to destroy
void rxcore_shader_program_destroy(rxcore_shader_program_t *program);


#endif // __SHADER_H__