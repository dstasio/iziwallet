/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2014 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */

#include <windows.h>
#include <d3d11.h>

#include "windy.h"
#include "windy_platform.h"

#include "win32_layer.h"
#include <stdio.h>

#include "backends/imgui_impl_dx11.cpp"
#include "backends/imgui_impl_win32.cpp"
#include "imgui.cpp"
//#include "imgui.h"
#include "imgui_draw.cpp"
//#include "imgui_internal.h"
#include "imgui_widgets.cpp"
//#include "imgui_demo.cpp"

#include "sqlite3.h"

#if WINDY_INTERNAL
#define output_string(s, ...)        {char Buffer[100];sprintf_s(Buffer, s, __VA_ARGS__);OutputDebugStringA(Buffer);}
#define throw_error_and_exit(e, ...) {output_string(" ------------------------------[ERROR] " ## e, __VA_ARGS__); getchar(); global_error = true;}
#define throw_error(e, ...)           output_string(" ------------------------------[ERROR] " ## e, __VA_ARGS__)
#define inform(i, ...)                output_string(" ------------------------------[INFO] " ## i, __VA_ARGS__)
#define warn(w, ...)                  output_string(" ------------------------------[WARNING] " ## w, __VA_ARGS__)
#else
#define output_string(s, ...)
#define throw_error_and_exit(e, ...)
#define throw_error(e, ...)
#define inform(i, ...)
#define warn(w, ...)
#endif
#define key_down(code, key)    {if(Message.wParam == (code))  input.held.key = 1;}
#define key_up(code, key)      {if(Message.wParam == (code)) {input.held.key = 0;input.pressed.key = 1;}}
#define raw_mouse_button(id, key)  {if(raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_ ## id ##_DOWN)  input.held.key = 1; else if (raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_ ## id ##_UP) {input.held.key = 0;input.pressed.key = 1;}}
#define mouse_down(key)    {input.held.key = 1;}
#define mouse_up(key)      {{input.held.key = 0;input.pressed.key = 1;}}
#define file_time_to_u64(wt) ((wt).dwLowDateTime | ((u64)((wt).dwHighDateTime) << 32))

global sqlite3 *global_db;
global b32 global_running;
global b32 global_error;
global u32 global_width = 1280;
global u32 global_height = 720;
global Platform_Renderer *global_renderer;
global Memory_Pool global_memory = {};
global ImGuiIO *io;

inline u64
win32_get_last_write_time(char *Path)
{
    WIN32_FILE_ATTRIBUTE_DATA FileAttribs = {};
    GetFileAttributesExA(Path, GetFileExInfoStandard, (void *)&FileAttribs);
    FILETIME last_write_time = FileAttribs.ftLastWriteTime;

    u64 result = file_time_to_u64(last_write_time);
    return result;
}

internal
PLATFORM_READ_FILE(win32_read_file)
{
    Input_File Result = {};
    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        // TODO(dave): Currently only supports up to 4GB files
        u32 FileSize = GetFileSize(FileHandle, 0);
        DWORD BytesRead;

        // TODO(dave): Remove this allocation
        u8 *Buffer = (u8 *)VirtualAlloc(0, FileSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if(ReadFile(FileHandle, Buffer, FileSize, &BytesRead, 0))
        {
            Result.data = Buffer;
            Result.size = (u32)BytesRead;
            Result.path = Path;
            Result.write_time = win32_get_last_write_time(Path);
        }
        else
        {
            throw_error("Unable to read file: %s\n", Path);
        }

        CloseHandle(FileHandle);
    }
    else
    {
        throw_error("Unable to open file: %s\n", Path);
        DWORD error = GetLastError();
        error = 0;
    }

    return(Result);
}

