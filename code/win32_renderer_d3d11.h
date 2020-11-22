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

    ID3D11RenderTargetView  *render_target_rgb;
};

#define WIN32_RENDERER_D3D11_H
#endif
