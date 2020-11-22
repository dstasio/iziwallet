/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "win32_renderer_d3d11.h"

inline void
cat(char *src0, char *src1, char *dest)
{
    while (*src0)  *(dest++) = *(src0++);
    while (*src1)  *(dest++) = *(src1++);
    *dest = '\0';
}

inline void
d3d11_resize_render_targets()
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

PLATFORM_LOAD_RENDERER(win32_load_d3d11)
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;

    d3d11_resize_render_targets();

//    ID3D11SamplerState *sampler;
//    D3D11_SAMPLER_DESC sampler_desc = {};
//    sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
//    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
//    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
//    sampler_desc.MipLODBias = -1;
//    sampler_desc.MaxAnisotropy = 16;
//    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//    sampler_desc.MinLOD = 0;
//    sampler_desc.MaxLOD = 100;
//    d11->device->CreateSamplerState(&sampler_desc, &sampler);
//    d11->context->PSSetSamplers(0, 1, &sampler);

    //
    // rasterizer set-up
    //
//    D3D11_RASTERIZER_DESC raster_settings = {};
//    raster_settings.FillMode = D3D11_FILL_SOLID;
//    raster_settings.CullMode = D3D11_CULL_BACK;
//    raster_settings.FrontCounterClockwise = 1;
//    raster_settings.DepthBias = 0;
//    raster_settings.DepthBiasClamp = 0;
//    raster_settings.SlopeScaledDepthBias = 0;
//    raster_settings.DepthClipEnable = 1;
//    raster_settings.ScissorEnable = 0;
//    raster_settings.MultisampleEnable = 0;
//    raster_settings.AntialiasedLineEnable = 0;

//    ID3D11RasterizerState *raster_state = 0;
//    d11->device->CreateRasterizerState(&raster_settings, &raster_state);
//    d11->context->RSSetState(raster_state);

    //
    // Alpha blending.
    //
//    CD3D11_BLEND_DESC blend_state_desc = {};
//    blend_state_desc.RenderTarget[0].BlendEnable = 1;
//    //blend_state_desc.RenderTarget[0].LogicOpEnable = 0;
//    blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//    blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
//    blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//    blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//    blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
//    blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
//    //blend_state_desc.RenderTarget[0].LogicOp;
//    blend_state_desc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

//    ID3D11BlendState *blend_state = 0;
//    d11->device->CreateBlendState(&blend_state_desc, &blend_state);
//    d11->context->OMSetBlendState(blend_state, 0, 0xFFFFFFFF);
}

inline PLATFORM_SET_RENDER_TARGETS(d3d11_set_default_render_targets)
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;
    d11->context->OMSetRenderTargets(1, &d11->render_target_rgb, 0);
}

inline PLATFORM_CLEAR(d3d11_clear)
{
    D11_Renderer *d11 = (D11_Renderer *)global_renderer->platform;
    r32 rgba[] = {color.x, color.y, color.z, 1.f};
    d11->context->ClearRenderTargetView(d11->render_target_rgb, rgba);
}