internal 
PLATFORM_RELOAD_CHANGED_FILE(win32_reload_file_if_changed)
{
    b32 has_changed = false;
    u64 current_write_time = win32_get_last_write_time(file->path);

    Input_File new_file = {};
    if(current_write_time != file->write_time)
    {
        u32 trial_count = 0;
        while(!new_file.size)
        {
            VirtualFree(new_file.data, 0, MEM_RELEASE);
            new_file = win32_read_file(file->path);
            new_file.write_time = current_write_time;
            has_changed = true;

            if (trial_count++ >= 100) // Try to read until it succeeds, or until trial count reaches max.
            {
                has_changed = false;
                throw_error_and_exit("Couldn't read changed file '%s'!", file->path);
                break;
            }
        }

        if (has_changed) {
            VirtualFree(file->data, 0, MEM_RELEASE);
            *file = new_file;
        }
    }

    return(has_changed);
}

struct Saldo
{
    r32 effective;
    r32 expected;
    r32 credit;
};

#define COL_client   0
#define COL_date     1
#define COL_comment  2
#define COL_value    3
#define COL_promise  4
#define COL_wallet   5
int credit_count_step(void *input, int n_cols, char **cols, char **col_names)
{
    Saldo *saldo = (Saldo *)input;
    char *c = *cols;
    int col_index = 0;

    r32 value = 0;
    b32 promise = 0;
    while (col_index < n_cols)
    {
        switch (col_index)
        {
            case COL_client:
            {
            } break;
            case COL_date:
            {
            } break;
            case COL_comment:
            {
            } break;
            case COL_value:
            {
                sscanf_s(c, "%f", &value);
            } break;
            case COL_promise:
            {
                sscanf_s(c, "%d", &promise);
            } break;
            case COL_wallet:
            {
            } break;
        }
        c = cols[++col_index];
    }

    if (promise)
    {
        saldo->credit += value;
    }
    else
    {
        saldo->effective += value;
    }
    return 0;
}

void credit_count(Saldo *saldo)
{
    char *err = 0;
    sqlite3_exec(global_db, "SELECT * FROM movimenti;", credit_count_step, (void *)saldo, &err);
    Assert(!err);
    saldo->expected = saldo->effective + saldo->credit;
}

struct Linked_Transaction
{
    u32 date;
    r32 value;
    b32 promised;
    char comment[64];
    char wallet[32];
    void *next;
};

struct Transactor
{
    char name[64];
    u32 last_date;
    r32 total_value;
    r32 total_promised;
    Linked_Transaction *transactions;
};

global Transactor global_transactors[50];
global u32 n_transactors;

int store_transaction(void *input, int n_cols, char **cols, char **col_names)
{
    Transactor *transactor = 0;
    for (u32 t_index = 0; t_index < n_transactors; ++t_index)
    {
        if(!strcmp(cols[COL_client], global_transactors[t_index].name))
        {
            transactor = &global_transactors[t_index];
            break;
        }
    }
    if (!transactor)
    {
        transactor = &global_transactors[n_transactors++];
        strcpy(transactor->name, cols[COL_client]);
    }

    Linked_Transaction **tsaction = &transactor->transactions;
    while (*tsaction) {tsaction = (Linked_Transaction **)&(*tsaction)->next;}
    *tsaction = push_struct(Linked_Transaction);

    sscanf_s(cols[COL_date],    "%u", &(*tsaction)->date);
    sscanf_s(cols[COL_value],   "%f", &(*tsaction)->value);
    sscanf_s(cols[COL_promise], "%d", &(*tsaction)->promised);
    strcpy((*tsaction)->comment, cols[COL_comment]);
    strcpy((*tsaction)->wallet,  cols[COL_wallet]);

    if ((*tsaction)->promised)                      transactor->total_promised += (*tsaction)->value;
    else                                            transactor->total_value += (*tsaction)->value;
    if ((*tsaction)->date > transactor->last_date)  transactor->last_date = (*tsaction)->date;

    return 0;
}

#include "win32_renderer_d3d11.cpp"

