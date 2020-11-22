/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy.h"

GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    Wallet_State *state = (Wallet_State *)memory->storage;
    if(!memory->is_initialized)
    {
        Memory_Pool mempool = {};
        mempool.base = (u8 *)memory->storage;
        mempool.size = memory->storage_size;
        mempool.used = sizeof(Wallet_State);

        memory->is_initialized = true;
    }

    //
    // ---------------------------------------------------------------
    //

}
