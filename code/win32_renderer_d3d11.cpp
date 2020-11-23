/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "win32_renderer_d3d11.h"

inline void cat(char *src0, char *src1, char *dest)
{
    while (*src0)  *(dest++) = *(src0++);
    while (*src1)  *(dest++) = *(src1++);
    *dest = '\0';
}

inline void d3d11_resize_render_targets()
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;
    if (d11->swap_chain && d11->context) {
        d11->context->OMSetRenderTargets(0, 0, 0);
        if (d11->render_target_rgb)
            d11->render_target_rgb->Release();

        d11->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        ID3D11Resource  *backbuffer = 0;

        d11->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);
        d11->device->CreateRenderTargetView(backbuffer, 0, &d11->render_target_rgb);

        backbuffer->Release();

        // ===========================================================
        // Viewport set-up
        // ===========================================================
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width  = (r32)global_width;
        viewport.Height = (r32)global_height;
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;
        d11->context->RSSetViewports(1, &viewport);
    }
}

internal void win32_load_d3d11(Platform_Renderer *renderer)
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;

    d3d11_resize_render_targets();
}

inline void d3d11_set_default_render_targets()
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;
    d11->context->OMSetRenderTargets(1, &d11->render_target_rgb, 0);
}

inline void d3d11_clear(v3 color)
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;
    r32 rgba[] = {color.x, color.y, color.z, 1.f};
    d11->context->ClearRenderTargetView(d11->render_target_rgb, rgba);
}