LRESULT CALLBACK WindyProc(
    HWND   WindowHandle,
    UINT   Message,
    WPARAM w,
    LPARAM l
)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_LBUTTONDOWN: { io->MouseDown[0] = 1; } break;
        case WM_LBUTTONUP:   { io->MouseDown[0] = 0; } break;
        case WM_RBUTTONDOWN: { io->MouseDown[1] = 1; } break;
        case WM_RBUTTONUP:   { io->MouseDown[1] = 0; } break;
        case WM_MBUTTONDOWN: { io->MouseDown[2] = 1; } break;
        case WM_MBUTTONUP:   { io->MouseDown[2] = 0; } break;
        case WM_MOUSEWHEEL: 
        {
            io->MouseWheel = (r32)GET_WHEEL_DELTA_WPARAM(w);
        } break;

        case WM_KEYDOWN:
        {
            if (w < 256)
            {
                if (io)  io->KeysDown[w] = 1;
            }
        } break;

        case WM_KEYUP:
        {
            if (w < 256)
            {
                if (io)  io->KeysDown[w] = 0;
            }
        } break;

        case WM_CLOSE:
        {
            DestroyWindow(WindowHandle);
        } break;

        case WM_DESTROY:
        {
            global_running = false;
            PostQuitMessage(0);
        } break;

        case WM_SIZE:
        {
            global_width = LOWORD(l);
            global_height = HIWORD(l);

            if (global_renderer)  d3d11_resize_render_targets();
        } break;

        // @todo: implement this
        case WM_SETCURSOR:
        {
            HCURSOR arrow = LoadCursor(0, IDC_ARROW);
            SetCursor(arrow);
        } break;

        case WM_CHAR:
        {
            if (w > 0 && w < 0x10000 && io)
            {
                io->AddInputCharacterUTF16((unsigned short)w);
            }
        } break;

        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, w, l);
        } break;
    }
    return(Result);
}

