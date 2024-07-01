#ifndef __MESH_PRIMATIVES_H__
#define __MESH_PRIMATIVES_H__

#include <rxcore/rendering/mesh.h>

#define RXCORE_MESH_OUT_ARGS rxcore_vertex_t **vertex_out, uint32_t *vertex_count_out, uint32_t **indices_out, uint32_t *indices_count_out

void rxcore_mesh_primatives_quad(RXCORE_MESH_OUT_ARGS)
{
    *vertex_count_out = 4;
    *indices_count_out = 6;

    *vertex_out = (rxcore_vertex_t *)malloc(sizeof(rxcore_vertex_t) * (*vertex_count_out));
    *indices_out = (uint32_t *)malloc(sizeof(uint32_t) * (*indices_count_out));

    (*vertex_out)[0] = (rxcore_vertex_t){.position = gs_v3(0.5f, 0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(1.0f, 1.0f)};
    (*vertex_out)[1] = (rxcore_vertex_t){.position = gs_v3(-0.5f, 0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(0.0f, 1.0f)};
    (*vertex_out)[2] = (rxcore_vertex_t){.position = gs_v3(-0.5f, -0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(0.0f, 0.0f)};
    (*vertex_out)[3] = (rxcore_vertex_t){.position = gs_v3(0.5f, -0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(1.0f, 0.0f)};

    (*indices_out)[0] = 0;
    (*indices_out)[1] = 1;
    (*indices_out)[2] = 2;
    (*indices_out)[3] = 2;
    (*indices_out)[4] = 3;
    (*indices_out)[5] = 0;
}

void rxcore_mesh_primatives_triangle(RXCORE_MESH_OUT_ARGS)
{
    *vertex_count_out = 3;
    *indices_count_out = 3;

    *vertex_out = (rxcore_vertex_t *)malloc(sizeof(rxcore_vertex_t) * (*vertex_count_out));
    *indices_out = (uint32_t *)malloc(sizeof(uint32_t) * (*indices_count_out));

    (*vertex_out)[0] = (rxcore_vertex_t){.position = gs_v3(0.0f, 0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(0.5f, 1.0f)};
    (*vertex_out)[1] = (rxcore_vertex_t){.position = gs_v3(-0.5f, -0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(0.0f, 0.0f)};
    (*vertex_out)[2] = (rxcore_vertex_t){.position = gs_v3(0.5f, -0.5f, 0.0f), .normal = gs_v3(0.0f, 0.0f, 1.0f), .uv = gs_v2(1.0f, 0.0f)};

    (*indices_out)[0] = 0;
    (*indices_out)[1] = 1;
    (*indices_out)[2] = 2;

}

#endif // __MESH_PRIMATIVES_H__