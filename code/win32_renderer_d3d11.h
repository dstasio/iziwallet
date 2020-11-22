#if !defined(WIN32_RENDERER_D3D11_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */

struct D11_Renderer
{
    ID3D11Device        *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain      *swap_chain;

    ID3D11DepthStencilState *depth_nostencil_state;
    ID3D11DepthStencilState *nodepth_nostencil_state;
    ID3D11RenderTargetView  *render_target_rgb;
    ID3D11DepthStencilView  *render_target_depth;

    ID3D11Buffer *matrix_buff;
    ID3D11Buffer *light_buff;
    ID3D11Buffer *settings_buff;
    ID3D11Buffer *debug_buff;
};

#define WIN32_RENDERER_D3D11_H
#endif