int
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommanLine,
    int ShowFlag
)
{
    // @todo add support for low-resolution performance counters
    // @todo maybe add support for 32-bit
    i64 performance_counter_frequency = 0;
    Assert(QueryPerformanceFrequency((LARGE_INTEGER *)&performance_counter_frequency));
    // @todo make this hardware-dependant
    r32 target_ms_per_frame = 1.f/60.f;

    WNDCLASSA WindyClass = {};
    WindyClass.style = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
    WindyClass.lpfnWndProc = WindyProc;
    //WindyClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindyClass.hInstance = Instance;
    WindyClass.lpszClassName = "WindyClass";
    ATOM Result = RegisterClassA(&WindyClass);

    RECT WindowDimensions = {0, 0, (i32)global_width, (i32)global_height};
    AdjustWindowRect(&WindowDimensions, WS_OVERLAPPEDWINDOW, FALSE);
    WindowDimensions.right -= WindowDimensions.left;
    WindowDimensions.bottom -= WindowDimensions.top;

    HWND main_window = CreateWindowA("WindyClass", "Windy3",
                                    WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    WindowDimensions.right,
                                    WindowDimensions.bottom,
                                    0, 0, Instance, 0);

    if(main_window)
    {
        ShowCursor(1);
        RECT window_rect = {};
        GetWindowRect(main_window, &window_rect);
        u32 window_width  = window_rect.right - window_rect.left;
        u32 window_height = window_rect.bottom - window_rect.top;

#if WINDY_INTERNAL
        LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
        LPVOID BaseAddress = 0;
#endif
        global_running = true;
        global_error = false;
        global_memory.size = Megabytes(500);
        global_memory.base = (u8 *)VirtualAlloc(BaseAddress, global_memory.size,
                                                MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

        // @todo: probably a global variable is a good idea?
        Platform_Renderer renderer = {};
        D11_Renderer d11 = {};
        renderer.load_renderer    = win32_load_d3d11;
        renderer.clear               = d3d11_clear;
        renderer.set_render_targets  = d3d11_set_default_render_targets;
        renderer.platform = (void *)&d11;
        global_renderer = &renderer;

        DXGI_MODE_DESC display_mode_desc = {};
        //DisplayModeDescriptor.Width = global_width;
        //DisplayModeDescriptor.Height = global_height;
        display_mode_desc.RefreshRate = {60, 1};
        display_mode_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        display_mode_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        display_mode_desc.Scaling = DXGI_MODE_SCALING_CENTERED;

        DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
        swap_chain_desc.BufferDesc = display_mode_desc;
        swap_chain_desc.SampleDesc = {1, 0};
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 2;
        swap_chain_desc.OutputWindow = main_window;
        swap_chain_desc.Windowed = true;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swap_chain_desc.Flags;

        D3D11CreateDeviceAndSwapChain(
            0,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT|D3D11_CREATE_DEVICE_DEBUG,
            0,
            0,
            D3D11_SDK_VERSION,
            &swap_chain_desc,
            &d11.swap_chain,
            &d11.device,
            0,
            &d11.context
        );

        renderer.load_renderer(&renderer);

        ImGui::CreateContext();
        io = &ImGui::GetIO();
        ImGui_ImplWin32_Init(main_window);
        ImGui_ImplDX11_Init(d11.device, d11.context);


        // ===========================================================================================
        // Sqlite3 
        // ===========================================================================================
        char *dberr = 0;
        sqlite3_open("data.db", &global_db);
        sqlite3_exec(global_db, "CREATE TABLE IF NOT EXISTS saldo(name TEXT PRIMARY KEY, comment TEXT, value REAL NOT NULL);", 0, 0, &dberr);
        //sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Entrate',   '', 0);", 0, 0, &dberr);
        //sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Usicte',    '', 0);", 0, 0, &dberr);
        //sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Attivita',  '', 0);", 0, 0, &dberr);
        //sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Passivita', '', 0);", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Paypal 1', 'Paypal di Giacomo', 0);", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Paypal 2', 'Paypal di Angelo', 0);", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO saldo VALUES ('Paypal 3', 'Paypal di Lello', 0);", 0, 0, &dberr);

        sqlite3_exec(global_db, "CREATE TABLE IF NOT EXISTS\
                     movimenti(cliente TEXT, data INT, commento TEXT, valore REAL NOT NULL, promessa BOOL, tasca TEXT, PRIMARY KEY (cliente, data), FOREIGN KEY(tasca) REFERENCES saldo(name));",
                     0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Carmelo',   123412341234, 'mi ha chiamato bello',  17.35, 1, 'Paypal 1');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Carmelo',   123412341235,                     '',  15,    0, 'Paypal 1');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Ezechiele', 123412341235,   'viene dalla bibbia', -15,    0, 'Paypal 1');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Ezechiele', 123412341237,   'viene dalla bibbia', -07,    1, 'Paypal 1');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Carmelo',   123412341254, 'mi ha chiamato bello',  17.35, 1, 'Paypal 2');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Carmelo',   123412341255,                     '',  15,    0, 'Paypal 2');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Ezechiele', 123412341255,   'viene dalla bibbia', -10,    1, 'Paypal 2');", 0, 0, &dberr);
        sqlite3_exec(global_db, "INSERT OR IGNORE INTO movimenti VALUES ('Ezechiele', 123412341257,   'viene dalla bibbia', -10,    0, 'Paypal 2');", 0, 0, &dberr);
        Assert(!dberr);

        sqlite3_exec(global_db, "SELECT * FROM movimenti;", store_transaction, 0, &dberr);
        Assert(!dberr);

        MSG Message = {};
        u32 Count = 0;

        i64 last_performance_counter = 0;
        i64 current_performance_counter = 0;
        Assert(QueryPerformanceCounter((LARGE_INTEGER *)&last_performance_counter));
        while(global_running && !global_error)
        {
            Assert(QueryPerformanceCounter((LARGE_INTEGER *)&current_performance_counter));
            r32 dtime = (r32)(current_performance_counter - last_performance_counter) / (r32)performance_counter_frequency;
            while(dtime <= target_ms_per_frame)
            {
                while(PeekMessageA(&Message, main_window, 0, 0, PM_REMOVE))
                {
//                        case WM_MOUSEMOVE:
//                        {
//                            // @todo: doing the division here might cause problems if a window resize
//                            //        happens between frames.
//                            input.mouse.x = (r32)(((i16*)&Message.lParam)[0]) / global_width;
//                            input.mouse.y = (r32)(((i16*)&Message.lParam)[1]) / global_height;
//                        } break;

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                Assert(QueryPerformanceCounter((LARGE_INTEGER *)&current_performance_counter));
                dtime = (r32)(current_performance_counter - last_performance_counter) / (r32)performance_counter_frequency;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            char text_buffer[500] = {};

            renderer.set_render_targets();
            renderer.clear({0.06f, 0.3f, 0.45f});

            ImGui::Begin("IziWallet", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            ImGui::SetWindowSize(ImVec2((r32)global_width, (r32)global_height));
            ImGui::SetWindowPos(ImVec2(0, 0));
            ImGui::Text("Transactions");

            Saldo saldo = {};

            credit_count(&saldo);

            sprintf_s(text_buffer, "Saldo Effettivo: %.2f", saldo.effective);
            if (ImGui::Button(text_buffer))
                ImGui::OpenPopup("popup_credit");
            if (ImGui::BeginPopup("popup_credit"))
            {
                sprintf_s(text_buffer, "Saldo Previsto: %.2f", saldo.expected);
                ImGui::Text(text_buffer);
                sprintf_s(text_buffer, "Credito: %.2f", saldo.credit);
                ImGui::Text(text_buffer);
                ImGui::EndPopup();
            }

            ImGui::Text("");
            ImGui::Text("");
            ImGui::Text("");
            ImGui::Text("Movimenti");
            ImGui::Columns(6, "movimenti");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Cliente");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Data");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Dettagli");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Entrata/Usicta");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Attivita/Passivita");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Tasca");  ImGui::NextColumn();
            ImGui::Separator();

            static b32 selections[50] = {};
            for (u32 trans_index = 0; trans_index < n_transactors; ++trans_index)
            {
                b32 expand = 0;
                Transactor *tsactor = &global_transactors[trans_index];
                if (ImGui::Selectable(tsactor->name, false, ImGuiSelectableFlags_SpanAllColumns))
                    selections[trans_index] = !selections[trans_index];
                ImGui::NextColumn();

                sprintf_s(text_buffer, "%u", tsactor->last_date);
                ImGui::Text(text_buffer);                                    ImGui::NextColumn();

                ImGui::Text("");                                             ImGui::NextColumn();
                
                sprintf_s(text_buffer, "% .2f", tsactor->total_value);
                ImGui::Text(text_buffer);                                    ImGui::NextColumn();

                sprintf_s(text_buffer, "% .2f", tsactor->total_promised);
                ImGui::Text(text_buffer);                                    ImGui::NextColumn();
                
                ImGui::Text("TODO");                                         ImGui::NextColumn();

                if (selections[trans_index])
                {
                    Linked_Transaction *tsaction = tsactor->transactions;
                    while (tsaction)
                    {
                        ImGui::NextColumn();

                        sprintf_s(text_buffer, "%u", tsactor->last_date);
                        ImGui::Text(text_buffer);                                    ImGui::NextColumn();

                        ImGui::Text(tsaction->comment);                              ImGui::NextColumn();

                        sprintf_s(text_buffer, "% .2f", tsaction->value);
                        if (tsaction->promised)
                        {
                            ImGui::NextColumn();
                            ImGui::Text(text_buffer);                                ImGui::NextColumn();
                        }
                        else
                        {
                            ImGui::Text(text_buffer);                                ImGui::NextColumn();
                            ImGui::NextColumn();
                        }


                        ImGui::Text(tsaction->wallet);                               ImGui::NextColumn();

                        tsaction = (Linked_Transaction *)tsaction->next;
                    }
                }
                ImGui::Separator();
            }

            local_persist char buf_client[64] = {};
            ImGui::InputText("", buf_client, 64);

            ImGui::End();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            last_performance_counter = current_performance_counter;
            d11.swap_chain->Present(0, 0);
        }
    }
    else
    {
        // TODO: Logging
        return 1;
    }

    if (global_error)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
