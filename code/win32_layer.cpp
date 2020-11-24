/* ========================================================================
   $File: $
   $Date: $
   $Version: 0.1 $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2014 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#define MAX_TRANSACTION_DETAIL_LENGTH 64
#define MAX_TRANSACTOR_NAME_LENGTH    32
#define PAYPAL_NAME_INPUT_WIDTH       (256+128)
#define MAX_PAYPAL_NAME_LENGTH        MAX_TRANSACTOR_NAME_LENGTH
#define MAX_PAYPALS                   10

#include <windows.h>
#include <d3d11.h>

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

inline r32
Max(r32 a, r32 b)
{
    r32 result = a;
    if (b > a)
        result = b;
    return result;
}

inline u64
win32_get_last_write_time(char *Path)
{
    WIN32_FILE_ATTRIBUTE_DATA FileAttribs = {};
    GetFileAttributesExA(Path, GetFileExInfoStandard, (void *)&FileAttribs);
    FILETIME last_write_time = FileAttribs.ftLastWriteTime;

    u64 result = file_time_to_u64(last_write_time);
    return result;
}

internal Input_File win32_read_file(char *Path)
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

internal b32 win32_reload_file_if_changed(Input_File *file)
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
    
    r32 paypals[MAX_PAYPALS];
    char paypal_ids[MAX_PAYPALS][MAX_PAYPAL_NAME_LENGTH];
    u32 n_paypals;
};

struct Linked_Transaction
{
    u64 date;
    r32 value;
    b32 promised;
    char details[MAX_TRANSACTION_DETAIL_LENGTH];
    u32 wallet;
    void *next;
};

struct Transactor
{
    char name[MAX_TRANSACTOR_NAME_LENGTH];
    u64 last_date;
    r32 total_value;
    r32 total_promised;
    Linked_Transaction *transactions;
};

int init_paypals(void *input, int n_cols, char **cols, char **col_names)
{
    Saldo *saldo = (Saldo *)input;
    u32 index;
    sscanf_s(cols[0], "%u", &index);

    Assert((index-1) == saldo->n_paypals);
    strcpy(saldo->paypal_ids[saldo->n_paypals++], cols[1]);
    return 0;
}

#define COL_client   0
#define COL_date     1
#define COL_details  2
#define COL_value    3
#define COL_promise  4
#define COL_wallet   5
int credit_count_step(void *input, int n_cols, char **cols, char **col_names)
{
    Saldo *saldo = (Saldo *)input;

    r32 value = 0;
    b32 promise = 0;
    sscanf_s(cols[COL_value],   "%f", &value);
    sscanf_s(cols[COL_promise], "%d", &promise);

    if (promise)
        saldo->credit += value;
    else
    {
        saldo->effective += value;

        u32 p_index = 0;
        sscanf_s(cols[COL_wallet], "Paypal %u", &p_index);
        Assert(p_index < 3);
        saldo->paypals[p_index] += value;
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

void credit_manual_add(Saldo *saldo, Linked_Transaction *tsaction)
{
    if (tsaction->promised)
        saldo->credit += tsaction->promised;
    else
    {
        saldo->effective += tsaction->value;

        u32 p_index = 0;
        saldo->paypals[tsaction->wallet] += tsaction->value;
    }
}

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

    sscanf_s(cols[COL_date],    "%llu", &(*tsaction)->date);
    sscanf_s(cols[COL_value],   "%f", &(*tsaction)->value);
    sscanf_s(cols[COL_promise], "%d", &(*tsaction)->promised);
    strcpy((*tsaction)->details, cols[COL_details]);
    sscanf_s(cols[COL_wallet], "%u", &(*tsaction)->wallet);

    if ((*tsaction)->promised)                      transactor->total_promised += (*tsaction)->value;
    else                                            transactor->total_value += (*tsaction)->value;
    if ((*tsaction)->date > transactor->last_date)  transactor->last_date = (*tsaction)->date;

    if (input)
    {
        credit_manual_add((Saldo *)input, *tsaction);
    }

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
    LRESULT result = 1;
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
            if (LOWORD(l) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            {}
            else if ((LOWORD(l) == HTRIGHT) || (LOWORD(l) == HTLEFT))
                SetCursor(LoadCursor(0, IDC_SIZEWE));
            else if ((LOWORD(l) == HTTOP) || (LOWORD(l) == HTBOTTOM))
                SetCursor(LoadCursor(0, IDC_SIZENS));
            else if ((LOWORD(l) == HTBOTTOMLEFT) || (LOWORD(l) == HTTOPRIGHT))
                SetCursor(LoadCursor(0, IDC_SIZENESW));
            else if ((LOWORD(l) == HTBOTTOMRIGHT) || (LOWORD(l) == HTTOPLEFT))
                SetCursor(LoadCursor(0, IDC_SIZENWSE));
            else
                result = 0;
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
            result = DefWindowProcA(WindowHandle, Message, w, l);
        } break;
    }
    return(result);
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
    s64 performance_counter_frequency = 0;
    AssertKeep(QueryPerformanceFrequency((LARGE_INTEGER *)&performance_counter_frequency));
    // @todo make this hardware-dependant
    r32 target_ms_per_frame = 1.f/60.f;

    WNDCLASSA WindyClass = {};
    WindyClass.style = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
    WindyClass.lpfnWndProc = WindyProc;
    //WindyClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindyClass.hInstance = Instance;
    WindyClass.lpszClassName = "WindyClass";
    ATOM Result = RegisterClassA(&WindyClass);

    RECT WindowDimensions = {0, 0, (s32)global_width, (s32)global_height};
    AdjustWindowRect(&WindowDimensions, WS_OVERLAPPEDWINDOW, FALSE);
    WindowDimensions.right -= WindowDimensions.left;
    WindowDimensions.bottom -= WindowDimensions.top;

    HWND main_window = CreateWindowA("WindyClass", "Iziwallet beta 0.1",
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
#if WINDY_DEBUG
            D3D11_CREATE_DEVICE_BGRA_SUPPORT|D3D11_CREATE_DEVICE_DEBUG,
#else
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#endif
            0,
            0,
            D3D11_SDK_VERSION,
            &swap_chain_desc,
            &d11.swap_chain,
            &d11.device,
            0,
            &d11.context
        );

        win32_load_d3d11(&renderer);

        ImGui::CreateContext();
        io = &ImGui::GetIO();
        io->Fonts->AddFontFromFileTTF("fonts/Redaction.otf", 24.f);
        ImGui_ImplWin32_Init(main_window);
        ImGui_ImplDX11_Init(d11.device, d11.context);
        ImGui::StyleColorsLight();
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


        // ===========================================================================================
        // Sqlite3 
        // ===========================================================================================
        Saldo saldo = {};

        char *dberr = 0;
        sqlite3_open("data.db", &global_db);
        sqlite3_exec(global_db, "CREATE TABLE IF NOT EXISTS conti(nome TEXT PRIMARY KEY, valore REAL NOT NULL);", 0, 0, &dberr);

        sqlite3_exec(global_db, "CREATE TABLE IF NOT EXISTS\
                     movimenti(cliente TEXT, data INT, dettagli TEXT, valore REAL NOT NULL, promessa BOOL, tasca INT, PRIMARY KEY (cliente, data), FOREIGN KEY(tasca) REFERENCES saldo(rowid));",
                     0, 0, &dberr);
        Assert(!dberr);

        sqlite3_exec(global_db, "SELECT rowid,nome FROM conti ORDER BY rowid;", init_paypals, (void *)&saldo, &dberr);
        Assert(!dberr);

        sqlite3_exec(global_db, "SELECT * FROM movimenti;", store_transaction, 0, &dberr);
        Assert(!dberr);
        credit_count(&saldo);

        MSG Message = {};
        u32 Count = 0;

        s64 last_performance_counter = 0;
        s64 current_performance_counter = 0;
        AssertKeep(QueryPerformanceCounter((LARGE_INTEGER *)&last_performance_counter));
        while(global_running && !global_error)
        {
            AssertKeep(QueryPerformanceCounter((LARGE_INTEGER *)&current_performance_counter));
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
                AssertKeep(QueryPerformanceCounter((LARGE_INTEGER *)&current_performance_counter));
                dtime = (r32)(current_performance_counter - last_performance_counter) / (r32)performance_counter_frequency;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            char text_buffer[500] = {};

            d3d11_set_default_render_targets();
            d3d11_clear({1.f, 1.f, 1.f});//{0.06f, 0.3f, 0.45f});

            ImGui::Begin("IziWallet", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            ImGui::SetWindowSize(ImVec2((r32)global_width, (r32)global_height));
            ImGui::SetWindowPos(ImVec2(0, 0));

            ImGui::Columns(2, "saldo");
            ImGui::Text("Saldo Effettivo: %.2f", saldo.effective);
            ImGui::Text("Saldo Previsto: %.2f", saldo.expected);
            ImGui::Text("Credito: %.2f", saldo.credit);

            ImGui::NextColumn();

            if (!saldo.n_paypals)
            {
                ImGui::TextColored(ImVec4(1.f, 0.2f, 0.f, 1.f), "Non ci sono conti PayPal!");
            }
            else
            {
                for (u32 paypal_id = 0; paypal_id < saldo.n_paypals; ++paypal_id)
                {
                    sprintf_s(text_buffer, "E##%u", paypal_id);
                    local_persist s32 editing = -1;
                    if (ImGui::Button(text_buffer) && (editing < 0))
                        editing = paypal_id;

                    if (editing == (s32)paypal_id) {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(PAYPAL_NAME_INPUT_WIDTH);
                        if(ImGui::InputText("##edit_paypal_id", saldo.paypal_ids[paypal_id], MAX_PAYPAL_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            sprintf_s(text_buffer, "UPDATE OR FAIL conti SET nome='%s' WHERE rowid=%u;", saldo.paypal_ids[paypal_id], paypal_id+1);
                            sqlite3_exec(global_db, text_buffer, 0, 0, &dberr);
                            Assert(!dberr);
                            editing = -1;
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::Text("%s: %.2f", saldo.paypal_ids[paypal_id], saldo.paypals[paypal_id]);
                    }
                }
            }
            {
                ImGui::PushItemWidth(PAYPAL_NAME_INPUT_WIDTH);
                ImGui::InputText("##new_paypal_id", saldo.paypal_ids[saldo.n_paypals], MAX_PAYPAL_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::SameLine();
                if (ImGui::Button("+##add_paypal"))
                {
                    sprintf_s(text_buffer, "INSERT OR IGNORE INTO conti VALUES ('%s', 0);", saldo.paypal_ids[saldo.n_paypals++]);
                    sqlite3_exec(global_db, text_buffer, 0, 0, &dberr);
                }

            }

            ImGui::Columns(1);

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
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Attività/Passività");  ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.f), "Tasca");  ImGui::NextColumn();
            ImGui::Separator();

            static b32 selections[50] = {};
            for (u32 trans_index = 0; trans_index < n_transactors; ++trans_index)
            {
                b32 expand = 0;
                Transactor *tsactor = &global_transactors[trans_index];
                r32 promised_after_payments = Max(tsactor->total_promised - tsactor->total_value, 0.f);

                if (trans_index & 0x1)
                    ImGui::PushStyleColor(ImGuiCol_Header, (u32)(0x22 << 24));
                else
                    ImGui::PushStyleColor(ImGuiCol_Header, (u32)(0x33 << 24));
                if (ImGui::Selectable(tsactor->name, true, ImGuiSelectableFlags_SpanAllColumns))
                    selections[trans_index] = !selections[trans_index];
                ImGui::NextColumn();

                SYSTEMTIME st = {};
                FileTimeToSystemTime((FILETIME *)&tsactor->last_date, &st);
                ImGui::Text("%02d/%02d/%d %02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);   ImGui::NextColumn();

                ImGui::Text("");                                                                             ImGui::NextColumn();
                
                if ((!selections[trans_index]) || (!promised_after_payments))
                    sprintf_s(text_buffer, "% .2f", tsactor->total_value);
                else
                    text_buffer[0] = 0;
                ImGui::Text(text_buffer);                                                                    ImGui::NextColumn();

                ImGui::Text("% .2f", promised_after_payments);                                               ImGui::NextColumn();
                
                ImGui::Text("TODO");                                                                         ImGui::NextColumn();

                if (selections[trans_index])
                {
                    Linked_Transaction *tsaction = tsactor->transactions;
                    while (tsaction)
                    {
                        if (!tsaction->promised)
                        {
                            if (tsaction->value < 0.f)   ImGui::PushStyleColor(ImGuiCol_Header, 0x44050FDD);
                            else                         ImGui::PushStyleColor(ImGuiCol_Header, 0x330FDD00);
                            ImGui::Selectable("", true, ImGuiSelectableFlags_SpanAllColumns);
                            ImGui::NextColumn();

                            FileTimeToSystemTime((FILETIME *)&tsaction->date, &st);
                            ImGui::Text("%02d/%02d/%d %02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);   ImGui::NextColumn();

                            ImGui::Text(tsaction->details);                                                              ImGui::NextColumn();

                            sprintf_s(text_buffer, "% .2f", tsaction->value);
                            if (tsaction->promised)
                            {
                                ImGui::NextColumn();
                                ImGui::Text(text_buffer);                                                                ImGui::NextColumn();
                            }
                            else
                            {
                                ImGui::Text(text_buffer);                                                                ImGui::NextColumn();
                                ImGui::NextColumn();
                            }


                            ImGui::Text(saldo.paypal_ids[tsaction->wallet]);                                             ImGui::NextColumn();

                            ImGui::PopStyleColor();
                        }
                        tsaction = (Linked_Transaction *)tsaction->next;
                    }
                }
                ImGui::Separator();
                ImGui::PopStyleColor();
            }

            static int paypal_current = 0;

            local_persist char buf_client [64] = {};
            local_persist char buf_comment[64] = {};
            local_persist char buf_value  [64] = {};
            local_persist char buf_promise[64] = {};
            ImGui::PushItemWidth(-1.f);
            ImGui::InputText("##cl", buf_client,  64, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine);                                       ImGui::NextColumn();      ImGui::NextColumn();
            ImGui::PushItemWidth(-1.f);
            ImGui::InputText("##co", buf_comment, 64, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine);                                       ImGui::NextColumn();
            ImGui::PushItemWidth(-1.f);
            ImGui::InputText("##va", buf_value,   64, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine|ImGuiInputTextFlags_CharsDecimal);      ImGui::NextColumn();
            ImGui::PushItemWidth(-1.f);
            ImGui::InputText("##pr", buf_promise, 64, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine|ImGuiInputTextFlags_CharsDecimal);      ImGui::NextColumn();
            ImGui::PushItemWidth(-1.f);
            if (ImGui::BeginCombo("", saldo.paypal_ids[paypal_current]))
            {
                for (int n = 0; n < 3; n++)
                {
                    const bool is_selected = (paypal_current == n);
                    if (ImGui::Selectable(saldo.paypal_ids[n], is_selected))
                        paypal_current = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::NextColumn();
            ImGui::Columns(1);
            if (ImGui::Button("Inserisci"))
            {
                Assert((!buf_value[0]) != (!buf_promise[0]));
                SYSTEMTIME system_time = {};
                FILETIME epoch = {};
                GetSystemTime(&system_time);
                SystemTimeToFileTime(&system_time, &epoch);

                b32 is_promise = !!buf_promise[0];
                sprintf_s(text_buffer, "INSERT OR IGNORE INTO movimenti VALUES ('%s', %llu, '%s', %s, %d, 'Paypal %1u');", buf_client, *((u64 *)&epoch), buf_comment, is_promise ? buf_promise : buf_value, is_promise, paypal_current);
                sqlite3_exec(global_db, text_buffer, 0, 0, &dberr);
                Assert(!dberr);

                s64 last_rowid = sqlite3_last_insert_rowid(global_db);
                if (last_rowid)
                {
                    sprintf_s(text_buffer, "SELECT * FROM movimenti WHERE rowid = %lld;", last_rowid);
                    sqlite3_exec(global_db, text_buffer, store_transaction, (void *)&saldo, &dberr);
                }
            }

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
