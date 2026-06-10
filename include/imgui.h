// (headers)

// Help:

// Resources:


#define IMGUI_VERSION       "1.92.5"
#define IMGUI_VERSION_NUM   19250
#define IMGUI_HAS_TABLE             // Added BeginTable() - from IMGUI_VERSION_NUM >= 18000
#define IMGUI_HAS_TEXTURES          // Added ImGuiBackendFlags_RendererHasTextures - from IMGUI_VERSION_NUM >= 19198

/*

Index of this file:

*/

#pragma once

#ifdef IMGUI_USER_CONFIG
#include IMGUI_USER_CONFIG
#endif
#include "imconfig.h"

#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Includes
#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

#ifndef IMGUI_API
#define IMGUI_API
#endif
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API              IMGUI_API
#endif

#ifndef IM_ASSERT
#include <assert.h>
#define IM_ASSERT(_EXPR)            assert(_EXPR)                               // You can override the default assert handler by editing imconfig.h
#endif
#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!
#define IM_UNUSED(_VAR)             ((void)(_VAR))                              // Used to silence "unused variable warnings". Often useful as asserts may be stripped out from final builds.
#define IM_STRINGIFY_HELPER(_EXPR)  #_EXPR
#define IM_STRINGIFY(_EXPR)         IM_STRINGIFY_HELPER(_EXPR)                  // Preprocessor idiom to stringify e.g. an integer or a macro.

#define IMGUI_CHECKVERSION()        ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx))

#if !defined(IMGUI_USE_STB_SPRINTF) && defined(__MINGW32__) && !defined(__clang__)
#define IM_FMTARGS(FMT)             __attribute__((format(gnu_printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(gnu_printf, FMT, 0)))
#elif !defined(IMGUI_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define IM_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#else
#define IM_FMTARGS(FMT)
#define IM_FMTLIST(FMT)
#endif

#if defined(_MSC_VER) && !defined(__clang__)  && !defined(__INTEL_COMPILER) && !defined(IMGUI_DEBUG_PARANOID)
#define IM_MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define IM_MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define IM_MSVC_RUNTIME_CHECKS_OFF
#define IM_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant
#pragma clang diagnostic ignored "-Wreserved-identifier"            // warning: identifier '_Xxx' is reserved because it starts with '_' followed by a capital letter
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef unsigned int        ImGuiID;// A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char         ImS8;   // 8-bit signed integer
typedef unsigned char       ImU8;   // 8-bit unsigned integer
typedef signed short        ImS16;  // 16-bit signed integer
typedef unsigned short      ImU16;  // 16-bit unsigned integer
typedef signed int          ImS32;  // 32-bit signed integer == int
typedef unsigned int        ImU32;  // 32-bit unsigned integer (often used to store packed colors)
typedef signed   long long  ImS64;  // 64-bit signed integer
typedef unsigned long long  ImU64;  // 64-bit unsigned integer

struct ImDrawChannel;               // Temporary storage to output draw commands out of order, used by ImDrawListSplitter and ImDrawList::ChannelsSplit()
struct ImDrawCmd;                   // A single draw command within a parent ImDrawList (generally maps to 1 GPU draw call, unless it is a callback)
struct ImDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
struct ImDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
struct ImDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
struct ImDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
struct ImDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct ImFont;                      // Runtime data for a single font within a parent ImFontAtlas
struct ImFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct ImFontAtlasBuilder;          // Opaque storage for building a ImFontAtlas
struct ImFontAtlasRect;             // Output of ImFontAtlas::GetCustomRect() when using custom rectangles.
struct ImFontBaked;                 // Baked data for a ImFont at a given size.
struct ImFontConfig;                // Configuration data when adding a font or merging fonts
struct ImFontGlyph;                 // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
struct ImFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
struct ImFontLoader;                // Opaque interface to a font loading backend (stb_truetype, FreeType etc.).
struct ImTextureData;               // Specs and pixel storage for a texture used by Dear ImGui.
struct ImTextureRect;               // Coordinates of a rectangle within a texture.
struct ImColor;                     // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)

struct ImGuiContext;                // Dear ImGui context (opaque structure, unless including imgui_internal.h)
struct ImGuiIO;                     // Main configuration and I/O between your application and ImGui (also see: ImGuiPlatformIO)
struct ImGuiInputTextCallbackData;  // Shared state of InputText() when using custom ImGuiInputTextCallback (rare/advanced use)
struct ImGuiKeyData;                // Storage for ImGuiIO and IsKeyDown(), IsKeyPressed() etc functions.
struct ImGuiListClipper;            // Helper to manually clip large list of items
struct ImGuiMultiSelectIO;          // Structure to interact with a BeginMultiSelect()/EndMultiSelect() block
struct ImGuiOnceUponAFrame;         // Helper for running a block of code not more than once a frame
struct ImGuiPayload;                // User data payload for drag and drop operations
struct ImGuiPlatformIO;             // Interface between platform/renderer backends and ImGui (e.g. Clipboard, IME hooks). Extends ImGuiIO. In docking branch, this gets extended to support multi-viewports.
struct ImGuiPlatformImeData;        // Platform IME data for io.PlatformSetImeDataFn() function.
struct ImGuiSelectionBasicStorage;  // Optional helper to store multi-selection state + apply multi-selection requests.
struct ImGuiSelectionExternalStorage;//Optional helper to apply multi-selection requests to existing randomly accessible storage.
struct ImGuiSelectionRequest;       // A selection request (stored in ImGuiMultiSelectIO)
struct ImGuiSizeCallbackData;       // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
struct ImGuiStorage;                // Helper for key->value storage (container sorted by key)
struct ImGuiStoragePair;            // Helper for key->value storage (pair)
struct ImGuiStyle;                  // Runtime data for styling/colors
struct ImGuiTableSortSpecs;         // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
struct ImGuiTableColumnSortSpecs;   // Sorting specification for one column of a table
struct ImGuiTextBuffer;             // Helper to hold and append into a text buffer (~string builder)
struct ImGuiTextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct ImGuiViewport;               // A Platform Window (always only one in 'master' branch), in the future may represent Platform Monitor

// Enumerations
enum ImGuiDir : int;                // -> enum ImGuiDir              // Enum: A cardinal direction (Left, Right, Up, Down)
enum ImGuiKey : int;                // -> enum ImGuiKey              // Enum: A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value)
enum ImGuiMouseSource : int;        // -> enum ImGuiMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
enum ImGuiSortDirection : ImU8;     // -> enum ImGuiSortDirection    // Enum: A sorting direction (ascending or descending)
typedef int ImGuiCol;               // -> enum ImGuiCol_             // Enum: A color identifier for styling
typedef int ImGuiCond;              // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
typedef int ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type
typedef int ImGuiMouseButton;       // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int ImGuiMouseCursor;       // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor shape
typedef int ImGuiStyleVar;          // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
typedef int ImGuiTableBgTarget;     // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()

typedef int ImDrawFlags;            // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
typedef int ImDrawListFlags;        // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
typedef int ImDrawTextFlags;        // -> enum ImDrawTextFlags_      // Internal, do not use!
typedef int ImFontFlags;            // -> enum ImFontFlags_          // Flags: for ImFont
typedef int ImFontAtlasFlags;       // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas
typedef int ImGuiBackendFlags;      // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
typedef int ImGuiButtonFlags;       // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()
typedef int ImGuiChildFlags;        // -> enum ImGuiChildFlags_      // Flags: for BeginChild()
typedef int ImGuiColorEditFlags;    // -> enum ImGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int ImGuiConfigFlags;       // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags
typedef int ImGuiComboFlags;        // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
typedef int ImGuiDragDropFlags;     // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int ImGuiFocusedFlags;      // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
typedef int ImGuiHoveredFlags;      // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int ImGuiInputFlags;        // -> enum ImGuiInputFlags_      // Flags: for Shortcut(), SetNextItemShortcut()
typedef int ImGuiInputTextFlags;    // -> enum ImGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int ImGuiItemFlags;         // -> enum ImGuiItemFlags_       // Flags: for PushItemFlag(), shared by all items
typedef int ImGuiKeyChord;          // -> ImGuiKey | ImGuiMod_XXX    // Flags: for IsKeyChordPressed(), Shortcut() etc. an ImGuiKey optionally OR-ed with one or more ImGuiMod_XXX values.
typedef int ImGuiListClipperFlags;  // -> enum ImGuiListClipperFlags_// Flags: for ImGuiListClipper
typedef int ImGuiPopupFlags;        // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int ImGuiMultiSelectFlags;  // -> enum ImGuiMultiSelectFlags_// Flags: for BeginMultiSelect()
typedef int ImGuiSelectableFlags;   // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()
typedef int ImGuiSliderFlags;       // -> enum ImGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int ImGuiTabBarFlags;       // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
typedef int ImGuiTabItemFlags;      // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
typedef int ImGuiTableFlags;        // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
typedef int ImGuiTableColumnFlags;  // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
typedef int ImGuiTableRowFlags;     // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()
typedef int ImGuiTreeNodeFlags;     // -> enum ImGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int ImGuiViewportFlags;     // -> enum ImGuiViewportFlags_   // Flags: for ImGuiViewport
typedef int ImGuiWindowFlags;       // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

typedef unsigned int ImWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned short ImWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef IMGUI_USE_WCHAR32            // ImWchar [configurable type: override in imconfig.h with '#define IMGUI_USE_WCHAR32' to support Unicode planes 1-16]
typedef ImWchar32 ImWchar;
#else
typedef ImWchar16 ImWchar;
#endif

typedef ImS64 ImGuiSelectionUserData;

typedef int     (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData* data);    // Callback function for ImGui::InputText()
typedef void    (*ImGuiSizeCallback)(ImGuiSizeCallbackData* data);              // Callback function for ImGui::SetNextWindowSizeConstraints()
typedef void*   (*ImGuiMemAllocFunc)(size_t sz, void* user_data);               // Function signature for ImGui::SetAllocatorFunctions()
typedef void    (*ImGuiMemFreeFunc)(void* ptr, void* user_data);                // Function signature for ImGui::SetAllocatorFunctions()

IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec2
{
    float                                   x, y;
    constexpr ImVec2()                      : x(0.0f), y(0.0f) { }
    constexpr ImVec2(float _x, float _y)    : x(_x), y(_y) { }
    float& operator[] (size_t idx)          { IM_ASSERT(idx == 0 || idx == 1); return ((float*)(void*)(char*)this)[idx]; } // We very rarely use this [] operator, so the assert overhead is fine.
    float  operator[] (size_t idx) const    { IM_ASSERT(idx == 0 || idx == 1); return ((const float*)(const void*)(const char*)this)[idx]; }
#ifdef IM_VEC2_CLASS_EXTRA
    IM_VEC2_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec2.
#endif
};

struct ImVec4
{
    float                                                     x, y, z, w;
    constexpr ImVec4()                                        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    constexpr ImVec4(float _x, float _y, float _z, float _w)  : x(_x), y(_y), z(_z), w(_w) { }
#ifdef IM_VEC4_CLASS_EXTRA
    IM_VEC4_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec4.
#endif
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//   (e.g. Used by DX11 backend to a `ID3D11ShaderResourceView*`; Used by OpenGL backends to store `GLuint`;
// History:
#ifndef ImTextureID
typedef ImU64 ImTextureID;      // Default: store up to 64-bits (any pointer or integer). A majority of backends are ok with that.
#endif

#ifndef ImTextureID_Invalid
#define ImTextureID_Invalid     ((ImTextureID)0)
#endif

IM_MSVC_RUNTIME_CHECKS_OFF
struct ImTextureRef
{
    ImTextureRef()                          { _TexData = NULL; _TexID = ImTextureID_Invalid; }
    ImTextureRef(ImTextureID tex_id)        { _TexData = NULL; _TexID = tex_id; }
#if !defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && !defined(ImTextureID)
    ImTextureRef(void* tex_id)              { _TexData = NULL; _TexID = (ImTextureID)(size_t)tex_id; }  // For legacy backends casting to ImTextureID
#endif

    inline ImTextureID  GetTexID() const;   // == (_TexData ? _TexData->TexID : _TexID) // Implemented below in the file.

    ImTextureData*      _TexData;           //      A texture, generally owned by a ImFontAtlas. Will convert to ImTextureID during render loop, after texture has been uploaded.
    ImTextureID         _TexID;             // _OR_ Low-level backend texture identifier, if already uploaded or created by user/app. Generally provided to e.g. ImGui::Image() calls.
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace ImGui
{
    IMGUI_API ImGuiContext* CreateContext(ImFontAtlas* shared_font_atlas = NULL);
    IMGUI_API void          DestroyContext(ImGuiContext* ctx = NULL);   // NULL = destroy current context
    IMGUI_API ImGuiContext* GetCurrentContext();
    IMGUI_API void          SetCurrentContext(ImGuiContext* ctx);

    // Main
    IMGUI_API ImGuiIO&      GetIO();                                    // access the ImGuiIO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
    IMGUI_API ImGuiPlatformIO& GetPlatformIO();                         // access the ImGuiPlatformIO structure (mostly hooks/functions to connect to platform/renderer and OS Clipboard, IME etc.)
    IMGUI_API ImGuiStyle&   GetStyle();                                 // access the Style structure (colors, sizes). Always use PushStyleColor(), PushStyleVar() to modify style mid-frame!
    IMGUI_API void          NewFrame();                                 // start a new Dear ImGui frame, you can submit any command from this point until Render()/EndFrame().
    IMGUI_API void          EndFrame();                                 // ends the Dear ImGui frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
    IMGUI_API void          Render();                                   // ends the Dear ImGui frame, finalize the draw data. You can then get call GetDrawData().
    IMGUI_API ImDrawData*   GetDrawData();                              // valid after Render() and until the next call to NewFrame(). Call ImGui_ImplXXXX_RenderDrawData() function in your Renderer Backend to render.

    IMGUI_API void          ShowDemoWindow(bool* p_open = NULL);        // create Demo window. demonstrate most ImGui features. call this to learn about the library! try to make it always available in your application!
    IMGUI_API void          ShowMetricsWindow(bool* p_open = NULL);     // create Metrics/Debugger window. display Dear ImGui internals: windows, draw commands, various internal state, etc.
    IMGUI_API void          ShowDebugLogWindow(bool* p_open = NULL);    // create Debug Log window. display a simplified log of important dear imgui events.
    IMGUI_API void          ShowIDStackToolWindow(bool* p_open = NULL); // create Stack Tool window. hover items with mouse to query information about the source of their unique ID.
    IMGUI_API void          ShowAboutWindow(bool* p_open = NULL);       // create About window. display Dear ImGui version, credits and build/system information.
    IMGUI_API void          ShowStyleEditor(ImGuiStyle* ref = NULL);    // add style editor block (not a window). you can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it uses the default style)
    IMGUI_API bool          ShowStyleSelector(const char* label);       // add style selector block (not a window), essentially a combo listing the default styles.
    IMGUI_API void          ShowFontSelector(const char* label);        // add font selector block (not a window), essentially a combo listing the loaded fonts.
    IMGUI_API void          ShowUserGuide();                            // add basic help/info block (not a window): how to manipulate ImGui as an end-user (mouse/keyboard controls).
    IMGUI_API const char*   GetVersion();                               // get the compiled version string e.g. "1.80 WIP" (essentially the value for IMGUI_VERSION from the compiled version of imgui.cpp)

    // Styles
    IMGUI_API void          StyleColorsDark(ImGuiStyle* dst = NULL);    // new, recommended style (default)
    IMGUI_API void          StyleColorsLight(ImGuiStyle* dst = NULL);   // best used with borders and a custom, thicker font
    IMGUI_API void          StyleColorsClassic(ImGuiStyle* dst = NULL); // classic imgui style

    // Windows
    IMGUI_API bool          Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
    IMGUI_API void          End();

    //      BeginChild("Name", size, false)   -> Begin("Name", size, 0); or Begin("Name", size, ImGuiChildFlags_None);
    //      BeginChild("Name", size, true)    -> Begin("Name", size, ImGuiChildFlags_Borders);
    IMGUI_API bool          BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    IMGUI_API bool          BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    IMGUI_API void          EndChild();

    IMGUI_API bool          IsWindowAppearing();
    IMGUI_API bool          IsWindowCollapsed();
    IMGUI_API bool          IsWindowFocused(ImGuiFocusedFlags flags=0); // is current window focused? or its root/child, depending on flags. see flags for options.
    IMGUI_API bool          IsWindowHovered(ImGuiHoveredFlags flags=0); // is current window hovered and hoverable (e.g. not blocked by a popup/modal)? See ImGuiHoveredFlags_ for options. IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui or to your underlying app, you should not use this function! Use the 'io.WantCaptureMouse' boolean for that! Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" for details.
    IMGUI_API ImDrawList*   GetWindowDrawList();                        // get draw list associated to the current window, to append your own drawing primitives
    IMGUI_API ImVec2        GetWindowPos();                             // get current window position in screen space (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    IMGUI_API ImVec2        GetWindowSize();                            // get current window size (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    IMGUI_API float         GetWindowWidth();                           // get current window width (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().x.
    IMGUI_API float         GetWindowHeight();                          // get current window height (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().y.

    IMGUI_API void          SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
    IMGUI_API void          SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
    IMGUI_API void          SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use 0.0f or FLT_MAX if you don't want limits. Use -1 for both min and max of same axis to preserve current size (which itself is a constraint). Use callback to apply non-trivial programmatic constraints.
    IMGUI_API void          SetNextWindowContentSize(const ImVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
    IMGUI_API void          SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                 // set next window collapsed state. call before Begin()
    IMGUI_API void          SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
    IMGUI_API void          SetNextWindowScroll(const ImVec2& scroll);                                  // set next window scrolling value (use < 0.0f to not affect a given axis).
    IMGUI_API void          SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
    IMGUI_API void          SetWindowPos(const ImVec2& pos, ImGuiCond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
    IMGUI_API void          SetWindowSize(const ImVec2& size, ImGuiCond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to ImVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
    IMGUI_API void          SetWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
    IMGUI_API void          SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
    IMGUI_API void          SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond = 0);      // set named window position.
    IMGUI_API void          SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
    IMGUI_API void          SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond = 0);   // set named window collapsed state
    IMGUI_API void          SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.

    IMGUI_API float         GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
    IMGUI_API float         GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
    IMGUI_API void          SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
    IMGUI_API void          SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
    IMGUI_API float         GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
    IMGUI_API float         GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
    IMGUI_API void          SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    IMGUI_API void          SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    IMGUI_API void          SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    IMGUI_API void          SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

    IMGUI_API void          PushFont(ImFont* font, float font_size_base_unscaled);          // Use NULL as a shortcut to keep current font. Use 0.0f to keep current size.
    IMGUI_API void          PopFont();
    IMGUI_API ImFont*       GetFont();                                                      // get current font
    IMGUI_API float         GetFontSize();                                                  // get current scaled font size (= height in pixels). AFTER global scale factors applied. *IMPORTANT* DO NOT PASS THIS VALUE TO PushFont()! Use ImGui::GetStyle().FontSizeBase to get value before global scale factors.
    IMGUI_API ImFontBaked*  GetFontBaked();                                                 // get current font bound at current size // == GetFont()->GetFontBaked(GetFontSize())

    IMGUI_API void          PushStyleColor(ImGuiCol idx, ImU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
    IMGUI_API void          PushStyleColor(ImGuiCol idx, const ImVec4& col);
    IMGUI_API void          PopStyleColor(int count = 1);
    IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame()!
    IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);             // modify a style ImVec2 variable. "
    IMGUI_API void          PushStyleVarX(ImGuiStyleVar idx, float val_x);                  // modify X component of a style ImVec2 variable. "
    IMGUI_API void          PushStyleVarY(ImGuiStyleVar idx, float val_y);                  // modify Y component of a style ImVec2 variable. "
    IMGUI_API void          PopStyleVar(int count = 1);
    IMGUI_API void          PushItemFlag(ImGuiItemFlags option, bool enabled);              // modify specified shared item flag, e.g. PushItemFlag(ImGuiItemFlags_NoTabStop, true)
    IMGUI_API void          PopItemFlag();

    IMGUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
    IMGUI_API void          PopItemWidth();
    IMGUI_API void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
    IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
    IMGUI_API void          PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
    IMGUI_API void          PopTextWrapPos();

    IMGUI_API ImVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a white pixel, useful to draw custom shapes via the ImDrawList API
    IMGUI_API ImU32         GetColorU32(ImGuiCol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for ImDrawList
    IMGUI_API ImU32         GetColorU32(const ImVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
    IMGUI_API ImU32         GetColorU32(ImU32 col, float alpha_mul = 1.0f);                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
    IMGUI_API const ImVec4& GetStyleColorVec4(ImGuiCol idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

    IMGUI_API ImVec2        GetCursorScreenPos();                                           // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND (prefer using this rather than GetCursorPos(), also more useful to work with ImDrawList API).
    IMGUI_API void          SetCursorScreenPos(const ImVec2& pos);                          // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND.
    IMGUI_API ImVec2        GetContentRegionAvail();                                        // available space from current position. THIS IS YOUR BEST FRIEND.
    IMGUI_API ImVec2        GetCursorPos();                                                 // [window-local] cursor position in window-local coordinates. This is not your best friend.
    IMGUI_API float         GetCursorPosX();                                                // [window-local] "
    IMGUI_API float         GetCursorPosY();                                                // [window-local] "
    IMGUI_API void          SetCursorPos(const ImVec2& local_pos);                          // [window-local] "
    IMGUI_API void          SetCursorPosX(float local_x);                                   // [window-local] "
    IMGUI_API void          SetCursorPosY(float local_y);                                   // [window-local] "
    IMGUI_API ImVec2        GetCursorStartPos();                                            // [window-local] initial cursor position, in window-local coordinates. Call GetCursorScreenPos() after Begin() to get the absolute coordinates version.

    IMGUI_API void          Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
    IMGUI_API void          SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
    IMGUI_API void          NewLine();                                                      // undo a SameLine() or force a new line when in a horizontal-layout context.
    IMGUI_API void          Spacing();                                                      // add vertical spacing.
    IMGUI_API void          Dummy(const ImVec2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
    IMGUI_API void          Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
    IMGUI_API void          Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
    IMGUI_API void          BeginGroup();                                                   // lock horizontal starting position
    IMGUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
    IMGUI_API void          AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
    IMGUI_API float         GetTextLineHeight();                                            // ~ FontSize
    IMGUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

    IMGUI_API void          PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
    IMGUI_API void          PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
    IMGUI_API void          PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
    IMGUI_API void          PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
    IMGUI_API void          PopID();                                                        // pop from the ID stack.
    IMGUI_API ImGuiID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into ImGuiStorage yourself
    IMGUI_API ImGuiID       GetID(const char* str_id_begin, const char* str_id_end);
    IMGUI_API ImGuiID       GetID(const void* ptr_id);
    IMGUI_API ImGuiID       GetID(int int_id);

    IMGUI_API void          TextUnformatted(const char* text, const char* text_end = NULL); // raw text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
    IMGUI_API void          Text(const char* fmt, ...)                                      IM_FMTARGS(1); // formatted text
    IMGUI_API void          TextV(const char* fmt, va_list args)                            IM_FMTLIST(1);
    IMGUI_API void          TextColored(const ImVec4& col, const char* fmt, ...)            IM_FMTARGS(2); // shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
    IMGUI_API void          TextColoredV(const ImVec4& col, const char* fmt, va_list args)  IM_FMTLIST(2);
    IMGUI_API void          TextDisabled(const char* fmt, ...)                              IM_FMTARGS(1); // shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
    IMGUI_API void          TextDisabledV(const char* fmt, va_list args)                    IM_FMTLIST(1);
    IMGUI_API void          TextWrapped(const char* fmt, ...)                               IM_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
    IMGUI_API void          TextWrappedV(const char* fmt, va_list args)                     IM_FMTLIST(1);
    IMGUI_API void          LabelText(const char* label, const char* fmt, ...)              IM_FMTARGS(2); // display text+label aligned the same way as value+label widgets
    IMGUI_API void          LabelTextV(const char* label, const char* fmt, va_list args)    IM_FMTLIST(2);
    IMGUI_API void          BulletText(const char* fmt, ...)                                IM_FMTARGS(1); // shortcut for Bullet()+Text()
    IMGUI_API void          BulletTextV(const char* fmt, va_list args)                      IM_FMTLIST(1);
    IMGUI_API void          SeparatorText(const char* label);                               // currently: formatted text with a horizontal line

    IMGUI_API bool          Button(const char* label, const ImVec2& size = ImVec2(0, 0));   // button
    IMGUI_API bool          SmallButton(const char* label);                                 // button with (FramePadding.y == 0) to easily embed within text
    IMGUI_API bool          InvisibleButton(const char* str_id, const ImVec2& size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
    IMGUI_API bool          ArrowButton(const char* str_id, ImGuiDir dir);                  // square button with an arrow shape
    IMGUI_API bool          Checkbox(const char* label, bool* v);
    IMGUI_API bool          CheckboxFlags(const char* label, int* flags, int flags_value);
    IMGUI_API bool          CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
    IMGUI_API bool          RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
    IMGUI_API bool          RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
    IMGUI_API void          ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL);
    IMGUI_API void          Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
    IMGUI_API bool          TextLink(const char* label);                                    // hyperlink text button, return true when clicked
    IMGUI_API bool          TextLinkOpenURL(const char* label, const char* url = NULL);     // hyperlink text button, automatically open file/url when clicked

    IMGUI_API void          Image(ImTextureRef tex_ref, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
    IMGUI_API void          ImageWithBg(ImTextureRef tex_ref, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
    IMGUI_API bool          ImageButton(const char* str_id, ImTextureRef tex_ref, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

    IMGUI_API bool          BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
    IMGUI_API void          EndCombo(); // only call EndCombo() if BeginCombo() returns true!
    IMGUI_API bool          Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    IMGUI_API bool          Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
    IMGUI_API bool          Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items = -1);

    IMGUI_API bool          DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // If v_min >= v_max we have no bound
    IMGUI_API bool          DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);  // If v_min >= v_max we have no bound
    IMGUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool          DragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);

    IMGUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    IMGUI_API bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool          SliderScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool          VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool          VSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);

    IMGUI_API bool          InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool          InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool          InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputFloat4(const char* label, float v[4], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
    IMGUI_API bool          InputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);

    IMGUI_API bool          ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    IMGUI_API bool          ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0);
    IMGUI_API bool          ColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    IMGUI_API bool          ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
    IMGUI_API bool          ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
    IMGUI_API void          SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

    IMGUI_API bool          TreeNode(const char* label);
    IMGUI_API bool          TreeNode(const char* str_id, const char* fmt, ...) IM_FMTARGS(2);   // helper variation to easily decorrelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
    IMGUI_API bool          TreeNode(const void* ptr_id, const char* fmt, ...) IM_FMTARGS(2);   // "
    IMGUI_API bool          TreeNodeV(const char* str_id, const char* fmt, va_list args) IM_FMTLIST(2);
    IMGUI_API bool          TreeNodeV(const void* ptr_id, const char* fmt, va_list args) IM_FMTLIST(2);
    IMGUI_API bool          TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0);
    IMGUI_API bool          TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(3);
    IMGUI_API bool          TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(3);
    IMGUI_API bool          TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
    IMGUI_API bool          TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
    IMGUI_API void          TreePush(const char* str_id);                                       // ~ Indent()+PushID(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
    IMGUI_API void          TreePush(const void* ptr_id);                                       // "
    IMGUI_API void          TreePop();                                                          // ~ Unindent()+PopID()
    IMGUI_API float         GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
    IMGUI_API bool          CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
    IMGUI_API bool          CollapsingHeader(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
    IMGUI_API void          SetNextItemOpen(bool is_open, ImGuiCond cond = 0);                  // set next TreeNode/CollapsingHeader open state.
    IMGUI_API void          SetNextItemStorageID(ImGuiID storage_id);                           // set id to use for open/close storage (default to same as item id).

    IMGUI_API bool          Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
    IMGUI_API bool          Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

    IMGUI_API ImGuiMultiSelectIO*   BeginMultiSelect(ImGuiMultiSelectFlags flags, int selection_size = -1, int items_count = -1);
    IMGUI_API ImGuiMultiSelectIO*   EndMultiSelect();
    IMGUI_API void                  SetNextItemSelectionUserData(ImGuiSelectionUserData selection_user_data);
    IMGUI_API bool                  IsItemToggledSelection();                                   // Was the last item selection state toggled? Useful if you need the per-item information _before_ reaching EndMultiSelect(). We only returns toggle _event_ in order to handle clipping correctly.

    IMGUI_API bool          BeginListBox(const char* label, const ImVec2& size = ImVec2(0, 0)); // open a framed scrolling region
    IMGUI_API void          EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
    IMGUI_API bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
    IMGUI_API bool          ListBox(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items = -1);

    IMGUI_API void          PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
    IMGUI_API void          PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
    IMGUI_API void          PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
    IMGUI_API void          PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));

    IMGUI_API void          Value(const char* prefix, bool b);
    IMGUI_API void          Value(const char* prefix, int v);
    IMGUI_API void          Value(const char* prefix, unsigned int v);
    IMGUI_API void          Value(const char* prefix, float v, const char* float_format = NULL);

    IMGUI_API bool          BeginMenuBar();                                                     // append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window).
    IMGUI_API void          EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
    IMGUI_API bool          BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
    IMGUI_API void          EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
    IMGUI_API bool          BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
    IMGUI_API void          EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
    IMGUI_API bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
    IMGUI_API bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL

    // Tooltips
    IMGUI_API bool          BeginTooltip();                                                     // begin/append a tooltip window.
    IMGUI_API void          EndTooltip();                                                       // only call EndTooltip() if BeginTooltip()/BeginItemTooltip() returns true!
    IMGUI_API void          SetTooltip(const char* fmt, ...) IM_FMTARGS(1);                     // set a text-only tooltip. Often used after a ImGui::IsItemHovered() check. Override any previous call to SetTooltip().
    IMGUI_API void          SetTooltipV(const char* fmt, va_list args) IM_FMTLIST(1);

    IMGUI_API bool          BeginItemTooltip();                                                 // begin/append a tooltip window if preceding item was hovered.
    IMGUI_API void          SetItemTooltip(const char* fmt, ...) IM_FMTARGS(1);                 // set a text-only tooltip if preceding item was hovered. override any previous call to SetTooltip().
    IMGUI_API void          SetItemTooltipV(const char* fmt, va_list args) IM_FMTLIST(1);

    IMGUI_API bool          BeginPopup(const char* str_id, ImGuiWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
    IMGUI_API bool          BeginPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
    IMGUI_API void          EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!

    IMGUI_API void          OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
    IMGUI_API void          OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags = 0);                             // id overload to facilitate calling from nested stacks
    IMGUI_API void          OpenPopupOnItemClick(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);   // helper to open popup when clicked on last item. Default to ImGuiPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
    IMGUI_API void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.

    IMGUI_API bool          BeginPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
    IMGUI_API bool          BeginPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);// open+begin popup when clicked on current window.
    IMGUI_API bool          BeginPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).

    IMGUI_API bool          IsPopupOpen(const char* str_id, ImGuiPopupFlags flags = 0);                         // return true if the popup is open.

    // Tables
    IMGUI_API bool          BeginTable(const char* str_id, int columns, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
    IMGUI_API void          EndTable();                                         // only call EndTable() if BeginTable() returns true!
    IMGUI_API void          TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.
    IMGUI_API bool          TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
    IMGUI_API bool          TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.

    IMGUI_API void          TableSetupColumn(const char* label, ImGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, ImGuiID user_id = 0);
    IMGUI_API void          TableSetupScrollFreeze(int cols, int rows);         // lock columns/rows so they stay visible when scrolled.
    IMGUI_API void          TableHeader(const char* label);                     // submit one header cell manually (rarely used)
    IMGUI_API void          TableHeadersRow();                                  // submit a row with headers cells based on data provided to TableSetupColumn() + submit context menu
    IMGUI_API void          TableAngledHeadersRow();                            // submit a row with angled headers for every column with the ImGuiTableColumnFlags_AngledHeader flag. MUST BE FIRST ROW.

    IMGUI_API ImGuiTableSortSpecs*  TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).  Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
    IMGUI_API int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
    IMGUI_API int                   TableGetColumnIndex();                      // return current column index.
    IMGUI_API int                   TableGetRowIndex();                         // return current row index (header rows are accounted for)
    IMGUI_API const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
    IMGUI_API ImGuiTableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
    IMGUI_API void                  TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with ImGuiTableFlags_ContextMenuInBody)
    IMGUI_API int                   TableGetHoveredColumn();                    // return hovered column. return -1 when table is not hovered. return columns_count if the unused space at the right of visible columns is hovered. Can also use (TableGetColumnFlags() & ImGuiTableColumnFlags_IsHovered) instead.
    IMGUI_API void                  TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);  // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.

    IMGUI_API void          Columns(int count = 1, const char* id = NULL, bool borders = true);
    IMGUI_API void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
    IMGUI_API int           GetColumnIndex();                                                   // get current column index
    IMGUI_API float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
    IMGUI_API void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
    IMGUI_API float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
    IMGUI_API void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
    IMGUI_API int           GetColumnsCount();

    IMGUI_API bool          BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);        // create and append into a TabBar
    IMGUI_API void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
    IMGUI_API bool          BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
    IMGUI_API void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
    IMGUI_API bool          TabItemButton(const char* label, ImGuiTabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
    IMGUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.

    // Logging/Capture
    IMGUI_API void          LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
    IMGUI_API void          LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
    IMGUI_API void          LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
    IMGUI_API void          LogFinish();                                                        // stop logging (close file, etc.)
    IMGUI_API void          LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
    IMGUI_API void          LogText(const char* fmt, ...) IM_FMTARGS(1);                        // pass text data straight to log (without being displayed)
    IMGUI_API void          LogTextV(const char* fmt, va_list args) IM_FMTLIST(1);

    IMGUI_API bool          BeginDragDropSource(ImGuiDragDropFlags flags = 0);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
    IMGUI_API bool          SetDragDropPayload(const char* type, const void* data, size_t sz, ImGuiCond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui. Return true when payload has been accepted.
    IMGUI_API void          EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
    IMGUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    IMGUI_API const ImGuiPayload*   AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    IMGUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    IMGUI_API const ImGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. returns NULL when drag and drop is finished or inactive. use ImGuiPayload::IsDataType() to test for the payload type.

    IMGUI_API void          BeginDisabled(bool disabled = true);
    IMGUI_API void          EndDisabled();

    // Clipping
    IMGUI_API void          PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
    IMGUI_API void          PopClipRect();

    IMGUI_API void          SetItemDefaultFocus();                                              // make last item the default focused item of a newly appearing window.
    IMGUI_API void          SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

    IMGUI_API void          SetNavCursorVisible(bool visible);                                  // alter visibility of keyboard/gamepad cursor. by default: show when using an arrow key, hide when clicking with mouse.

    IMGUI_API void          SetNextItemAllowOverlap();                                          // allow next item to be overlapped by a subsequent item. Useful with invisible buttons, selectable, treenode covering an area where subsequent items may need to be added. Note that both Selectable() and TreeNode() have dedicated flags doing this.

    IMGUI_API bool          IsItemHovered(ImGuiHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.
    IMGUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
    IMGUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
    IMGUI_API bool          IsItemClicked(ImGuiMouseButton mouse_button = 0);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
    IMGUI_API bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
    IMGUI_API bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
    IMGUI_API bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
    IMGUI_API bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that require continuous editing.
    IMGUI_API bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
    IMGUI_API bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
    IMGUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
    IMGUI_API bool          IsAnyItemActive();                                                  // is any item active?
    IMGUI_API bool          IsAnyItemFocused();                                                 // is any item focused?
    IMGUI_API ImGuiID       GetItemID();                                                        // get ID of last item (~~ often same ImGui::GetID(label) beforehand)
    IMGUI_API ImVec2        GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
    IMGUI_API ImVec2        GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
    IMGUI_API ImVec2        GetItemRectSize();                                                  // get size of last item

    // Viewports
    IMGUI_API ImGuiViewport* GetMainViewport();                                                 // return primary/default viewport. This can never be NULL.

    IMGUI_API ImDrawList*   GetBackgroundDrawList();                                            // this draw list will be the first rendered one. Useful to quickly draw shapes/text behind dear imgui contents.
    IMGUI_API ImDrawList*   GetForegroundDrawList();                                            // this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.

    IMGUI_API bool          IsRectVisible(const ImVec2& size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
    IMGUI_API bool          IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
    IMGUI_API double        GetTime();                                                          // get global imgui time. incremented by io.DeltaTime every frame.
    IMGUI_API int           GetFrameCount();                                                    // get global imgui frame count. incremented by 1 every frame.
    IMGUI_API ImDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own ImDrawList instances.
    IMGUI_API const char*   GetStyleColorName(ImGuiCol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
    IMGUI_API void          SetStateStorage(ImGuiStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
    IMGUI_API ImGuiStorage* GetStateStorage();

    IMGUI_API ImVec2        CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);

    IMGUI_API ImVec4        ColorConvertU32ToFloat4(ImU32 in);
    IMGUI_API ImU32         ColorConvertFloat4ToU32(const ImVec4& in);
    IMGUI_API void          ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
    IMGUI_API void          ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

    IMGUI_API bool          IsKeyDown(ImGuiKey key);                                            // is key being held.
    IMGUI_API bool          IsKeyPressed(ImGuiKey key, bool repeat = true);                     // was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
    IMGUI_API bool          IsKeyReleased(ImGuiKey key);                                        // was key released (went from Down to !Down)?
    IMGUI_API bool          IsKeyChordPressed(ImGuiKeyChord key_chord);                         // was key chord (mods + key) pressed, e.g. you can pass 'ImGuiMod_Ctrl | ImGuiKey_S' as a key-chord. This doesn't do any routing or focus check, please consider using Shortcut() function instead.
    IMGUI_API int           GetKeyPressedAmount(ImGuiKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
    IMGUI_API const char*   GetKeyName(ImGuiKey key);                                           // [DEBUG] returns English name of the key. Those names are provided for debugging purpose and are not meant to be saved persistently nor compared.
    IMGUI_API void          SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard);        // Override io.WantCaptureKeyboard flag next frame (said flag is left for your application to handle, typically when true it instructs your app to ignore inputs). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard"; after the next NewFrame() call.

    IMGUI_API bool          Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags = 0);
    IMGUI_API void          SetNextItemShortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags = 0);

    IMGUI_API void          SetItemKeyOwner(ImGuiKey key);                                      // Set key owner to last item ID if it is hovered or active. Equivalent to 'if (IsItemHovered() || IsItemActive()) { SetKeyOwner(key, GetItemID());'.

    IMGUI_API bool          IsMouseDown(ImGuiMouseButton button);                               // is mouse button held?
    IMGUI_API bool          IsMouseClicked(ImGuiMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    IMGUI_API bool          IsMouseReleased(ImGuiMouseButton button);                           // did mouse button released? (went from Down to !Down)
    IMGUI_API bool          IsMouseDoubleClicked(ImGuiMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    IMGUI_API bool          IsMouseReleasedWithDelay(ImGuiMouseButton button, float delay);     // delayed mouse release (use very sparingly!). Generally used with 'delay >= io.MouseDoubleClickTime' + combined with a 'io.MouseClickedLastCount==1' test. This is a very rarely used UI idiom, but some apps use this: e.g. MS Explorer single click on an icon to rename.
    IMGUI_API int           GetMouseClickedCount(ImGuiMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    IMGUI_API bool          IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
    IMGUI_API bool          IsMousePosValid(const ImVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    IMGUI_API bool          IsAnyMouseDown();                                                   // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
    IMGUI_API ImVec2        GetMousePos();                                                      // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
    IMGUI_API ImVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
    IMGUI_API bool          IsMouseDragging(ImGuiMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    IMGUI_API ImVec2        GetMouseDragDelta(ImGuiMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    IMGUI_API void          ResetMouseDragDelta(ImGuiMouseButton button = 0);                   //
    IMGUI_API ImGuiMouseCursor GetMouseCursor();                                                // get desired mouse cursor shape. Important: reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
    IMGUI_API void          SetMouseCursor(ImGuiMouseCursor cursor_type);                       // set desired mouse cursor shape
    IMGUI_API void          SetNextFrameWantCaptureMouse(bool want_capture_mouse);              // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instructs your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

    IMGUI_API const char*   GetClipboardText();
    IMGUI_API void          SetClipboardText(const char* text);

    IMGUI_API void          LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
    IMGUI_API void          LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
    IMGUI_API void          SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
    IMGUI_API const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

    IMGUI_API void          DebugTextEncoding(const char* text);
    IMGUI_API void          DebugFlashStyleColor(ImGuiCol idx);
    IMGUI_API void          DebugStartItemPicker();
    IMGUI_API bool          DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by IMGUI_CHECKVERSION() macro.
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    IMGUI_API void          DebugLog(const char* fmt, ...)           IM_FMTARGS(1); // Call via IMGUI_DEBUG_LOG() for maximum stripping in caller code!
    IMGUI_API void          DebugLogV(const char* fmt, va_list args) IM_FMTLIST(1);
#endif

    IMGUI_API void          SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data = NULL);
    IMGUI_API void          GetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data);
    IMGUI_API void*         MemAlloc(size_t size);
    IMGUI_API void          MemFree(void* ptr);

} // namespace ImGui

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum ImGuiWindowFlags_
{
    ImGuiWindowFlags_None                   = 0,
    ImGuiWindowFlags_NoTitleBar             = 1 << 0,   // Disable title-bar
    ImGuiWindowFlags_NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
    ImGuiWindowFlags_NoMove                 = 1 << 2,   // Disable user moving the window
    ImGuiWindowFlags_NoScrollbar            = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    ImGuiWindowFlags_NoScrollWithMouse      = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    ImGuiWindowFlags_NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    ImGuiWindowFlags_AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
    ImGuiWindowFlags_NoBackground           = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    ImGuiWindowFlags_NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
    ImGuiWindowFlags_NoMouseInputs          = 1 << 9,   // Disable catching mouse, hovering test with pass through.
    ImGuiWindowFlags_MenuBar                = 1 << 10,  // Has a menu-bar
    ImGuiWindowFlags_HorizontalScrollbar    = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    ImGuiWindowFlags_NoFocusOnAppearing     = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
    ImGuiWindowFlags_NoBringToFrontOnFocus  = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    ImGuiWindowFlags_AlwaysVerticalScrollbar= 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    ImGuiWindowFlags_AlwaysHorizontalScrollbar=1<< 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    ImGuiWindowFlags_NoNavInputs            = 1 << 16,  // No keyboard/gamepad navigation within the window
    ImGuiWindowFlags_NoNavFocus             = 1 << 17,  // No focusing toward this window with keyboard/gamepad navigation (e.g. skipped by Ctrl+Tab)
    ImGuiWindowFlags_UnsavedDocument        = 1 << 18,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    ImGuiWindowFlags_NoNav                  = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    ImGuiWindowFlags_NoDecoration           = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
    ImGuiWindowFlags_NoInputs               = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,

    // [Internal]
    ImGuiWindowFlags_ChildWindow            = 1 << 24,  // Don't use! For internal use by BeginChild()
    ImGuiWindowFlags_Tooltip                = 1 << 25,  // Don't use! For internal use by BeginTooltip()
    ImGuiWindowFlags_Popup                  = 1 << 26,  // Don't use! For internal use by BeginPopup()
    ImGuiWindowFlags_Modal                  = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
    ImGuiWindowFlags_ChildMenu              = 1 << 28,  // Don't use! For internal use by BeginMenu()

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
};

enum ImGuiChildFlags_
{
    ImGuiChildFlags_None                    = 0,
    ImGuiChildFlags_Borders                 = 1 << 0,   // Show an outer border and enable WindowPadding. (IMPORTANT: this is always == 1 == true for legacy reason)
    ImGuiChildFlags_AlwaysUseWindowPadding  = 1 << 1,   // Pad with style.WindowPadding even if no border are drawn (no padding by default for non-bordered child windows because it makes more sense)
    ImGuiChildFlags_ResizeX                 = 1 << 2,   // Allow resize from right border (layout direction). Enable .ini saving (unless ImGuiWindowFlags_NoSavedSettings passed to window flags)
    ImGuiChildFlags_ResizeY                 = 1 << 3,   // Allow resize from bottom border (layout direction). "
    ImGuiChildFlags_AutoResizeX             = 1 << 4,   // Enable auto-resizing width. Read "IMPORTANT: Size measurement" details above.
    ImGuiChildFlags_AutoResizeY             = 1 << 5,   // Enable auto-resizing height. Read "IMPORTANT: Size measurement" details above.
    ImGuiChildFlags_AlwaysAutoResize        = 1 << 6,   // Combined with AutoResizeX/AutoResizeY. Always measure size even when child is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.
    ImGuiChildFlags_FrameStyle              = 1 << 7,   // Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
    ImGuiChildFlags_NavFlattened            = 1 << 8,   // [BETA] Share focus scope, allow keyboard/gamepad navigation to cross over parent border to this child or between sibling child windows.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
};

enum ImGuiItemFlags_
{
    ImGuiItemFlags_None                     = 0,        // (Default)
    ImGuiItemFlags_NoTabStop                = 1 << 0,   // false    // Disable keyboard tabbing. This is a "lighter" version of ImGuiItemFlags_NoNav.
    ImGuiItemFlags_NoNav                    = 1 << 1,   // false    // Disable any form of focusing (keyboard/gamepad directional navigation and SetKeyboardFocusHere() calls).
    ImGuiItemFlags_NoNavDefaultFocus        = 1 << 2,   // false    // Disable item being a candidate for default focus (e.g. used by title bar items).
    ImGuiItemFlags_ButtonRepeat             = 1 << 3,   // false    // Any button-like behavior will have repeat mode enabled (based on io.KeyRepeatDelay and io.KeyRepeatRate values). Note that you can also call IsItemActive() after any button to tell if it is being held.
    ImGuiItemFlags_AutoClosePopups          = 1 << 4,   // true     // MenuItem()/Selectable() automatically close their parent popup window.
    ImGuiItemFlags_AllowDuplicateId         = 1 << 5,   // false    // Allow submitting an item with the same identifier as an item already submitted this frame without triggering a warning tooltip if io.ConfigDebugHighlightIdConflicts is set.
};

enum ImGuiInputTextFlags_
{
    ImGuiInputTextFlags_None                = 0,
    ImGuiInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    ImGuiInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    ImGuiInputTextFlags_CharsScientific     = 1 << 2,   // Allow 0123456789.+-*/eE (Scientific notation input)
    ImGuiInputTextFlags_CharsUppercase      = 1 << 3,   // Turn a..z into A..Z
    ImGuiInputTextFlags_CharsNoBlank        = 1 << 4,   // Filter out spaces, tabs

    // Inputs
    ImGuiInputTextFlags_AllowTabInput       = 1 << 5,   // Pressing TAB input a '\t' character into the text field
    ImGuiInputTextFlags_EnterReturnsTrue    = 1 << 6,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider using IsItemDeactivatedAfterEdit() instead!
    ImGuiInputTextFlags_EscapeClearsAll     = 1 << 7,   // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)
    ImGuiInputTextFlags_CtrlEnterForNewLine = 1 << 8,   // In multi-line mode, validate with Enter, add new line with Ctrl+Enter (default is opposite: validate with Ctrl+Enter, add line with Enter).

    ImGuiInputTextFlags_ReadOnly            = 1 << 9,   // Read-only mode
    ImGuiInputTextFlags_Password            = 1 << 10,  // Password mode, display all characters as '*', disable copy
    ImGuiInputTextFlags_AlwaysOverwrite     = 1 << 11,  // Overwrite mode
    ImGuiInputTextFlags_AutoSelectAll       = 1 << 12,  // Select entire text when first taking mouse focus
    ImGuiInputTextFlags_ParseEmptyRefVal    = 1 << 13,  // InputFloat(), InputInt(), InputScalar() etc. only: parse empty string as zero value.
    ImGuiInputTextFlags_DisplayEmptyRefVal  = 1 << 14,  // InputFloat(), InputInt(), InputScalar() etc. only: when value is zero, do not display it. Generally used with ImGuiInputTextFlags_ParseEmptyRefVal.
    ImGuiInputTextFlags_NoHorizontalScroll  = 1 << 15,  // Disable following the cursor horizontally
    ImGuiInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().

    ImGuiInputTextFlags_ElideLeft           = 1 << 17,  // When text doesn't fit, elide left side to ensure right side stays visible. Useful for path/filenames. Single-line only!

    ImGuiInputTextFlags_CallbackCompletion  = 1 << 18,  // Callback on pressing TAB (for completion handling)
    ImGuiInputTextFlags_CallbackHistory     = 1 << 19,  // Callback on pressing Up/Down arrows (for history handling)
    ImGuiInputTextFlags_CallbackAlways      = 1 << 20,  // Callback on each iteration. User code may query cursor position, modify text buffer.
    ImGuiInputTextFlags_CallbackCharFilter  = 1 << 21,  // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    ImGuiInputTextFlags_CallbackResize      = 1 << 22,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
    ImGuiInputTextFlags_CallbackEdit        = 1 << 23,  // Callback on any edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.

    ImGuiInputTextFlags_WordWrap            = 1 << 24,  // InputTextMultiline(): word-wrap lines that are too long.

};

enum ImGuiTreeNodeFlags_
{
    ImGuiTreeNodeFlags_None                 = 0,
    ImGuiTreeNodeFlags_Selected             = 1 << 0,   // Draw as selected
    ImGuiTreeNodeFlags_Framed               = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
    ImGuiTreeNodeFlags_AllowOverlap         = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
    ImGuiTreeNodeFlags_NoTreePushOnOpen     = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    ImGuiTreeNodeFlags_NoAutoOpenOnLog      = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    ImGuiTreeNodeFlags_DefaultOpen          = 1 << 5,   // Default node to be open
    ImGuiTreeNodeFlags_OpenOnDoubleClick    = 1 << 6,   // Open on double-click instead of simple click (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    ImGuiTreeNodeFlags_OpenOnArrow          = 1 << 7,   // Open when clicking on the arrow part (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    ImGuiTreeNodeFlags_Leaf                 = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes).
    ImGuiTreeNodeFlags_Bullet               = 1 << 9,   // Display a bullet instead of arrow. IMPORTANT: node can still be marked open/close if you don't set the _Leaf flag!
    ImGuiTreeNodeFlags_FramePadding         = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding() before the node.
    ImGuiTreeNodeFlags_SpanAvailWidth       = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line without using AllowOverlap mode.
    ImGuiTreeNodeFlags_SpanFullWidth        = 1 << 12,  // Extend hit box to the left-most and right-most edges (cover the indent area).
    ImGuiTreeNodeFlags_SpanLabelWidth       = 1 << 13,  // Narrow hit box + narrow hovering highlight, will only cover the label text.
    ImGuiTreeNodeFlags_SpanAllColumns       = 1 << 14,  // Frame will span all columns of its container table (label will still fit in current column)
    ImGuiTreeNodeFlags_LabelSpanAllColumns  = 1 << 15,  // Label will span all columns of its container table
    ImGuiTreeNodeFlags_NavLeftJumpsToParent = 1 << 17,  // Nav: left arrow moves back to parent. This is processed in TreePop() when there's an unfulfilled Left nav request remaining.
    ImGuiTreeNodeFlags_CollapsingHeader     = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog,

    ImGuiTreeNodeFlags_DrawLinesNone        = 1 << 18,  // No lines drawn
    ImGuiTreeNodeFlags_DrawLinesFull        = 1 << 19,  // Horizontal lines to child nodes. Vertical line drawn down to TreePop() position: cover full contents. Faster (for large trees).
    ImGuiTreeNodeFlags_DrawLinesToNodes     = 1 << 20,  // Horizontal lines to child nodes. Vertical line drawn down to bottom-most child node. Slower (for large trees).

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiTreeNodeFlags_NavLeftJumpsBackHere = ImGuiTreeNodeFlags_NavLeftJumpsToParent,  // Renamed in 1.92.0
    ImGuiTreeNodeFlags_SpanTextWidth        = ImGuiTreeNodeFlags_SpanLabelWidth,        // Renamed in 1.90.7
#endif
};

enum ImGuiPopupFlags_
{
    ImGuiPopupFlags_None                    = 0,
    ImGuiPopupFlags_MouseButtonLeft         = 0,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGuiMouseButton_Left)
    ImGuiPopupFlags_MouseButtonRight        = 1,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGuiMouseButton_Right)
    ImGuiPopupFlags_MouseButtonMiddle       = 2,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGuiMouseButton_Middle)
    ImGuiPopupFlags_MouseButtonMask_        = 0x1F,
    ImGuiPopupFlags_MouseButtonDefault_     = 1,
    ImGuiPopupFlags_NoReopen                = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't reopen same popup if already open (won't reposition, won't reinitialize navigation)
    ImGuiPopupFlags_NoOpenOverExistingPopup = 1 << 7,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    ImGuiPopupFlags_NoOpenOverItems         = 1 << 8,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    ImGuiPopupFlags_AnyPopupId              = 1 << 10,  // For IsPopupOpen(): ignore the ImGuiID parameter and test for any popup.
    ImGuiPopupFlags_AnyPopupLevel           = 1 << 11,  // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    ImGuiPopupFlags_AnyPopup                = ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel,
};

enum ImGuiSelectableFlags_
{
    ImGuiSelectableFlags_None               = 0,
    ImGuiSelectableFlags_NoAutoClosePopups  = 1 << 0,   // Clicking this doesn't close parent popup window (overrides ImGuiItemFlags_AutoClosePopups)
    ImGuiSelectableFlags_SpanAllColumns     = 1 << 1,   // Frame will span all columns of its container table (text will still fit in current column)
    ImGuiSelectableFlags_AllowDoubleClick   = 1 << 2,   // Generate press events on double clicks too
    ImGuiSelectableFlags_Disabled           = 1 << 3,   // Cannot be selected, display grayed out text
    ImGuiSelectableFlags_AllowOverlap       = 1 << 4,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
    ImGuiSelectableFlags_Highlight          = 1 << 5,   // Make the item be displayed as if it is hovered
    ImGuiSelectableFlags_SelectOnNav        = 1 << 6,   // Auto-select when moved into, unless Ctrl is held. Automatic when in a BeginMultiSelect() block.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiSelectableFlags_DontClosePopups    = ImGuiSelectableFlags_NoAutoClosePopups,   // Renamed in 1.91.0
#endif
};

enum ImGuiComboFlags_
{
    ImGuiComboFlags_None                    = 0,
    ImGuiComboFlags_PopupAlignLeft          = 1 << 0,   // Align the popup toward the left by default
    ImGuiComboFlags_HeightSmall             = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
    ImGuiComboFlags_HeightRegular           = 1 << 2,   // Max ~8 items visible (default)
    ImGuiComboFlags_HeightLarge             = 1 << 3,   // Max ~20 items visible
    ImGuiComboFlags_HeightLargest           = 1 << 4,   // As many fitting items as possible
    ImGuiComboFlags_NoArrowButton           = 1 << 5,   // Display on the preview box without the square arrow button
    ImGuiComboFlags_NoPreview               = 1 << 6,   // Display only a square arrow button
    ImGuiComboFlags_WidthFitPreview         = 1 << 7,   // Width dynamically calculated from preview contents
    ImGuiComboFlags_HeightMask_             = ImGuiComboFlags_HeightSmall | ImGuiComboFlags_HeightRegular | ImGuiComboFlags_HeightLarge | ImGuiComboFlags_HeightLargest,
};

enum ImGuiTabBarFlags_
{
    ImGuiTabBarFlags_None                           = 0,
    ImGuiTabBarFlags_Reorderable                    = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
    ImGuiTabBarFlags_AutoSelectNewTabs              = 1 << 1,   // Automatically select new tabs when they appear
    ImGuiTabBarFlags_TabListPopupButton             = 1 << 2,   // Disable buttons to open the tab list popup
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton   = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    ImGuiTabBarFlags_NoTabListScrollingButtons      = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is ImGuiTabBarFlags_FittingPolicyScroll)
    ImGuiTabBarFlags_NoTooltip                      = 1 << 5,   // Disable tooltips when hovering a tab
    ImGuiTabBarFlags_DrawSelectedOverline           = 1 << 6,   // Draw selected overline markers over selected tab

    ImGuiTabBarFlags_FittingPolicyMixed             = 1 << 7,   // Shrink down tabs when they don't fit, until width is style.TabMinWidthShrink, then enable scrolling buttons.
    ImGuiTabBarFlags_FittingPolicyShrink            = 1 << 8,   // Shrink down tabs when they don't fit
    ImGuiTabBarFlags_FittingPolicyScroll            = 1 << 9,   // Enable scrolling buttons when tabs don't fit
    ImGuiTabBarFlags_FittingPolicyMask_             = ImGuiTabBarFlags_FittingPolicyMixed | ImGuiTabBarFlags_FittingPolicyShrink | ImGuiTabBarFlags_FittingPolicyScroll,
    ImGuiTabBarFlags_FittingPolicyDefault_          = ImGuiTabBarFlags_FittingPolicyMixed,

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiTabBarFlags_FittingPolicyResizeDown        = ImGuiTabBarFlags_FittingPolicyShrink, // Renamed in 1.92.2
#endif
};

enum ImGuiTabItemFlags_
{
    ImGuiTabItemFlags_None                          = 0,
    ImGuiTabItemFlags_UnsavedDocument               = 1 << 0,   // Display a dot next to the title + set ImGuiTabItemFlags_NoAssumedClosure.
    ImGuiTabItemFlags_SetSelected                   = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    ImGuiTabItemFlags_NoCloseWithMiddleMouseButton  = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    ImGuiTabItemFlags_NoPushId                      = 1 << 3,   // Don't call PushID()/PopID() on BeginTabItem()/EndTabItem()
    ImGuiTabItemFlags_NoTooltip                     = 1 << 4,   // Disable tooltip for the given tab
    ImGuiTabItemFlags_NoReorder                     = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
    ImGuiTabItemFlags_Leading                       = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    ImGuiTabItemFlags_Trailing                      = 1 << 7,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
    ImGuiTabItemFlags_NoAssumedClosure              = 1 << 8,   // Tab is selected when trying to close + closure is not immediately assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
};

enum ImGuiFocusedFlags_
{
    ImGuiFocusedFlags_None                          = 0,
    ImGuiFocusedFlags_ChildWindows                  = 1 << 0,   // Return true if any children of the window is focused
    ImGuiFocusedFlags_RootWindow                    = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
    ImGuiFocusedFlags_AnyWindow                     = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
    ImGuiFocusedFlags_NoPopupHierarchy              = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    ImGuiFocusedFlags_RootAndChildWindows           = ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows,
};

enum ImGuiHoveredFlags_
{
    ImGuiHoveredFlags_None                          = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    ImGuiHoveredFlags_ChildWindows                  = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
    ImGuiHoveredFlags_RootWindow                    = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    ImGuiHoveredFlags_AnyWindow                     = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
    ImGuiHoveredFlags_NoPopupHierarchy              = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    ImGuiHoveredFlags_AllowWhenBlockedByPopup       = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
    ImGuiHoveredFlags_AllowWhenBlockedByActiveItem  = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    ImGuiHoveredFlags_AllowWhenOverlappedByItem     = 1 << 8,   // IsItemHovered() only: Return true even if the item uses AllowOverlap mode and is overlapped by another hoverable item.
    ImGuiHoveredFlags_AllowWhenOverlappedByWindow   = 1 << 9,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window.
    ImGuiHoveredFlags_AllowWhenDisabled             = 1 << 10,  // IsItemHovered() only: Return true even if the item is disabled
    ImGuiHoveredFlags_NoNavOverride                 = 1 << 11,  // IsItemHovered() only: Disable using keyboard/gamepad navigation state when active, always query mouse
    ImGuiHoveredFlags_AllowWhenOverlapped           = ImGuiHoveredFlags_AllowWhenOverlappedByItem | ImGuiHoveredFlags_AllowWhenOverlappedByWindow,
    ImGuiHoveredFlags_RectOnly                      = ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped,
    ImGuiHoveredFlags_RootAndChildWindows           = ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows,

    ImGuiHoveredFlags_ForTooltip                    = 1 << 12,  // Shortcut for standard flags when using IsItemHovered() + SetTooltip() sequence.

    ImGuiHoveredFlags_Stationary                    = 1 << 13,  // Require mouse to be stationary for style.HoverStationaryDelay (~0.15 sec) _at least one time_. After this, can move on same item/window. Using the stationary test tends to reduces the need for a long delay.
    ImGuiHoveredFlags_DelayNone                     = 1 << 14,  // IsItemHovered() only: Return true immediately (default). As this is the default you generally ignore this.
    ImGuiHoveredFlags_DelayShort                    = 1 << 15,  // IsItemHovered() only: Return true after style.HoverDelayShort elapsed (~0.15 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    ImGuiHoveredFlags_DelayNormal                   = 1 << 16,  // IsItemHovered() only: Return true after style.HoverDelayNormal elapsed (~0.40 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    ImGuiHoveredFlags_NoSharedDelay                 = 1 << 17,  // IsItemHovered() only: Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};

enum ImGuiDragDropFlags_
{
    ImGuiDragDropFlags_None                         = 0,
    ImGuiDragDropFlags_SourceNoPreviewTooltip       = 1 << 0,   // Disable preview tooltip. By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
    ImGuiDragDropFlags_SourceNoDisableHover         = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
    ImGuiDragDropFlags_SourceNoHoldToOpenOthers     = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    ImGuiDragDropFlags_SourceAllowNullID            = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.
    ImGuiDragDropFlags_SourceExtern                 = 1 << 4,   // External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    ImGuiDragDropFlags_PayloadAutoExpire            = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
    ImGuiDragDropFlags_PayloadNoCrossContext        = 1 << 6,   // Hint to specify that the payload may not be copied outside current dear imgui context.
    ImGuiDragDropFlags_PayloadNoCrossProcess        = 1 << 7,   // Hint to specify that the payload may not be copied outside current process.
    ImGuiDragDropFlags_AcceptBeforeDelivery         = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    ImGuiDragDropFlags_AcceptNoDrawDefaultRect      = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
    ImGuiDragDropFlags_AcceptNoPreviewTooltip       = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
    ImGuiDragDropFlags_AcceptDrawAsHovered          = 1 << 13,  // Accepting item will render as if hovered. Useful for e.g. a Button() used as a drop target.
    ImGuiDragDropFlags_AcceptPeekOnly               = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect, // For peeking ahead and inspecting the payload before delivery.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiDragDropFlags_SourceAutoExpirePayload = ImGuiDragDropFlags_PayloadAutoExpire, // Renamed in 1.90.9
#endif
};

#define IMGUI_PAYLOAD_TYPE_COLOR_3F     "_COL3F"    // float[3]: Standard type for colors, without alpha. User code may use this type.
#define IMGUI_PAYLOAD_TYPE_COLOR_4F     "_COL4F"    // float[4]: Standard type for colors. User code may use this type.

enum ImGuiDataType_
{
    ImGuiDataType_S8,       // signed char / char (with sensible compilers)
    ImGuiDataType_U8,       // unsigned char
    ImGuiDataType_S16,      // short
    ImGuiDataType_U16,      // unsigned short
    ImGuiDataType_S32,      // int
    ImGuiDataType_U32,      // unsigned int
    ImGuiDataType_S64,      // long long / __int64
    ImGuiDataType_U64,      // unsigned long long / unsigned __int64
    ImGuiDataType_Float,    // float
    ImGuiDataType_Double,   // double
    ImGuiDataType_Bool,     // bool (provided for user convenience, not supported by scalar widgets)
    ImGuiDataType_String,   // char* (provided for user convenience, not supported by scalar widgets)
    ImGuiDataType_COUNT
};

enum ImGuiDir : int
{
    ImGuiDir_None    = -1,
    ImGuiDir_Left    = 0,
    ImGuiDir_Right   = 1,
    ImGuiDir_Up      = 2,
    ImGuiDir_Down    = 3,
    ImGuiDir_COUNT
};

enum ImGuiSortDirection : ImU8
{
    ImGuiSortDirection_None         = 0,
    ImGuiSortDirection_Ascending    = 1,    // Ascending = 0->9, A->Z etc.
    ImGuiSortDirection_Descending   = 2     // Descending = 9->0, Z->A etc.
};

enum ImGuiKey : int
{
    // Keyboard
    ImGuiKey_None = 0,
    ImGuiKey_NamedKey_BEGIN = 512,  // First valid key value (other than 0)

    ImGuiKey_Tab = 512,             // == ImGuiKey_NamedKey_BEGIN
    ImGuiKey_LeftArrow,
    ImGuiKey_RightArrow,
    ImGuiKey_UpArrow,
    ImGuiKey_DownArrow,
    ImGuiKey_PageUp,
    ImGuiKey_PageDown,
    ImGuiKey_Home,
    ImGuiKey_End,
    ImGuiKey_Insert,
    ImGuiKey_Delete,
    ImGuiKey_Backspace,
    ImGuiKey_Space,
    ImGuiKey_Enter,
    ImGuiKey_Escape,
    ImGuiKey_LeftCtrl, ImGuiKey_LeftShift, ImGuiKey_LeftAlt, ImGuiKey_LeftSuper,     // Also see ImGuiMod_Ctrl, ImGuiMod_Shift, ImGuiMod_Alt, ImGuiMod_Super below!
    ImGuiKey_RightCtrl, ImGuiKey_RightShift, ImGuiKey_RightAlt, ImGuiKey_RightSuper,
    ImGuiKey_Menu,
    ImGuiKey_0, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9,
    ImGuiKey_A, ImGuiKey_B, ImGuiKey_C, ImGuiKey_D, ImGuiKey_E, ImGuiKey_F, ImGuiKey_G, ImGuiKey_H, ImGuiKey_I, ImGuiKey_J,
    ImGuiKey_K, ImGuiKey_L, ImGuiKey_M, ImGuiKey_N, ImGuiKey_O, ImGuiKey_P, ImGuiKey_Q, ImGuiKey_R, ImGuiKey_S, ImGuiKey_T,
    ImGuiKey_U, ImGuiKey_V, ImGuiKey_W, ImGuiKey_X, ImGuiKey_Y, ImGuiKey_Z,
    ImGuiKey_F1, ImGuiKey_F2, ImGuiKey_F3, ImGuiKey_F4, ImGuiKey_F5, ImGuiKey_F6,
    ImGuiKey_F7, ImGuiKey_F8, ImGuiKey_F9, ImGuiKey_F10, ImGuiKey_F11, ImGuiKey_F12,
    ImGuiKey_F13, ImGuiKey_F14, ImGuiKey_F15, ImGuiKey_F16, ImGuiKey_F17, ImGuiKey_F18,
    ImGuiKey_F19, ImGuiKey_F20, ImGuiKey_F21, ImGuiKey_F22, ImGuiKey_F23, ImGuiKey_F24,
    ImGuiKey_Apostrophe,        // '
    ImGuiKey_Comma,             // ,
    ImGuiKey_Minus,             // -
    ImGuiKey_Period,            // .
    ImGuiKey_Slash,             // /
    ImGuiKey_Semicolon,         // ;
    ImGuiKey_Equal,             // =
    ImGuiKey_LeftBracket,       // [
    ImGuiKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    ImGuiKey_RightBracket,      // ]
    ImGuiKey_GraveAccent,       // `
    ImGuiKey_CapsLock,
    ImGuiKey_ScrollLock,
    ImGuiKey_NumLock,
    ImGuiKey_PrintScreen,
    ImGuiKey_Pause,
    ImGuiKey_Keypad0, ImGuiKey_Keypad1, ImGuiKey_Keypad2, ImGuiKey_Keypad3, ImGuiKey_Keypad4,
    ImGuiKey_Keypad5, ImGuiKey_Keypad6, ImGuiKey_Keypad7, ImGuiKey_Keypad8, ImGuiKey_Keypad9,
    ImGuiKey_KeypadDecimal,
    ImGuiKey_KeypadDivide,
    ImGuiKey_KeypadMultiply,
    ImGuiKey_KeypadSubtract,
    ImGuiKey_KeypadAdd,
    ImGuiKey_KeypadEnter,
    ImGuiKey_KeypadEqual,
    ImGuiKey_AppBack,               // Available on some keyboard/mouses. Often referred as "Browser Back"
    ImGuiKey_AppForward,
    ImGuiKey_Oem102,                // Non-US backslash.

    // Gamepad
    ImGuiKey_GamepadStart,          // Menu        | +       | Options  |
    ImGuiKey_GamepadBack,           // View        | -       | Share    |
    ImGuiKey_GamepadFaceLeft,       // X           | Y       | Square   | Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
    ImGuiKey_GamepadFaceRight,      // B           | A       | Circle   | Cancel / Close / Exit
    ImGuiKey_GamepadFaceUp,         // Y           | X       | Triangle | Text Input / On-screen Keyboard
    ImGuiKey_GamepadFaceDown,       // A           | B       | Cross    | Activate / Open / Toggle / Tweak
    ImGuiKey_GamepadDpadLeft,       // D-pad Left  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadRight,      // D-pad Right | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadUp,         // D-pad Up    | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadDpadDown,       // D-pad Down  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    ImGuiKey_GamepadL1,             // L Bumper    | L       | L1       | Tweak Slower / Focus Previous (in Windowing mode)
    ImGuiKey_GamepadR1,             // R Bumper    | R       | R1       | Tweak Faster / Focus Next (in Windowing mode)
    ImGuiKey_GamepadL2,             // L Trigger   | ZL      | L2       | [Analog]
    ImGuiKey_GamepadR2,             // R Trigger   | ZR      | R2       | [Analog]
    ImGuiKey_GamepadL3,             // L Stick     | L3      | L3       |
    ImGuiKey_GamepadR3,             // R Stick     | R3      | R3       |
    ImGuiKey_GamepadLStickLeft,     //             |         |          | [Analog] Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickRight,    //             |         |          | [Analog] Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickUp,       //             |         |          | [Analog] Move Window (in Windowing mode)
    ImGuiKey_GamepadLStickDown,     //             |         |          | [Analog] Move Window (in Windowing mode)
    ImGuiKey_GamepadRStickLeft,     //             |         |          | [Analog]
    ImGuiKey_GamepadRStickRight,    //             |         |          | [Analog]
    ImGuiKey_GamepadRStickUp,       //             |         |          | [Analog]
    ImGuiKey_GamepadRStickDown,     //             |         |          | [Analog]

    ImGuiKey_MouseLeft, ImGuiKey_MouseRight, ImGuiKey_MouseMiddle, ImGuiKey_MouseX1, ImGuiKey_MouseX2, ImGuiKey_MouseWheelX, ImGuiKey_MouseWheelY,

    ImGuiKey_ReservedForModCtrl, ImGuiKey_ReservedForModShift, ImGuiKey_ReservedForModAlt, ImGuiKey_ReservedForModSuper,

    ImGuiKey_NamedKey_END,
    ImGuiKey_NamedKey_COUNT = ImGuiKey_NamedKey_END - ImGuiKey_NamedKey_BEGIN,

    ImGuiMod_None                   = 0,
    ImGuiMod_Ctrl                   = 1 << 12, // Ctrl (non-macOS), Cmd (macOS)
    ImGuiMod_Shift                  = 1 << 13, // Shift
    ImGuiMod_Alt                    = 1 << 14, // Option/Menu
    ImGuiMod_Super                  = 1 << 15, // Windows/Super (non-macOS), Ctrl (macOS)
    ImGuiMod_Mask_                  = 0xF000,  // 4-bits

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiKey_COUNT                  = ImGuiKey_NamedKey_END,    // Obsoleted in 1.91.5 because it was misleading (since named keys don't start at 0 anymore)
    ImGuiMod_Shortcut               = ImGuiMod_Ctrl,            // Removed in 1.90.7, you can now simply use ImGuiMod_Ctrl
#endif
};

enum ImGuiInputFlags_
{
    ImGuiInputFlags_None                    = 0,
    ImGuiInputFlags_Repeat                  = 1 << 0,   // Enable repeat. Return true on successive repeats. Default for legacy IsKeyPressed(). NOT Default for legacy IsMouseClicked(). MUST BE == 1.

    ImGuiInputFlags_RouteActive             = 1 << 10,  // Route to active item only.
    ImGuiInputFlags_RouteFocused            = 1 << 11,  // Route to windows in the focus stack (DEFAULT). Deep-most focused window takes inputs. Active item takes inputs over deep-most focused window.
    ImGuiInputFlags_RouteGlobal             = 1 << 12,  // Global route (unless a focused window or active item registered the route).
    ImGuiInputFlags_RouteAlways             = 1 << 13,  // Do not register route, poll keys directly.
    ImGuiInputFlags_RouteOverFocused        = 1 << 14,  // Option: global route: higher priority than focused route (unless active item in focused route).
    ImGuiInputFlags_RouteOverActive         = 1 << 15,  // Option: global route: higher priority than active item. Unlikely you need to use that: will interfere with every active items, e.g. Ctrl+A registered by InputText will be overridden by this. May not be fully honored as user/internal code is likely to always assume they can access keys when active.
    ImGuiInputFlags_RouteUnlessBgFocused    = 1 << 16,  // Option: global route: will not be applied if underlying background/void is focused (== no Dear ImGui windows are focused). Useful for overlay applications.
    ImGuiInputFlags_RouteFromRootWindow     = 1 << 17,  // Option: route evaluated from the point of view of root window rather than current window.

    ImGuiInputFlags_Tooltip                 = 1 << 18,  // Automatically display a tooltip when hovering item [BETA] Unsure of right api (opt-in/opt-out)
};

enum ImGuiConfigFlags_
{
    ImGuiConfigFlags_None                   = 0,
    ImGuiConfigFlags_NavEnableKeyboard      = 1 << 0,   // Master keyboard navigation enable flag. Enable full Tabbing + directional arrows + space/enter to activate.
    ImGuiConfigFlags_NavEnableGamepad       = 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set ImGuiBackendFlags_HasGamepad.
    ImGuiConfigFlags_NoMouse                = 1 << 4,   // Instruct dear imgui to disable mouse inputs and interactions.
    ImGuiConfigFlags_NoMouseCursorChange    = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.
    ImGuiConfigFlags_NoKeyboard             = 1 << 6,   // Instruct dear imgui to disable keyboard inputs and interactions. This is done by ignoring keyboard events and clearing existing states.

    ImGuiConfigFlags_IsSRGB                 = 1 << 20,  // Application is SRGB-aware.
    ImGuiConfigFlags_IsTouchScreen          = 1 << 21,  // Application is using a touch screen instead of a mouse.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiConfigFlags_NavEnableSetMousePos   = 1 << 2,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavMoveSetMousePos
    ImGuiConfigFlags_NavNoCaptureKeyboard   = 1 << 3,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavCaptureKeyboard
#endif
};

enum ImGuiBackendFlags_
{
    ImGuiBackendFlags_None                  = 0,
    ImGuiBackendFlags_HasGamepad            = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
    ImGuiBackendFlags_HasMouseCursors       = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    ImGuiBackendFlags_HasSetMousePos        = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if io.ConfigNavMoveSetMousePos is set).
    ImGuiBackendFlags_RendererHasVtxOffset  = 1 << 3,   // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
    ImGuiBackendFlags_RendererHasTextures   = 1 << 4,   // Backend Renderer supports ImTextureData requests to create/update/destroy textures. This enables incremental texture updates and texture reloads. See https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md for instructions on how to upgrade your custom backend.
};

enum ImGuiCol_
{
    ImGuiCol_Text,
    ImGuiCol_TextDisabled,
    ImGuiCol_WindowBg,              // Background of normal windows
    ImGuiCol_ChildBg,               // Background of child windows
    ImGuiCol_PopupBg,               // Background of popups, menus, tooltips windows
    ImGuiCol_Border,
    ImGuiCol_BorderShadow,
    ImGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    ImGuiCol_FrameBgHovered,
    ImGuiCol_FrameBgActive,
    ImGuiCol_TitleBg,               // Title bar
    ImGuiCol_TitleBgActive,         // Title bar when focused
    ImGuiCol_TitleBgCollapsed,      // Title bar when collapsed
    ImGuiCol_MenuBarBg,
    ImGuiCol_ScrollbarBg,
    ImGuiCol_ScrollbarGrab,
    ImGuiCol_ScrollbarGrabHovered,
    ImGuiCol_ScrollbarGrabActive,
    ImGuiCol_CheckMark,             // Checkbox tick and RadioButton circle
    ImGuiCol_SliderGrab,
    ImGuiCol_SliderGrabActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonHovered,
    ImGuiCol_ButtonActive,
    ImGuiCol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Separator,
    ImGuiCol_SeparatorHovered,
    ImGuiCol_SeparatorActive,
    ImGuiCol_ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
    ImGuiCol_ResizeGripHovered,
    ImGuiCol_ResizeGripActive,
    ImGuiCol_InputTextCursor,       // InputText cursor/caret
    ImGuiCol_TabHovered,            // Tab background, when hovered
    ImGuiCol_Tab,                   // Tab background, when tab-bar is focused & tab is unselected
    ImGuiCol_TabSelected,           // Tab background, when tab-bar is focused & tab is selected
    ImGuiCol_TabSelectedOverline,   // Tab horizontal overline, when tab-bar is focused & tab is selected
    ImGuiCol_TabDimmed,             // Tab background, when tab-bar is unfocused & tab is unselected
    ImGuiCol_TabDimmedSelected,     // Tab background, when tab-bar is unfocused & tab is selected
    ImGuiCol_TabDimmedSelectedOverline,//..horizontal overline, when tab-bar is unfocused & tab is selected
    ImGuiCol_PlotLines,
    ImGuiCol_PlotLinesHovered,
    ImGuiCol_PlotHistogram,
    ImGuiCol_PlotHistogramHovered,
    ImGuiCol_TableHeaderBg,         // Table header background
    ImGuiCol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    ImGuiCol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    ImGuiCol_TableRowBg,            // Table row background (even rows)
    ImGuiCol_TableRowBgAlt,         // Table row background (odd rows)
    ImGuiCol_TextLink,              // Hyperlink color
    ImGuiCol_TextSelectedBg,        // Selected text inside an InputText
    ImGuiCol_TreeLines,             // Tree node hierarchy outlines when using ImGuiTreeNodeFlags_DrawLines
    ImGuiCol_DragDropTarget,        // Rectangle border highlighting a drop target
    ImGuiCol_DragDropTargetBg,      // Rectangle background highlighting a drop target
    ImGuiCol_UnsavedMarker,         // Unsaved Document marker (in window title and tabs)
    ImGuiCol_NavCursor,             // Color of keyboard/gamepad navigation cursor/rectangle, when visible
    ImGuiCol_NavWindowingHighlight, // Highlight window when using Ctrl+Tab
    ImGuiCol_NavWindowingDimBg,     // Darken/colorize entire screen behind the Ctrl+Tab window list, when active
    ImGuiCol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    ImGuiCol_COUNT,

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiCol_TabActive = ImGuiCol_TabSelected,                  // [renamed in 1.90.9]
    ImGuiCol_TabUnfocused = ImGuiCol_TabDimmed,                 // [renamed in 1.90.9]
    ImGuiCol_TabUnfocusedActive = ImGuiCol_TabDimmedSelected,   // [renamed in 1.90.9]
    ImGuiCol_NavHighlight = ImGuiCol_NavCursor,                 // [renamed in 1.91.4]
#endif
};

enum ImGuiStyleVar_
{
    ImGuiStyleVar_Alpha,                    // float     Alpha
    ImGuiStyleVar_DisabledAlpha,            // float     DisabledAlpha
    ImGuiStyleVar_WindowPadding,            // ImVec2    WindowPadding
    ImGuiStyleVar_WindowRounding,           // float     WindowRounding
    ImGuiStyleVar_WindowBorderSize,         // float     WindowBorderSize
    ImGuiStyleVar_WindowMinSize,            // ImVec2    WindowMinSize
    ImGuiStyleVar_WindowTitleAlign,         // ImVec2    WindowTitleAlign
    ImGuiStyleVar_ChildRounding,            // float     ChildRounding
    ImGuiStyleVar_ChildBorderSize,          // float     ChildBorderSize
    ImGuiStyleVar_PopupRounding,            // float     PopupRounding
    ImGuiStyleVar_PopupBorderSize,          // float     PopupBorderSize
    ImGuiStyleVar_FramePadding,             // ImVec2    FramePadding
    ImGuiStyleVar_FrameRounding,            // float     FrameRounding
    ImGuiStyleVar_FrameBorderSize,          // float     FrameBorderSize
    ImGuiStyleVar_ItemSpacing,              // ImVec2    ItemSpacing
    ImGuiStyleVar_ItemInnerSpacing,         // ImVec2    ItemInnerSpacing
    ImGuiStyleVar_IndentSpacing,            // float     IndentSpacing
    ImGuiStyleVar_CellPadding,              // ImVec2    CellPadding
    ImGuiStyleVar_ScrollbarSize,            // float     ScrollbarSize
    ImGuiStyleVar_ScrollbarRounding,        // float     ScrollbarRounding
    ImGuiStyleVar_ScrollbarPadding,         // float     ScrollbarPadding
    ImGuiStyleVar_GrabMinSize,              // float     GrabMinSize
    ImGuiStyleVar_GrabRounding,             // float     GrabRounding
    ImGuiStyleVar_ImageBorderSize,          // float     ImageBorderSize
    ImGuiStyleVar_TabRounding,              // float     TabRounding
    ImGuiStyleVar_TabBorderSize,            // float     TabBorderSize
    ImGuiStyleVar_TabMinWidthBase,          // float     TabMinWidthBase
    ImGuiStyleVar_TabMinWidthShrink,        // float     TabMinWidthShrink
    ImGuiStyleVar_TabBarBorderSize,         // float     TabBarBorderSize
    ImGuiStyleVar_TabBarOverlineSize,       // float     TabBarOverlineSize
    ImGuiStyleVar_TableAngledHeadersAngle,  // float     TableAngledHeadersAngle
    ImGuiStyleVar_TableAngledHeadersTextAlign,// ImVec2  TableAngledHeadersTextAlign
    ImGuiStyleVar_TreeLinesSize,            // float     TreeLinesSize
    ImGuiStyleVar_TreeLinesRounding,        // float     TreeLinesRounding
    ImGuiStyleVar_ButtonTextAlign,          // ImVec2    ButtonTextAlign
    ImGuiStyleVar_SelectableTextAlign,      // ImVec2    SelectableTextAlign
    ImGuiStyleVar_SeparatorTextBorderSize,  // float     SeparatorTextBorderSize
    ImGuiStyleVar_SeparatorTextAlign,       // ImVec2    SeparatorTextAlign
    ImGuiStyleVar_SeparatorTextPadding,     // ImVec2    SeparatorTextPadding
    ImGuiStyleVar_COUNT
};

enum ImGuiButtonFlags_
{
    ImGuiButtonFlags_None                   = 0,
    ImGuiButtonFlags_MouseButtonLeft        = 1 << 0,   // React on left mouse button (default)
    ImGuiButtonFlags_MouseButtonRight       = 1 << 1,   // React on right mouse button
    ImGuiButtonFlags_MouseButtonMiddle      = 1 << 2,   // React on center mouse button
    ImGuiButtonFlags_MouseButtonMask_       = ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle, // [Internal]
    ImGuiButtonFlags_EnableNav              = 1 << 3,   // InvisibleButton(): do not disable navigation/tabbing. Otherwise disabled by default.
};

enum ImGuiColorEditFlags_
{
    ImGuiColorEditFlags_None            = 0,
    ImGuiColorEditFlags_NoAlpha         = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    ImGuiColorEditFlags_NoPicker        = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
    ImGuiColorEditFlags_NoOptions       = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    ImGuiColorEditFlags_NoSmallPreview  = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    ImGuiColorEditFlags_NoInputs        = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    ImGuiColorEditFlags_NoTooltip       = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    ImGuiColorEditFlags_NoLabel         = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    ImGuiColorEditFlags_NoSidePreview   = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    ImGuiColorEditFlags_NoDragDrop      = 1 << 9,   //              // ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.
    ImGuiColorEditFlags_NoBorder        = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)

    ImGuiColorEditFlags_AlphaOpaque     = 1 << 11,  //              // ColorEdit, ColorPicker, ColorButton: disable alpha in the preview,. Contrary to _NoAlpha it may still be edited when calling ColorEdit4()/ColorPicker4(). For ColorButton() this does the same as _NoAlpha.
    ImGuiColorEditFlags_AlphaNoBg       = 1 << 12,  //              // ColorEdit, ColorPicker, ColorButton: disable rendering a checkerboard background behind transparent color.
    ImGuiColorEditFlags_AlphaPreviewHalf= 1 << 13,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half transparent preview.

    ImGuiColorEditFlags_AlphaBar        = 1 << 16,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    ImGuiColorEditFlags_HDR             = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).
    ImGuiColorEditFlags_DisplayRGB      = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    ImGuiColorEditFlags_DisplayHSV      = 1 << 21,  // [Display]    // "
    ImGuiColorEditFlags_DisplayHex      = 1 << 22,  // [Display]    // "
    ImGuiColorEditFlags_Uint8           = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    ImGuiColorEditFlags_Float           = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    ImGuiColorEditFlags_PickerHueBar    = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    ImGuiColorEditFlags_PickerHueWheel  = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    ImGuiColorEditFlags_InputRGB        = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    ImGuiColorEditFlags_InputHSV        = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

    ImGuiColorEditFlags_DefaultOptions_ = ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar,

    ImGuiColorEditFlags_AlphaMask_      = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaOpaque | ImGuiColorEditFlags_AlphaNoBg | ImGuiColorEditFlags_AlphaPreviewHalf,
    ImGuiColorEditFlags_DisplayMask_    = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_DisplayHex,
    ImGuiColorEditFlags_DataTypeMask_   = ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_Float,
    ImGuiColorEditFlags_PickerMask_     = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_PickerHueBar,
    ImGuiColorEditFlags_InputMask_      = ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_InputHSV,

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiColorEditFlags_AlphaPreview = 0, // Removed in 1.91.8. This is the default now. Will display a checkerboard unless ImGuiColorEditFlags_AlphaNoBg is set.
#endif
};

enum ImGuiSliderFlags_
{
    ImGuiSliderFlags_None               = 0,
    ImGuiSliderFlags_Logarithmic        = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    ImGuiSliderFlags_NoRoundToFormat    = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).
    ImGuiSliderFlags_NoInput            = 1 << 7,       // Disable Ctrl+Click or Enter key allowing to input text directly into the widget.
    ImGuiSliderFlags_WrapAround         = 1 << 8,       // Enable wrapping around from max to min and from min to max. Only supported by DragXXX() functions for now.
    ImGuiSliderFlags_ClampOnInput       = 1 << 9,       // Clamp value to min/max bounds when input manually with Ctrl+Click. By default Ctrl+Click allows going out of bounds.
    ImGuiSliderFlags_ClampZeroRange     = 1 << 10,      // Clamp even if min==max==0.0f. Otherwise due to legacy reason DragXXX functions don't clamp with those values. When your clamping limits are dynamic you almost always want to use it.
    ImGuiSliderFlags_NoSpeedTweaks      = 1 << 11,      // Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.
    ImGuiSliderFlags_AlwaysClamp        = ImGuiSliderFlags_ClampOnInput | ImGuiSliderFlags_ClampZeroRange,
    ImGuiSliderFlags_InvalidMask_       = 0x7000000F,   // [Internal] We treat using those bits as being potentially a 'float power' argument from the previous API that has got miscast to this enum, and will trigger an assert if needed.
};

enum ImGuiMouseButton_
{
    ImGuiMouseButton_Left = 0,
    ImGuiMouseButton_Right = 1,
    ImGuiMouseButton_Middle = 2,
    ImGuiMouseButton_COUNT = 5
};

enum ImGuiMouseCursor_
{
    ImGuiMouseCursor_None = -1,
    ImGuiMouseCursor_Arrow = 0,
    ImGuiMouseCursor_TextInput,         // When hovering over InputText, etc.
    ImGuiMouseCursor_ResizeAll,         // (Unused by Dear ImGui functions)
    ImGuiMouseCursor_ResizeNS,          // When hovering over a horizontal border
    ImGuiMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
    ImGuiMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
    ImGuiMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
    ImGuiMouseCursor_Hand,              // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
    ImGuiMouseCursor_Wait,              // When waiting for something to process/load.
    ImGuiMouseCursor_Progress,          // When waiting for something to process/load, but application is still interactive.
    ImGuiMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    ImGuiMouseCursor_COUNT
};

enum ImGuiMouseSource : int
{
    ImGuiMouseSource_Mouse = 0,         // Input is coming from an actual mouse.
    ImGuiMouseSource_TouchScreen,       // Input is coming from a touch screen (no hovering prior to initial press, less precise initial press aiming, dual-axis wheeling possible).
    ImGuiMouseSource_Pen,               // Input is coming from a pressure/magnetic pen (often used in conjunction with high-sampling rates).
    ImGuiMouseSource_COUNT
};

enum ImGuiCond_
{
    ImGuiCond_None          = 0,        // No condition (always set the variable), same as _Always
    ImGuiCond_Always        = 1 << 0,   // No condition (always set the variable), same as _None
    ImGuiCond_Once          = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    ImGuiCond_FirstUseEver  = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    ImGuiCond_Appearing     = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum ImGuiTableFlags_
{
    // Features
    ImGuiTableFlags_None                       = 0,
    ImGuiTableFlags_Resizable                  = 1 << 0,   // Enable resizing columns.
    ImGuiTableFlags_Reorderable                = 1 << 1,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
    ImGuiTableFlags_Hideable                   = 1 << 2,   // Enable hiding/disabling columns in context menu.
    ImGuiTableFlags_Sortable                   = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see ImGuiTableFlags_SortMulti and ImGuiTableFlags_SortTristate.
    ImGuiTableFlags_NoSavedSettings            = 1 << 4,   // Disable persisting columns order, width and sort settings in the .ini file.
    ImGuiTableFlags_ContextMenuInBody          = 1 << 5,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
    // Decorations
    ImGuiTableFlags_RowBg                      = 1 << 6,   // Set each RowBg color with ImGuiCol_TableRowBg or ImGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)
    ImGuiTableFlags_BordersInnerH              = 1 << 7,   // Draw horizontal borders between rows.
    ImGuiTableFlags_BordersOuterH              = 1 << 8,   // Draw horizontal borders at the top and bottom.
    ImGuiTableFlags_BordersInnerV              = 1 << 9,   // Draw vertical borders between columns.
    ImGuiTableFlags_BordersOuterV              = 1 << 10,  // Draw vertical borders on the left and right sides.
    ImGuiTableFlags_BordersH                   = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuterH, // Draw horizontal borders.
    ImGuiTableFlags_BordersV                   = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV, // Draw vertical borders.
    ImGuiTableFlags_BordersInner               = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH, // Draw inner borders.
    ImGuiTableFlags_BordersOuter               = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH, // Draw outer borders.
    ImGuiTableFlags_Borders                    = ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter,   // Draw all borders.
    ImGuiTableFlags_NoBordersInBody            = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
    ImGuiTableFlags_NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
    ImGuiTableFlags_SizingFixedFit             = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    ImGuiTableFlags_SizingFixedSame            = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGuiTableFlags_NoKeepColumnsVisible.
    ImGuiTableFlags_SizingStretchProp          = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    ImGuiTableFlags_SizingStretchSame          = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
    ImGuiTableFlags_NoHostExtendX              = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    ImGuiTableFlags_NoHostExtendY              = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    ImGuiTableFlags_NoKeepColumnsVisible       = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    ImGuiTableFlags_PreciseWidths              = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
    // Clipping
    ImGuiTableFlags_NoClip                     = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
    // Padding
    ImGuiTableFlags_PadOuterX                  = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
    ImGuiTableFlags_NoPadOuterX                = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
    ImGuiTableFlags_NoPadInnerX                = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
    // Scrolling
    ImGuiTableFlags_ScrollX                    = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
    ImGuiTableFlags_ScrollY                    = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
    // Sorting
    ImGuiTableFlags_SortMulti                  = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    ImGuiTableFlags_SortTristate               = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).
    // Miscellaneous
    ImGuiTableFlags_HighlightHoveredColumn     = 1 << 28,  // Highlight column headers when hovered (may evolve into a fuller highlight)

    ImGuiTableFlags_SizingMask_                = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingStretchSame,
};

enum ImGuiTableColumnFlags_
{
    ImGuiTableColumnFlags_None                  = 0,
    ImGuiTableColumnFlags_Disabled              = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    ImGuiTableColumnFlags_DefaultHide           = 1 << 1,   // Default as a hidden/disabled column.
    ImGuiTableColumnFlags_DefaultSort           = 1 << 2,   // Default as a sorting column.
    ImGuiTableColumnFlags_WidthStretch          = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    ImGuiTableColumnFlags_WidthFixed            = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    ImGuiTableColumnFlags_NoResize              = 1 << 5,   // Disable manual resizing.
    ImGuiTableColumnFlags_NoReorder             = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    ImGuiTableColumnFlags_NoHide                = 1 << 7,   // Disable ability to hide/disable this column.
    ImGuiTableColumnFlags_NoClip                = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    ImGuiTableColumnFlags_NoSort                = 1 << 9,   // Disable ability to sort on this field (even if ImGuiTableFlags_Sortable is set on the table).
    ImGuiTableColumnFlags_NoSortAscending       = 1 << 10,  // Disable ability to sort in the ascending direction.
    ImGuiTableColumnFlags_NoSortDescending      = 1 << 11,  // Disable ability to sort in the descending direction.
    ImGuiTableColumnFlags_NoHeaderLabel         = 1 << 12,  // TableHeadersRow() will submit an empty label for this column. Convenient for some small columns. Name will still appear in context menu or in angled headers. You may append into this cell by calling TableSetColumnIndex() right after the TableHeadersRow() call.
    ImGuiTableColumnFlags_NoHeaderWidth         = 1 << 13,  // Disable header text width contribution to automatic column width.
    ImGuiTableColumnFlags_PreferSortAscending   = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
    ImGuiTableColumnFlags_PreferSortDescending  = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
    ImGuiTableColumnFlags_IndentEnable          = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
    ImGuiTableColumnFlags_IndentDisable         = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.
    ImGuiTableColumnFlags_AngledHeader          = 1 << 18,  // TableHeadersRow() will submit an angled header row for this column. Note this will add an extra row.

    ImGuiTableColumnFlags_IsEnabled             = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    ImGuiTableColumnFlags_IsVisible             = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
    ImGuiTableColumnFlags_IsSorted              = 1 << 26,  // Status: is currently part of the sort specs
    ImGuiTableColumnFlags_IsHovered             = 1 << 27,  // Status: is hovered by mouse

    ImGuiTableColumnFlags_WidthMask_            = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_WidthFixed,
    ImGuiTableColumnFlags_IndentMask_           = ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_IndentDisable,
    ImGuiTableColumnFlags_StatusMask_           = ImGuiTableColumnFlags_IsEnabled | ImGuiTableColumnFlags_IsVisible | ImGuiTableColumnFlags_IsSorted | ImGuiTableColumnFlags_IsHovered,
    ImGuiTableColumnFlags_NoDirectResize_       = 1 << 30,  // [Internal] Disable user resizing this column directly (it may however we resized indirectly from its left edge)
};

enum ImGuiTableRowFlags_
{
    ImGuiTableRowFlags_None                     = 0,
    ImGuiTableRowFlags_Headers                  = 1 << 0,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};

enum ImGuiTableBgTarget_
{
    ImGuiTableBgTarget_None                     = 0,
    ImGuiTableBgTarget_RowBg0                   = 1,        // Set row background color 0 (generally used for background, automatically set when ImGuiTableFlags_RowBg is used)
    ImGuiTableBgTarget_RowBg1                   = 2,        // Set row background color 1 (generally used for selection marking)
    ImGuiTableBgTarget_CellBg                   = 3,        // Set cell background color (top-most color)
};

struct ImGuiTableSortSpecs
{
    const ImGuiTableColumnSortSpecs* Specs;     // Pointer to sort spec array.
    int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when ImGuiTableFlags_SortMulti is enabled. May be == 0 when ImGuiTableFlags_SortTristate is enabled.
    bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

    ImGuiTableSortSpecs()       { memset(this, 0, sizeof(*this)); }
};

struct ImGuiTableColumnSortSpecs
{
    ImGuiID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
    ImS16                       ColumnIndex;        // Index of the column
    ImS16                       SortOrder;          // Index within parent ImGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
    ImGuiSortDirection          SortDirection;      // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending

    ImGuiTableColumnSortSpecs() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
#define IMGUI_DEBUG_LOG(...)        ImGui::DebugLog(__VA_ARGS__)
#else
#define IMGUI_DEBUG_LOG(...)        ((void)0)
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImNewWrapper {};
inline void* operator new(size_t, ImNewWrapper, void* ptr) { return ptr; }
inline void  operator delete(void*, ImNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()
#define IM_ALLOC(_SIZE)                     ImGui::MemAlloc(_SIZE)
#define IM_FREE(_PTR)                       ImGui::MemFree(_PTR)
#define IM_PLACEMENT_NEW(_PTR)              new(ImNewWrapper(), _PTR)
#define IM_NEW(_TYPE)                       new(ImNewWrapper(), ImGui::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void IM_DELETE(T* p)   { if (p) { p->~T(); ImGui::MemFree(p); } }

//-----------------------------------------------------------------------------
// ImVector<>
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IM_MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct ImVector
{
    int                 Size;
    int                 Capacity;
    T*                  Data;

    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    inline ImVector()                                       { Size = Capacity = 0; Data = NULL; }
    inline ImVector(const ImVector<T>& src)                 { Size = Capacity = 0; Data = NULL; operator=(src); }
    inline ImVector<T>& operator=(const ImVector<T>& src)   { clear(); resize(src.Size); if (Data && src.Data) memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this; }
    inline ~ImVector()                                      { if (Data) IM_FREE(Data); } // Important: does not destruct anything

    inline void         clear()                             { if (Data) { Size = Capacity = 0; IM_FREE(Data); Data = NULL; } }  // Important: does not destruct anything
    inline void         clear_delete()                      { for (int n = 0; n < Size; n++) IM_DELETE(Data[n]); clear(); }     // Important: never called automatically! always explicit.
    inline void         clear_destruct()                    { for (int n = 0; n < Size; n++) Data[n].~T(); clear(); }           // Important: never called automatically! always explicit.

    inline bool         empty() const                       { return Size == 0; }
    inline int          size() const                        { return Size; }
    inline int          size_in_bytes() const               { return Size * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { IM_ASSERT(i >= 0 && i < Size); return Data[i]; }
    inline const T&     operator[](int i) const             { IM_ASSERT(i >= 0 && i < Size); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Size; }
    inline const T*     end() const                         { return Data + Size; }
    inline T&           front()                             { IM_ASSERT(Size > 0); return Data[0]; }
    inline const T&     front() const                       { IM_ASSERT(Size > 0); return Data[0]; }
    inline T&           back()                              { IM_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const T&     back() const                        { IM_ASSERT(Size > 0); return Data[Size - 1]; }
    inline void         swap(ImVector<T>& rhs)              { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    inline void         shrink(int new_size)                { IM_ASSERT(new_size <= Size); Size = new_size; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; T* new_data = (T*)IM_ALLOC((size_t)new_capacity * sizeof(T)); if (Data) { memcpy(new_data, Data, (size_t)Size * sizeof(T)); IM_FREE(Data); } Data = new_data; Capacity = new_capacity; }
    inline void         reserve_discard(int new_capacity)   { if (new_capacity <= Capacity) return; if (Data) IM_FREE(Data); Data = (T*)IM_ALLOC((size_t)new_capacity * sizeof(T)); Capacity = new_capacity; }

    inline void         push_back(const T& v)               { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); memcpy(&Data[Size], &v, sizeof(v)); Size++; }
    inline void         pop_back()                          { IM_ASSERT(Size > 0); Size--; }
    inline void         push_front(const T& v)              { if (Size == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { IM_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T)); Size--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ IM_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T)); Size -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { IM_ASSERT(it >= Data && it < Data + Size);  const ptrdiff_t off = it - Data; if (it < Data + Size - 1) memcpy(Data + off, Data + Size - 1, sizeof(T)); Size--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { IM_ASSERT(it >= Data && it <= Data + Size); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Size++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline int          find_index(const T& v) const        { const T* data_end = Data + Size; const T* it = find(v); if (it == data_end) return -1; const ptrdiff_t off = it - Data; return (int)off; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { IM_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; return (int)off; }
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImGuiStyle
{
    float       FontSizeBase;               // Current base font size before external global factors are applied. Use PushFont(NULL, size) to modify. Use ImGui::GetFontSize() to obtain scaled value.
    float       FontScaleMain;              // Main global scale factor. May be set by application once, or exposed to end-user.
    float       FontScaleDpi;               // Additional global scale factor from viewport/monitor contents scale. When io.ConfigDpiScaleFonts is enabled, this is automatically overwritten when changing monitor DPI.

    float       Alpha;                      // Global alpha applies to everything in Dear ImGui.
    float       DisabledAlpha;              // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    ImVec2      WindowPadding;              // Padding within a window.
    float       WindowRounding;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    float       WindowBorderSize;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       WindowBorderHoverPadding;   // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    ImVec2      WindowMinSize;              // Minimum window size. This is a global setting. If you want to constrain individual windows, use SetNextWindowSizeConstraints().
    ImVec2      WindowTitleAlign;           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
    ImGuiDir    WindowMenuButtonPosition;   // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left.
    float       ChildRounding;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
    float       ChildBorderSize;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       PopupRounding;              // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
    float       PopupBorderSize;            // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    ImVec2      FramePadding;               // Padding within a framed rectangle (used by most widgets).
    float       FrameRounding;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
    float       FrameBorderSize;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    ImVec2      ItemSpacing;                // Horizontal and vertical spacing between widgets/lines.
    ImVec2      ItemInnerSpacing;           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
    ImVec2      CellPadding;                // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    ImVec2      TouchExtraPadding;          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    float       IndentSpacing;              // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    float       ColumnsMinSpacing;          // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    float       ScrollbarSize;              // Width of the vertical scrollbar, Height of the horizontal scrollbar.
    float       ScrollbarRounding;          // Radius of grab corners for scrollbar.
    float       ScrollbarPadding;           // Padding of scrollbar grab within its frame (same for both axes).
    float       GrabMinSize;                // Minimum width/height of a grab box for slider/scrollbar.
    float       GrabRounding;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    float       LogSliderDeadzone;          // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    float       ImageBorderSize;            // Thickness of border around Image() calls.
    float       TabRounding;                // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    float       TabBorderSize;              // Thickness of border around tabs.
    float       TabMinWidthBase;            // Minimum tab width, to make tabs larger than their contents. TabBar buttons are not affected.
    float       TabMinWidthShrink;          // Minimum tab width after shrinking, when using ImGuiTabBarFlags_FittingPolicyMixed policy.
    float       TabCloseButtonMinWidthSelected;     // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width.
    float       TabCloseButtonMinWidthUnselected;   // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    float       TabBarBorderSize;           // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    float       TabBarOverlineSize;         // Thickness of tab-bar overline, which highlights the selected tab-bar.
    float       TableAngledHeadersAngle;    // Angle of angled headers (supported values range from -50.0f degrees to +50.0f degrees).
    ImVec2      TableAngledHeadersTextAlign;// Alignment of angled headers within the cell
    ImGuiTreeNodeFlags TreeLinesFlags;      // Default way to draw lines connecting TreeNode hierarchy. ImGuiTreeNodeFlags_DrawLinesNone or ImGuiTreeNodeFlags_DrawLinesFull or ImGuiTreeNodeFlags_DrawLinesToNodes.
    float       TreeLinesSize;              // Thickness of outlines when using ImGuiTreeNodeFlags_DrawLines.
    float       TreeLinesRounding;          // Radius of lines connecting child nodes to the vertical line.
    float       DragDropTargetRounding;     // Radius of the drag and drop target frame.
    float       DragDropTargetBorderSize;   // Thickness of the drag and drop target border.
    float       DragDropTargetPadding;      // Size to expand the drag and drop target from actual target item size.
    ImGuiDir    ColorButtonPosition;        // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
    ImVec2      ButtonTextAlign;            // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
    ImVec2      SelectableTextAlign;        // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    float       SeparatorTextBorderSize;    // Thickness of border in SeparatorText()
    ImVec2      SeparatorTextAlign;         // Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    ImVec2      SeparatorTextPadding;       // Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    ImVec2      DisplayWindowPadding;       // Apply to regular windows: amount which we enforce to keep visible when moving near edges of your screen.
    ImVec2      DisplaySafeAreaPadding;     // Apply to every windows, menus, popups, tooltips: amount where we avoid displaying contents. Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).
    float       MouseCursorScale;           // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later.
    bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
    bool        AntiAliasedLinesUseTex;     // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to ImDrawList).
    bool        AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
    float       CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    float       CircleTessellationMaxError; // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Colors
    ImVec4      Colors[ImGuiCol_COUNT];

    // Behaviors
    float             HoverStationaryDelay;     // Delay for IsItemHovered(ImGuiHoveredFlags_Stationary). Time required to consider mouse stationary.
    float             HoverDelayShort;          // Delay for IsItemHovered(ImGuiHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    float             HoverDelayNormal;         // Delay for IsItemHovered(ImGuiHoveredFlags_DelayNormal). "
    ImGuiHoveredFlags HoverFlagsForTooltipMouse;// Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    ImGuiHoveredFlags HoverFlagsForTooltipNav;  // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    float       _MainScale;                 // FIXME-WIP: Reference scale, as applied by ScaleAllSizes().
    float       _NextFrameFontSizeBase;     // FIXME: Temporary hack until we finish remaining work.

    // Functions
    IMGUI_API   ImGuiStyle();
    IMGUI_API   void ScaleAllSizes(float scale_factor); // Scale all spacing/padding/thickness values. Do not scale fonts.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImGuiKeyData
{
    bool        Down;               // True for if key is down
    float       DownDuration;       // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float       DownDurationPrev;   // Last frame duration the key has been down
    float       AnalogValue;        // 0.0f..1.0f for gamepad values
};

struct ImGuiIO
{
    //------------------------------------------------------------------
    //------------------------------------------------------------------

    ImGuiConfigFlags   ConfigFlags;             // = 0              // See ImGuiConfigFlags_ enum. Set by user/application. Keyboard/Gamepad navigation options, etc.
    ImGuiBackendFlags  BackendFlags;            // = 0              // See ImGuiBackendFlags_ enum. Set by backend (imgui_impl_xxx files or custom backend) to communicate features supported by the backend.
    ImVec2      DisplaySize;                    // <unset>          // Main display size, in pixels (== GetMainViewport()->Size). May change every frame.
    ImVec2      DisplayFramebufferScale;        // = (1, 1)         // Main display density. For retina display where window coordinates are different from framebuffer coordinates. This will affect font density + will end up in ImDrawData::FramebufferScale.
    float       DeltaTime;                      // = 1.0f/60.0f     // Time elapsed since last frame, in seconds. May change every frame.
    float       IniSavingRate;                  // = 5.0f           // Minimum time between saving positions/sizes to .ini file, in seconds.
    const char* IniFilename;                    // = "imgui.ini"    // Path to .ini file (important: default "imgui.ini" is relative to current working dir!). Set NULL to disable automatic .ini loading/saving or if you want to manually call LoadIniSettingsXXX() / SaveIniSettingsXXX() functions.
    const char* LogFilename;                    // = "imgui_log.txt"// Path to .log file (default parameter to ImGui::LogToFile when no file is specified).
    void*       UserData;                       // = NULL           // Store your own data.

    ImFontAtlas*Fonts;                          // <auto>           // Font atlas: load, rasterize and pack one or more fonts into a single texture.
    ImFont*     FontDefault;                    // = NULL           // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
    bool        FontAllowUserScaling;           // = false          // Allow user scaling text of individual window with Ctrl+Wheel.

    bool        ConfigNavSwapGamepadButtons;    // = false          // Swap Activate<>Cancel (A<>B) buttons, matching typical "Nintendo/Japanese style" gamepad layout.
    bool        ConfigNavMoveSetMousePos;       // = false          // Directional/tabbing navigation teleports the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is difficult. Will update io.MousePos and set io.WantSetMousePos=true.
    bool        ConfigNavCaptureKeyboard;       // = true           // Sets io.WantCaptureKeyboard when io.NavActive is set.
    bool        ConfigNavEscapeClearFocusItem;  // = true           // Pressing Escape can clear focused item + navigation id/highlight. Set to false if you want to always keep highlight on.
    bool        ConfigNavEscapeClearFocusWindow;// = false          // Pressing Escape can clear focused window as well (super set of io.ConfigNavEscapeClearFocusItem).
    bool        ConfigNavCursorVisibleAuto;     // = true           // Using directional navigation key makes the cursor visible. Mouse click hides the cursor.
    bool        ConfigNavCursorVisibleAlways;   // = false          // Navigation cursor is always visible.

    bool        MouseDrawCursor;                // = false          // Request ImGui to draw a mouse cursor for you (if you are on a platform without a mouse cursor). Cannot be easily renamed to 'io.ConfigXXX' because this is frequently used by backend implementations.
    bool        ConfigMacOSXBehaviors;          // = defined(__APPLE__) // Swap Cmd<>Ctrl keys + OS X style text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
    bool        ConfigInputTrickleEventQueue;   // = true           // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    bool        ConfigInputTextCursorBlink;     // = true           // Enable blinking cursor (optional as some users consider it to be distracting).
    bool        ConfigInputTextEnterKeepActive; // = false          // [BETA] Pressing Enter will keep item active and select contents (single-line only).
    bool        ConfigDragClickToInputText;     // = false          // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.
    bool        ConfigWindowsResizeFromEdges;   // = true           // Enable resizing of windows from their edges and from the lower-left corner. This requires ImGuiBackendFlags_HasMouseCursors for better mouse cursor feedback. (This used to be a per-window ImGuiWindowFlags_ResizeFromAnySide flag)
    bool        ConfigWindowsMoveFromTitleBarOnly;  // = false      // Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.
    bool        ConfigWindowsCopyContentsWithCtrlC; // = false      // [EXPERIMENTAL] Ctrl+C copy the contents of focused window into the clipboard. Experimental because: (1) has known issues with nested Begin/End pairs (2) text output quality varies (3) text output is in submission order rather than spatial order.
    bool        ConfigScrollbarScrollByPage;    // = true           // Enable scrolling page by page when clicking outside the scrollbar grab. When disabled, always scroll to clicked location. When enabled, Shift+Click scrolls to clicked location.
    float       ConfigMemoryCompactTimer;       // = 60.0f          // Timer (in seconds) to free transient windows/tables memory buffers when unused. Set to -1.0f to disable.

    float       MouseDoubleClickTime;           // = 0.30f          // Time for a double-click, in seconds.
    float       MouseDoubleClickMaxDist;        // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float       MouseDragThreshold;             // = 6.0f           // Distance threshold before considering we are dragging.
    float       KeyRepeatDelay;                 // = 0.275f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float       KeyRepeatRate;                  // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    bool        ConfigErrorRecovery;                // = true       // Enable error recovery support. Some errors won't be detected and lead to direct crashes if recovery is disabled.
    bool        ConfigErrorRecoveryEnableAssert;    // = true       // Enable asserts on recoverable error. By default call IM_ASSERT() when returning from a failing IM_ASSERT_USER_ERROR()
    bool        ConfigErrorRecoveryEnableDebugLog;  // = true       // Enable debug log output on recoverable errors.
    bool        ConfigErrorRecoveryEnableTooltip;   // = true       // Enable tooltip on recoverable errors. The tooltip include a way to enable asserts if they were disabled.

    bool        ConfigDebugIsDebuggerPresent;   // = false          // Enable various tools calling IM_DEBUG_BREAK().

    bool        ConfigDebugHighlightIdConflicts;// = true           // Highlight and show an error message popup when multiple items have conflicting identifiers.
    bool        ConfigDebugHighlightIdConflictsShowItemPicker;//=true // Show "Item Picker" button in aforementioned popup.

    bool        ConfigDebugBeginReturnValueOnce;// = false          // First-time calls to Begin()/BeginChild() will return false. NEEDS TO BE SET AT APPLICATION BOOT TIME if you don't want to miss windows.
    bool        ConfigDebugBeginReturnValueLoop;// = false          // Some calls to Begin()/BeginChild() will return false. Will cycle through window depths then repeat. Suggested use: add "io.ConfigDebugBeginReturnValue = io.KeyShift" in your main loop then occasionally press SHIFT. Windows should be flickering while running.

    bool        ConfigDebugIgnoreFocusLoss;     // = false          // Ignore io.AddFocusEvent(false), consequently not calling io.ClearInputKeys()/io.ClearInputMouse() in input processing.

    bool        ConfigDebugIniSettings;         // = false          // Save .ini data with extra comments (particularly helpful for Docking, but makes saving slower)

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    const char* BackendPlatformName;            // = NULL
    const char* BackendRendererName;            // = NULL
    void*       BackendPlatformUserData;        // = NULL           // User data for platform backend
    void*       BackendRendererUserData;        // = NULL           // User data for renderer backend
    void*       BackendLanguageUserData;        // = NULL           // User data for non C++ programming language backend

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    IMGUI_API void  AddKeyEvent(ImGuiKey key, bool down);                   // Queue a new key down/up event. Key should be "translated" (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
    IMGUI_API void  AddKeyAnalogEvent(ImGuiKey key, bool down, float v);    // Queue a new key down/up event for analog values (e.g. ImGuiKey_Gamepad_ values). Dead-zones should be handled by the backend.
    IMGUI_API void  AddMousePosEvent(float x, float y);                     // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    IMGUI_API void  AddMouseButtonEvent(int button, bool down);             // Queue a mouse button change
    IMGUI_API void  AddMouseWheelEvent(float wheel_x, float wheel_y);       // Queue a mouse wheel update. wheel_y<0: scroll down, wheel_y>0: scroll up, wheel_x<0: scroll right, wheel_x>0: scroll left.
    IMGUI_API void  AddMouseSourceEvent(ImGuiMouseSource source);           // Queue a mouse source change (Mouse/TouchScreen/Pen)
    IMGUI_API void  AddFocusEvent(bool focused);                            // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    IMGUI_API void  AddInputCharacter(unsigned int c);                      // Queue a new character input
    IMGUI_API void  AddInputCharacterUTF16(ImWchar16 c);                    // Queue a new character input from a UTF-16 character, it can be a surrogate
    IMGUI_API void  AddInputCharactersUTF8(const char* str);                // Queue a new characters input from a UTF-8 string

    IMGUI_API void  SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index = -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions with native indices + specify native keycode, scancode.
    IMGUI_API void  SetAppAcceptingEvents(bool accepting_events);           // Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
    IMGUI_API void  ClearEventsQueue();                                     // Clear all incoming events.
    IMGUI_API void  ClearInputKeys();                                       // Clear current keyboard/gamepad state + current frame text input buffer. Equivalent to releasing all keys/buttons.
    IMGUI_API void  ClearInputMouse();                                      // Clear current mouse state.

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    bool        WantCaptureMouse;                   // Set when Dear ImGui will use mouse inputs, in this case do not dispatch them to your main game/application (either way, always pass on mouse inputs to imgui). (e.g. unclicked mouse is hovering over an imgui window, widget is active, mouse was clicked over an imgui window, etc.).
    bool        WantCaptureKeyboard;                // Set when Dear ImGui will use keyboard inputs, in this case do not dispatch them to your main game/application (either way, always pass keyboard inputs to imgui). (e.g. InputText active, or an imgui window is focused and navigation is enabled, etc.).
    bool        WantTextInput;                      // Mobile/console: when set, you may display an on-screen keyboard. This is set by Dear ImGui when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
    bool        WantSetMousePos;                    // MousePos has been altered, backend should reposition mouse on next frame. Rarely used! Set only when io.ConfigNavMoveSetMousePos is enabled.
    bool        WantSaveIniSettings;                // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. Important: clear io.WantSaveIniSettings yourself after saving!
    bool        NavActive;                          // Keyboard/Gamepad navigation is currently allowed (will handle ImGuiKey_NavXXX events) = a window is focused and it doesn't use the ImGuiWindowFlags_NoNavInputs flag.
    bool        NavVisible;                         // Keyboard/Gamepad navigation highlight is visible and allowed (will handle ImGuiKey_NavXXX events).
    float       Framerate;                          // Estimate of application framerate (rolling average over 60 frames, based on io.DeltaTime), in frame per second. Solely for convenience. Slow applications may not want to use a moving average or may want to reset underlying buffers occasionally.
    int         MetricsRenderVertices;              // Vertices output during last call to Render()
    int         MetricsRenderIndices;               // Indices output during last call to Render() = number of triangles * 3
    int         MetricsRenderWindows;               // Number of visible windows
    int         MetricsActiveWindows;               // Number of active windows
    ImVec2      MouseDelta;                         // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    ImGuiContext* Ctx;                              // Parent UI context (needs to be set explicitly by parent).

    ImVec2      MousePos;                           // Mouse position, in pixels. Set to ImVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool        MouseDown[5];                       // Mouse buttons: 0=left, 1=right, 2=middle + extras (ImGuiMouseButton_COUNT == 5). Dear ImGui mostly uses left and right buttons. Other buttons allow us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float       MouseWheel;                         // Mouse wheel Vertical: 1 unit scrolls about 5 lines text. >0 scrolls Up, <0 scrolls Down. Hold Shift to turn vertical scroll into horizontal scroll.
    float       MouseWheelH;                        // Mouse wheel Horizontal. >0 scrolls Left, <0 scrolls Right. Most users don't have a mouse with a horizontal wheel, may not be filled by all backends.
    ImGuiMouseSource MouseSource;                   // Mouse actual input peripheral (Mouse/TouchScreen/Pen).
    bool        KeyCtrl;                            // Keyboard modifier down: Ctrl (non-macOS), Cmd (macOS)
    bool        KeyShift;                           // Keyboard modifier down: Shift
    bool        KeyAlt;                             // Keyboard modifier down: Alt
    bool        KeySuper;                           // Keyboard modifier down: Windows/Super (non-macOS), Ctrl (macOS)

    ImGuiKeyChord KeyMods;                          // Key mods flags (any of ImGuiMod_Ctrl/ImGuiMod_Shift/ImGuiMod_Alt/ImGuiMod_Super flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags). Read-only, updated by NewFrame()
    ImGuiKeyData  KeysData[ImGuiKey_NamedKey_COUNT];// Key state for all known keys. MUST use 'key - ImGuiKey_NamedKey_BEGIN' as index. Use IsKeyXXX() functions to access this.
    bool        WantCaptureMouseUnlessPopupClose;   // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    ImVec2      MousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    ImVec2      MouseClickedPos[5];                 // Position at time of clicking
    double      MouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool        MouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool        MouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    ImU16       MouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    ImU16       MouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool        MouseReleased[5];                   // Mouse button went from Down to !Down
    double      MouseReleasedTime[5];               // Time of last released (rarely used! but useful to handle delayed single-click when trying to disambiguate them from double-click).
    bool        MouseDownOwned[5];                  // Track if button was clicked inside a dear imgui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside ImGui bounds.
    bool        MouseDownOwnedUnlessPopupClose[5];  // Track if button was clicked inside a dear imgui window.
    bool        MouseWheelRequestAxisSwap;          // On a non-Mac system, holding Shift requests WheelY to perform the equivalent of a WheelX event. On a Mac system this is already enforced by the system.
    bool        MouseCtrlLeftAsRightClick;          // (OSX) Set to true when the current click was a Ctrl+Click that spawned a simulated right click
    float       MouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float       MouseDownDurationPrev[5];           // Previous time the mouse button has been down
    float       MouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    float       PenPressure;                        // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear ImGui.
    bool        AppFocusLost;                       // Only modify via AddFocusEvent()
    bool        AppAcceptingEvents;                 // Only modify via SetAppAcceptingEvents()
    ImWchar16   InputQueueSurrogate;                // For AddInputCharacterUTF16()
    ImVector<ImWchar> InputQueueCharacters;         // Queue of _characters_ input (obtained by platform backend). Fill using AddInputCharacter() helper.


#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    float       FontGlobalScale;                    // Moved io.FontGlobalScale to style.FontScaleMain in 1.92 (June 2025)

    const char* (*GetClipboardTextFn)(void* user_data);
    void        (*SetClipboardTextFn)(void* user_data, const char* text);
    void*       ClipboardUserData;

#endif

    IMGUI_API   ImGuiIO();
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImGuiInputTextCallbackData
{
    ImGuiContext*       Ctx;            // Parent UI context
    ImGuiInputTextFlags EventFlag;      // One ImGuiInputTextFlags_Callback*    // Read-only
    ImGuiInputTextFlags Flags;          // What user passed to InputText()      // Read-only
    void*               UserData;       // What user passed to InputText()      // Read-only

    ImWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
    ImGuiKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
    char*               Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
    int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
    int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land: == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
    bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
    int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always]
    int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection
    int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

    IMGUI_API ImGuiInputTextCallbackData();
    IMGUI_API void      DeleteChars(int pos, int bytes_count);
    IMGUI_API void      InsertChars(int pos, const char* text, const char* text_end = NULL);
    void                SelectAll()             { SelectionStart = 0; SelectionEnd = BufTextLen; }
    void                ClearSelection()        { SelectionStart = SelectionEnd = BufTextLen; }
    bool                HasSelection() const    { return SelectionStart != SelectionEnd; }
};

struct ImGuiSizeCallbackData
{
    void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints(). Generally store an integer or float in here (need reinterpret_cast<>).
    ImVec2  Pos;            // Read-only.   Window position, for reference.
    ImVec2  CurrentSize;    // Read-only.   Current window size.
    ImVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

struct ImGuiPayload
{
    // Members
    void*           Data;               // Data (copied and owned by dear imgui)
    int             DataSize;           // Data size

    // [Internal]
    ImGuiID         SourceId;           // Source item id
    ImGuiID         SourceParentId;     // Source parent id (if available)
    int             DataFrameCount;     // Data timestamp
    char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
    bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
    bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

    ImGuiPayload()  { Clear(); }
    void Clear()    { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const                  { return Preview; }
    bool IsDelivery() const                 { return Delivery; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define IM_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef IMGUI_USE_WCHAR32
#define IM_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define IM_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

// Usage: static ImGuiOnceUponAFrame oaf; if (oaf) ImGui::Text("This will be called only once per frame");
struct ImGuiOnceUponAFrame
{
    ImGuiOnceUponAFrame() { RefFrame = -1; }
    mutable int RefFrame;
    operator bool() const { int current_frame = ImGui::GetFrameCount(); if (RefFrame == current_frame) return false; RefFrame = current_frame; return true; }
};

struct ImGuiTextFilter
{
    IMGUI_API           ImGuiTextFilter(const char* default_filter = "");
    IMGUI_API bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
    IMGUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
    IMGUI_API void      Build();
    void                Clear()          { InputBuf[0] = 0; Build(); }
    bool                IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct ImGuiTextRange
    {
        const char*     b;
        const char*     e;

        ImGuiTextRange()                                { b = e = NULL; }
        ImGuiTextRange(const char* _b, const char* _e)  { b = _b; e = _e; }
        bool            empty() const                   { return b == e; }
        IMGUI_API void  split(char separator, ImVector<ImGuiTextRange>* out) const;
    };
    char                    InputBuf[256];
    ImVector<ImGuiTextRange>Filters;
    int                     CountGrep;
};

struct ImGuiTextBuffer
{
    ImVector<char>      Buf;
    IMGUI_API static char EmptyString[1];

    ImGuiTextBuffer()   { }
    inline char         operator[](int i) const { IM_ASSERT(Buf.Data != NULL); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; } // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                resize(int size)        { if (Buf.Size > size) Buf.Data[size] = 0; Buf.resize(size ? size + 1 : 0, 0); } // Similar to resize(0) on ImVector: empty string but don't free buffer.
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
    IMGUI_API void      append(const char* str, const char* str_end = NULL);
    IMGUI_API void      appendf(const char* fmt, ...) IM_FMTARGS(2);
    IMGUI_API void      appendfv(const char* fmt, va_list args) IM_FMTLIST(2);
};

struct ImGuiStoragePair
{
    ImGuiID     key;
    union       { int val_i; float val_f; void* val_p; };
    ImGuiStoragePair(ImGuiID _key, int _val)    { key = _key; val_i = _val; }
    ImGuiStoragePair(ImGuiID _key, float _val)  { key = _key; val_f = _val; }
    ImGuiStoragePair(ImGuiID _key, void* _val)  { key = _key; val_p = _val; }
};

struct ImGuiStorage
{
    // [Internal]
    ImVector<ImGuiStoragePair>      Data;

    void                Clear() { Data.clear(); }
    IMGUI_API int       GetInt(ImGuiID key, int default_val = 0) const;
    IMGUI_API void      SetInt(ImGuiID key, int val);
    IMGUI_API bool      GetBool(ImGuiID key, bool default_val = false) const;
    IMGUI_API void      SetBool(ImGuiID key, bool val);
    IMGUI_API float     GetFloat(ImGuiID key, float default_val = 0.0f) const;
    IMGUI_API void      SetFloat(ImGuiID key, float val);
    IMGUI_API void*     GetVoidPtr(ImGuiID key) const; // default_val is NULL
    IMGUI_API void      SetVoidPtr(ImGuiID key, void* val);

    //      float* pvar = ImGui::GetFloatRef(key); ImGui::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    IMGUI_API int*      GetIntRef(ImGuiID key, int default_val = 0);
    IMGUI_API bool*     GetBoolRef(ImGuiID key, bool default_val = false);
    IMGUI_API float*    GetFloatRef(ImGuiID key, float default_val = 0.0f);
    IMGUI_API void**    GetVoidPtrRef(ImGuiID key, void* default_val = NULL);

    IMGUI_API void      BuildSortByKey();
    IMGUI_API void      SetAllInt(int val);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
};

enum ImGuiListClipperFlags_
{
    ImGuiListClipperFlags_None                  = 0,
    ImGuiListClipperFlags_NoSetTableRowCounters = 1 << 0,   // [Internal] Disabled modifying table row counters. Avoid assumption that 1 clipper item == 1 table row.
};

// Usage:
//   ImGuiListClipper clipper;
//           ImGui::Text("line number %d", i);
struct ImGuiListClipper
{
    ImGuiContext*   Ctx;                // Parent UI context
    int             DisplayStart;       // First item to display, updated by each call to Step()
    int             DisplayEnd;         // End of items to display (exclusive)
    int             ItemsCount;         // [Internal] Number of items
    float           ItemsHeight;        // [Internal] Height of item after a first step and item submission can calculate it
    double          StartPosY;          // [Internal] Cursor position at the time of Begin() or after table frozen rows are all processed
    double          StartSeekOffsetY;   // [Internal] Account for frozen rows in a table and initial loss of precision in very large windows.
    void*           TempData;           // [Internal] Internal data
    ImGuiListClipperFlags Flags;        // [Internal] Flags, currently not yet well exposed.

    IMGUI_API ImGuiListClipper();
    IMGUI_API ~ImGuiListClipper();
    IMGUI_API void  Begin(int items_count, float items_height = -1.0f);
    IMGUI_API void  End();             // Automatically called on the last call of Step() that returns false.
    IMGUI_API bool  Step();            // Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.

    inline void     IncludeItemByIndex(int item_index)                  { IncludeItemsByIndex(item_index, item_index + 1); }
    IMGUI_API void  IncludeItemsByIndex(int item_begin, int item_end);  // item_end is exclusive e.g. use (42, 42+1) to make item 42 never clipped.

    IMGUI_API void  SeekCursorForItem(int item_index);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif
};

#ifdef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
IM_MSVC_RUNTIME_CHECKS_OFF
inline ImVec2  operator*(const ImVec2& lhs, const float rhs)    { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
inline ImVec2  operator/(const ImVec2& lhs, const float rhs)    { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
inline ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
inline ImVec2  operator-(const ImVec2& lhs)                     { return ImVec2(-lhs.x, -lhs.y); }
inline ImVec2& operator*=(ImVec2& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
inline ImVec2& operator/=(ImVec2& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
inline bool    operator==(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool    operator!=(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }
inline ImVec4  operator*(const ImVec4& lhs, const float rhs)    { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
inline ImVec4  operator/(const ImVec4& lhs, const float rhs)    { return ImVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
inline ImVec4  operator+(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline ImVec4  operator-(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline ImVec4  operator*(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline ImVec4  operator/(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
inline ImVec4  operator-(const ImVec4& lhs)                     { return ImVec4(-lhs.x, -lhs.y, -lhs.z, -lhs.w); }
inline bool    operator==(const ImVec4& lhs, const ImVec4& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
inline bool    operator!=(const ImVec4& lhs, const ImVec4& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }
IM_MSVC_RUNTIME_CHECKS_RESTORE
#endif

#ifndef IM_COL32_R_SHIFT
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT    16
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    0
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#else
#define IM_COL32_R_SHIFT    0
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    16
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#endif
#endif
#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
#define IM_COL32_WHITE       IM_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK       IM_COL32(0,0,0,255)        // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0,0,0,0)          // Transparent black = 0x00000000

struct ImColor
{
    ImVec4          Value;

    constexpr ImColor()                                             { }
    constexpr ImColor(float r, float g, float b, float a = 1.0f)    : Value(r, g, b, a) { }
    constexpr ImColor(const ImVec4& col)                            : Value(col) {}
    constexpr ImColor(int r, int g, int b, int a = 255)             : Value((float)r * (1.0f / 255.0f), (float)g * (1.0f / 255.0f), (float)b * (1.0f / 255.0f), (float)a* (1.0f / 255.0f)) {}
    constexpr ImColor(ImU32 rgba)                                   : Value((float)((rgba >> IM_COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)) {}
    inline operator ImU32() const                                   { return ImGui::ColorConvertFloat4ToU32(Value); }
    inline operator ImVec4() const                                  { return Value; }

    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ ImGui::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z); Value.w = a; }
    static ImColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b); return ImColor(r, g, b, a); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Usage:

enum ImGuiMultiSelectFlags_
{
    ImGuiMultiSelectFlags_None                  = 0,
    ImGuiMultiSelectFlags_SingleSelect          = 1 << 0,   // Disable selecting more than one item. This is available to allow single-selection code to share same code/logic if desired. It essentially disables the main purpose of BeginMultiSelect() tho!
    ImGuiMultiSelectFlags_NoSelectAll           = 1 << 1,   // Disable Ctrl+A shortcut to select all.
    ImGuiMultiSelectFlags_NoRangeSelect         = 1 << 2,   // Disable Shift+selection mouse/keyboard support (useful for unordered 2D selection). With BoxSelect is also ensure contiguous SetRange requests are not combined into one. This allows not handling interpolation in SetRange requests.
    ImGuiMultiSelectFlags_NoAutoSelect          = 1 << 3,   // Disable selecting items when navigating (useful for e.g. supporting range-select in a list of checkboxes).
    ImGuiMultiSelectFlags_NoAutoClear           = 1 << 4,   // Disable clearing selection when navigating or selecting another one (generally used with ImGuiMultiSelectFlags_NoAutoSelect. useful for e.g. supporting range-select in a list of checkboxes).
    ImGuiMultiSelectFlags_NoAutoClearOnReselect = 1 << 5,   // Disable clearing selection when clicking/selecting an already selected item.
    ImGuiMultiSelectFlags_BoxSelect1d           = 1 << 6,   // Enable box-selection with same width and same x pos items (e.g. full row Selectable()). Box-selection works better with little bit of spacing between items hit-box in order to be able to aim at empty space.
    ImGuiMultiSelectFlags_BoxSelect2d           = 1 << 7,   // Enable box-selection with varying width or varying x pos items support (e.g. different width labels, or 2D layout/grid). This is slower: alters clipping logic so that e.g. horizontal movements will update selection of normally clipped items.
    ImGuiMultiSelectFlags_BoxSelectNoScroll     = 1 << 8,   // Disable scrolling when box-selecting near edges of scope.
    ImGuiMultiSelectFlags_ClearOnEscape         = 1 << 9,   // Clear selection when pressing Escape while scope is focused.
    ImGuiMultiSelectFlags_ClearOnClickVoid      = 1 << 10,  // Clear selection when clicking on empty location within scope.
    ImGuiMultiSelectFlags_ScopeWindow           = 1 << 11,  // Scope for _BoxSelect and _ClearOnClickVoid is whole window (Default). Use if BeginMultiSelect() covers a whole window or used a single time in same window.
    ImGuiMultiSelectFlags_ScopeRect             = 1 << 12,  // Scope for _BoxSelect and _ClearOnClickVoid is rectangle encompassing BeginMultiSelect()/EndMultiSelect(). Use if BeginMultiSelect() is called multiple times in same window.
    ImGuiMultiSelectFlags_SelectOnClick         = 1 << 13,  // Apply selection on mouse down when clicking on unselected item. (Default)
    ImGuiMultiSelectFlags_SelectOnClickRelease  = 1 << 14,  // Apply selection on mouse release when clicking an unselected item. Allow dragging an unselected item without altering selection.
    ImGuiMultiSelectFlags_NavWrapX              = 1 << 16,  // [Temporary] Enable navigation wrapping on X axis. Provided as a convenience because we don't have a design for the general Nav API for this yet. When the more general feature be public we may obsolete this flag in favor of new one.
    ImGuiMultiSelectFlags_NoSelectOnRightClick  = 1 << 17,  // Disable default right-click processing, which selects item on mouse down, and is designed for context-menus.
};

struct ImGuiMultiSelectIO
{
    ImVector<ImGuiSelectionRequest> Requests;   //  ms:w, app:r     /  ms:w  app:r   // Requests to apply to your selection data.
    ImGuiSelectionUserData      RangeSrcItem;   //  ms:w  app:r     /                // (If using clipper) Begin: Source item (often the first selected item) must never be clipped: use clipper.IncludeItemByIndex() to ensure it is submitted.
    ImGuiSelectionUserData      NavIdItem;      //  ms:w, app:r     /                // (If using deletion) Last known SetNextItemSelectionUserData() value for NavId (if part of submitted items).
    bool                        NavIdSelected;  //  ms:w, app:r     /        app:r   // (If using deletion) Last known selection state for NavId (if part of submitted items).
    bool                        RangeSrcReset;  //        app:w     /  ms:r          // (If using deletion) Set before EndMultiSelect() to reset ResetSrcItem (e.g. if deleted selection).
    int                         ItemsCount;     //  ms:w, app:r     /        app:r   // 'int items_count' parameter to BeginMultiSelect() is copied here for convenience, allowing simpler calls to your ApplyRequests handler. Not used internally.
};

enum ImGuiSelectionRequestType
{
    ImGuiSelectionRequestType_None = 0,
    ImGuiSelectionRequestType_SetAll,           // Request app to clear selection (if Selected==false) or select all items (if Selected==true). We cannot set RangeFirstItem/RangeLastItem as its contents is entirely up to user (not necessarily an index)
    ImGuiSelectionRequestType_SetRange,         // Request app to select/unselect [RangeFirstItem..RangeLastItem] items (inclusive) based on value of Selected. Only EndMultiSelect() request this, app code can read after BeginMultiSelect() and it will always be false.
};

struct ImGuiSelectionRequest
{
    ImGuiSelectionRequestType   Type;           //  ms:w, app:r     /  ms:w, app:r   // Request type. You'll most often receive 1 Clear + 1 SetRange with a single-item range.
    bool                        Selected;       //  ms:w, app:r     /  ms:w, app:r   // Parameter for SetAll/SetRange requests (true = select, false = unselect)
    ImS8                        RangeDirection; //                  /  ms:w  app:r   // Parameter for SetRange request: +1 when RangeFirstItem comes before RangeLastItem, -1 otherwise. Useful if you want to preserve selection order on a backward Shift+Click.
    ImGuiSelectionUserData      RangeFirstItem; //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from top to bottom).
    ImGuiSelectionUserData      RangeLastItem;  //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from bottom to top). Inclusive!
};

struct ImGuiSelectionBasicStorage
{
    // Members
    int             Size;           //          // Number of selected items, maintained by this helper.
    bool            PreserveOrder;  // = false  // GetNextSelectedItem() will return ordered selection (currently implemented by two additional sorts of selection. Could be improved)
    void*           UserData;       // = NULL   // User data for use by adapter function        // e.g. selection.UserData = (void*)my_items;
    ImGuiID         (*AdapterIndexToStorageId)(ImGuiSelectionBasicStorage* self, int idx);      // e.g. selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) { return ((MyItems**)self->UserData)[idx]->ID; };
    int             _SelectionOrder;// [Internal] Increasing counter to store selection order
    ImGuiStorage    _Storage;       // [Internal] Selection set. Think of this as similar to e.g. std::set<ImGuiID>. Prefer not accessing directly: iterate with GetNextSelectedItem().

    // Methods
    IMGUI_API ImGuiSelectionBasicStorage();
    IMGUI_API void  ApplyRequests(ImGuiMultiSelectIO* ms_io);   // Apply selection requests coming from BeginMultiSelect() and EndMultiSelect() functions. It uses 'items_count' passed to BeginMultiSelect()
    IMGUI_API bool  Contains(ImGuiID id) const;                 // Query if an item id is in selection.
    IMGUI_API void  Clear();                                    // Clear selection
    IMGUI_API void  Swap(ImGuiSelectionBasicStorage& r);        // Swap two selections
    IMGUI_API void  SetItemSelected(ImGuiID id, bool selected); // Add/remove an item from selection (generally done by ApplyRequests() function)
    IMGUI_API bool  GetNextSelectedItem(void** opaque_it, ImGuiID* out_id); // Iterate selection with 'void* it = NULL; ImGuiID id; while (selection.GetNextSelectedItem(&it, &id)) { ... }'
    inline ImGuiID  GetStorageIdFromIndex(int idx)              { return AdapterIndexToStorageId(this, idx); }  // Convert index to item id based on provided adapter.
};

struct ImGuiSelectionExternalStorage
{
    // Members
    void*           UserData;       // User data for use by adapter function                                // e.g. selection.UserData = (void*)my_items;
    void            (*AdapterSetItemSelected)(ImGuiSelectionExternalStorage* self, int idx, bool selected); // e.g. AdapterSetItemSelected = [](ImGuiSelectionExternalStorage* self, int idx, bool selected) { ((MyItems**)self->UserData)[idx]->Selected = selected; }

    // Methods
    IMGUI_API ImGuiSelectionExternalStorage();
    IMGUI_API void  ApplyRequests(ImGuiMultiSelectIO* ms_io);   // Apply selection requests by using AdapterSetItemSelected() calls
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX     (32)
#endif

#ifndef ImDrawIdx
typedef unsigned short ImDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
#endif

#ifndef ImDrawCallback
typedef void (*ImDrawCallback)(const ImDrawList* parent_list, const ImDrawCmd* cmd);
#endif

#define ImDrawCallback_ResetRenderState     (ImDrawCallback)(-8)

struct ImDrawCmd
{
    ImVec4          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    ImTextureRef    TexRef;             // 16   // Reference to a font/texture atlas (where backend called ImTextureData::SetTexID()) or to a user-provided texture ID (via e.g. ImGui::Image() calls). Both will lead to a ImTextureID value.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[] array, indices in idx_buffer[].
    ImDrawCallback  UserCallback;       // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData;   // 4-8  // Callback user data (when UserCallback != NULL). If called AddCallback() with size == 0, this is a copy of the AddCallback() argument. If called AddCallback() with size > 0, this is pointing to a buffer where data is stored.
    int             UserCallbackDataSize;  // 4 // Size of callback user data when using storage, otherwise 0.
    int             UserCallbackDataOffset;// 4 // [Internal] Offset of callback user data when using storage, otherwise -1.

    ImDrawCmd()     { memset(this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    inline ImTextureID GetTexID() const;    // == (TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID)
};

#ifndef IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct ImDrawVert
{
    ImVec2  pos;
    ImVec2  uv;
    ImU32   col;
};
#else
IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#endif

struct ImDrawCmdHeader
{
    ImVec4          ClipRect;
    ImTextureRef    TexRef;
    unsigned int    VtxOffset;
};

struct ImDrawChannel
{
    ImVector<ImDrawCmd>         _CmdBuffer;
    ImVector<ImDrawIdx>         _IdxBuffer;
};

struct ImDrawListSplitter
{
    int                         _Current;    // Current channel number (0)
    int                         _Count;      // Number of active channels (1+)
    ImVector<ImDrawChannel>     _Channels;   // Draw channels (not resized down so _Count might be < Channels.Size)

    inline ImDrawListSplitter()  { memset(this, 0, sizeof(*this)); }
    inline ~ImDrawListSplitter() { ClearFreeMemory(); }
    inline void                 Clear() { _Current = 0; _Count = 1; } // Do not clear Channels[] so our allocations are reused next frame
    IMGUI_API void              ClearFreeMemory();
    IMGUI_API void              Split(ImDrawList* draw_list, int count);
    IMGUI_API void              Merge(ImDrawList* draw_list);
    IMGUI_API void              SetCurrentChannel(ImDrawList* draw_list, int channel_idx);
};

enum ImDrawFlags_
{
    ImDrawFlags_None                        = 0,
    ImDrawFlags_Closed                      = 1 << 0, // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
    ImDrawFlags_RoundCornersTopLeft         = 1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
    ImDrawFlags_RoundCornersTopRight        = 1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
    ImDrawFlags_RoundCornersBottomLeft      = 1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
    ImDrawFlags_RoundCornersBottomRight     = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
    ImDrawFlags_RoundCornersNone            = 1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
    ImDrawFlags_RoundCornersTop             = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersBottom          = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersLeft            = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
    ImDrawFlags_RoundCornersRight           = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersAll             = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersDefault_        = ImDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
    ImDrawFlags_RoundCornersMask_           = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone,
};

enum ImDrawListFlags_
{
    ImDrawListFlags_None                    = 0,
    ImDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    ImDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    ImDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    ImDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
};

struct ImDrawList
{
    ImVector<ImDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    ImVector<ImDrawIdx>     IdxBuffer;          // Index buffer. Each command consume ImDrawCmd::ElemCount of those
    ImVector<ImDrawVert>    VtxBuffer;          // Vertex buffer.
    ImDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

    unsigned int            _VtxCurrentIdx;     // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    ImDrawListSharedData*   _Data;              // Pointer to shared draw data (you can use ImGui::GetDrawListSharedData() to get the one from current ImGui context)
    ImDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImVector<ImVec2>        _Path;              // [Internal] current path building
    ImDrawCmdHeader         _CmdHeader;         // [Internal] template of active commands. Fields should match those of CmdBuffer.back().
    ImDrawListSplitter      _Splitter;          // [Internal] for channels api (note: prefer using your own persistent instance of ImDrawListSplitter!)
    ImVector<ImVec4>        _ClipRectStack;     // [Internal]
    ImVector<ImTextureRef>  _TextureStack;      // [Internal]
    ImVector<ImU8>          _CallbacksDataBuf;  // [Internal]
    float                   _FringeScale;       // [Internal] anti-alias fringe is scaled by this value, this helps to keep things sharp while zooming at vertex buffer content
    const char*             _OwnerName;         // Pointer to owner window's name for debugging

    IMGUI_API ImDrawList(ImDrawListSharedData* shared_data);
    IMGUI_API ~ImDrawList();

    IMGUI_API void  PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
    IMGUI_API void  PushClipRectFullScreen();
    IMGUI_API void  PopClipRect();
    IMGUI_API void  PushTexture(ImTextureRef tex_ref);
    IMGUI_API void  PopTexture();
    inline ImVec2   GetClipRectMin() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.x, cr.y); }
    inline ImVec2   GetClipRectMax() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.z, cr.w); }

    // Primitives
    IMGUI_API void  AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
    IMGUI_API void  AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f);   // a: upper-left, b: lower-right (== upper-left + size)
    IMGUI_API void  AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0);                     // a: upper-left, b: lower-right (== upper-left + size)
    IMGUI_API void  AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
    IMGUI_API void  AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness = 1.0f);
    IMGUI_API void  AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col);
    IMGUI_API void  AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness = 1.0f);
    IMGUI_API void  AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col);
    IMGUI_API void  AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
    IMGUI_API void  AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments = 0);
    IMGUI_API void  AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness = 1.0f);
    IMGUI_API void  AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments);
    IMGUI_API void  AddEllipse(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
    IMGUI_API void  AddEllipseFilled(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0);
    IMGUI_API void  AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL);
    IMGUI_API void  AddText(ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
    IMGUI_API void  AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    IMGUI_API void  AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

    IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
    IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col);
    IMGUI_API void  AddConcavePolyFilled(const ImVec2* points, int num_points, ImU32 col);

    IMGUI_API void  AddImage(ImTextureRef tex_ref, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min = ImVec2(0, 0), const ImVec2& uv_max = ImVec2(1, 1), ImU32 col = IM_COL32_WHITE);
    IMGUI_API void  AddImageQuad(ImTextureRef tex_ref, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1 = ImVec2(0, 0), const ImVec2& uv2 = ImVec2(1, 0), const ImVec2& uv3 = ImVec2(1, 1), const ImVec2& uv4 = ImVec2(0, 1), ImU32 col = IM_COL32_WHITE);
    IMGUI_API void  AddImageRounded(ImTextureRef tex_ref, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawFlags flags = 0);

    inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const ImVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const ImVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(ImU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathFillConcave(ImU32 col)                                  { AddConcavePolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(ImU32 col, ImDrawFlags flags = 0, float thickness = 1.0f) { AddPolyline(_Path.Data, _Path.Size, col, flags, thickness); _Path.Size = 0; }
    IMGUI_API void  PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
    IMGUI_API void  PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
    IMGUI_API void  PathEllipticalArcTo(const ImVec2& center, const ImVec2& radius, float rot, float a_min, float a_max, int num_segments = 0); // Ellipse
    IMGUI_API void  PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
    IMGUI_API void  PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
    IMGUI_API void  PathRect(const ImVec2& rect_min, const ImVec2& rect_max, float rounding = 0.0f, ImDrawFlags flags = 0);

    IMGUI_API void  AddCallback(ImDrawCallback callback, void* userdata, size_t userdata_size = 0);

    IMGUI_API void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
    IMGUI_API ImDrawList* CloneOutput() const;                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer. For multi-threaded rendering, consider using `imgui_threaded_rendering` from https://github.com/ocornut/imgui_club instead.

    inline void     ChannelsSplit(int count)    { _Splitter.Split(this, count); }
    inline void     ChannelsMerge()             { _Splitter.Merge(this); }
    inline void     ChannelsSetCurrent(int n)   { _Splitter.SetCurrentChannel(this, n); }

    IMGUI_API void  PrimReserve(int idx_count, int vtx_count);
    IMGUI_API void  PrimUnreserve(int idx_count, int vtx_count);
    IMGUI_API void  PrimRect(const ImVec2& a, const ImVec2& b, ImU32 col);      // Axis aligned rectangle (composed of two triangles)
    IMGUI_API void  PrimRectUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col);
    IMGUI_API void  PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col);
    inline    void  PrimWriteVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col)    { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(ImDrawIdx idx)                                     { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col)         { PrimWriteIdx((ImDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); } // Write vertex with unique index

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void  PushTextureID(ImTextureRef tex_ref) { PushTexture(tex_ref); }   // RENAMED in 1.92.0
    inline    void  PopTextureID()                      { PopTexture(); }           // RENAMED in 1.92.0
#endif

    IMGUI_API void  _SetDrawListSharedData(ImDrawListSharedData* data);
    IMGUI_API void  _ResetForNewFrame();
    IMGUI_API void  _ClearFreeMemory();
    IMGUI_API void  _PopUnusedDrawCmd();
    IMGUI_API void  _TryMergeDrawCmds();
    IMGUI_API void  _OnChangedClipRect();
    IMGUI_API void  _OnChangedTexture();
    IMGUI_API void  _OnChangedVtxOffset();
    IMGUI_API void  _SetTexture(ImTextureRef tex_ref);
    IMGUI_API int   _CalcCircleAutoSegmentCount(float radius) const;
    IMGUI_API void  _PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
    IMGUI_API void  _PathArcToN(const ImVec2& center, float radius, float a_min, float a_max, int num_segments);
};

struct ImDrawData
{
    bool                Valid;              // Only valid after Render() is called and before the next NewFrame() is called.
    int                 CmdListsCount;      // == CmdLists.Size. (OBSOLETE: exists for legacy reasons). Number of ImDrawList* to render.
    int                 TotalIdxCount;      // For convenience, sum of all ImDrawList's IdxBuffer.Size
    int                 TotalVtxCount;      // For convenience, sum of all ImDrawList's VtxBuffer.Size
    ImVector<ImDrawList*> CmdLists;         // Array of ImDrawList* to render. The ImDrawLists are owned by ImGuiContext and only pointed to from here.
    ImVec2              DisplayPos;         // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    ImVec2              DisplaySize;        // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    ImVec2              FramebufferScale;   // Amount of pixels for each unit of DisplaySize. Copied from viewport->FramebufferScale (== io.DisplayFramebufferScale for main viewport). Generally (1,1) on normal display, (2,2) on OSX with Retina display.
    ImGuiViewport*      OwnerViewport;      // Viewport carrying the ImDrawData instance, might be of use to the renderer (generally not).
    ImVector<ImTextureData*>* Textures;     // List of textures to update. Most of the times the list is shared by all ImDrawData, has only 1 texture and it doesn't need any update. This almost always points to ImGui::GetPlatformIO().Textures[]. May be overridden or set to NULL if you want to manually update textures.

    // Functions
    ImDrawData()    { Clear(); }
    IMGUI_API void  Clear();
    IMGUI_API void  AddDrawList(ImDrawList* draw_list);     // Helper to add an external draw list into an existing ImDrawData.
    IMGUI_API void  DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
    IMGUI_API void  ScaleClipRects(const ImVec2& fb_scale); // Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than Dear ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#undef Status // X11 headers are leaking this.

enum ImTextureFormat
{
    ImTextureFormat_RGBA32,         // 4 components per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    ImTextureFormat_Alpha8,         // 1 component per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight
};

enum ImTextureStatus
{
    ImTextureStatus_OK,
    ImTextureStatus_Destroyed,      // Backend destroyed the texture.
    ImTextureStatus_WantCreate,     // Requesting backend to create the texture. Set status OK when done.
    ImTextureStatus_WantUpdates,    // Requesting backend to update specific blocks of pixels (write to texture portions which have never been used before). Set status OK when done.
    ImTextureStatus_WantDestroy,    // Requesting backend to destroy the texture. Set status to Destroyed when done.
};

struct ImTextureRect
{
    unsigned short      x, y;       // Upper-left coordinates of rectangle to update
    unsigned short      w, h;       // Size of rectangle to update (in pixels)
};

struct ImTextureData
{
    int                 UniqueID;               // w    -   // [DEBUG] Sequential index to facilitate identifying a texture when debugging/printing. Unique per atlas.
    ImTextureStatus     Status;                 // rw   rw  // ImTextureStatus_OK/_WantCreate/_WantUpdates/_WantDestroy. Always use SetStatus() to modify!
    void*               BackendUserData;        // -    rw  // Convenience storage for backend. Some backends may have enough with TexID.
    ImTextureID         TexID;                  // r    w   // Backend-specific texture identifier. Always use SetTexID() to modify! The identifier will stored in ImDrawCmd::GetTexID() and passed to backend's RenderDrawData function.
    ImTextureFormat     Format;                 // w    r   // ImTextureFormat_RGBA32 (default) or ImTextureFormat_Alpha8
    int                 Width;                  // w    r   // Texture width
    int                 Height;                 // w    r   // Texture height
    int                 BytesPerPixel;          // w    r   // 4 or 1
    unsigned char*      Pixels;                 // w    r   // Pointer to buffer holding 'Width*Height' pixels and 'Width*Height*BytesPerPixels' bytes.
    ImTextureRect       UsedRect;               // w    r   // Bounding box encompassing all past and queued Updates[].
    ImTextureRect       UpdateRect;             // w    r   // Bounding box encompassing all queued Updates[].
    ImVector<ImTextureRect> Updates;            // w    r   // Array of individual updates.
    int                 UnusedFrames;           // w    r   // In order to facilitate handling Status==WantDestroy in some backend: this is a count successive frames where the texture was not used. Always >0 when Status==WantDestroy.
    unsigned short      RefCount;               // w    r   // Number of contexts using this texture. Used during backend shutdown.
    bool                UseColors;              // w    r   // Tell whether our texture data is known to use colors (rather than just white + alpha).
    bool                WantDestroyNextFrame;   // rw   -   // [Internal] Queued to set ImTextureStatus_WantDestroy next frame. May still be used in the current frame.

    // Functions
    ImTextureData()     { memset(this, 0, sizeof(*this)); Status = ImTextureStatus_Destroyed; TexID = ImTextureID_Invalid; }
    ~ImTextureData()    { DestroyPixels(); }
    IMGUI_API void      Create(ImTextureFormat format, int w, int h);
    IMGUI_API void      DestroyPixels();
    void*               GetPixels()                 { IM_ASSERT(Pixels != NULL); return Pixels; }
    void*               GetPixelsAt(int x, int y)   { IM_ASSERT(Pixels != NULL); return Pixels + (x + y * Width) * BytesPerPixel; }
    int                 GetSizeInBytes() const      { return Width * Height * BytesPerPixel; }
    int                 GetPitch() const            { return Width * BytesPerPixel; }
    ImTextureRef        GetTexRef()                 { ImTextureRef tex_ref; tex_ref._TexData = this; tex_ref._TexID = ImTextureID_Invalid; return tex_ref; }
    ImTextureID         GetTexID() const            { return TexID; }

    void    SetTexID(ImTextureID tex_id)            { TexID = tex_id; }
    void    SetStatus(ImTextureStatus status)       { Status = status; if (status == ImTextureStatus_Destroyed && !WantDestroyNextFrame) Status = ImTextureStatus_WantCreate; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImFontConfig
{
    char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
    void*           FontData;               //          // TTF/OTF data
    int             FontDataSize;           //          // TTF/OTF data size
    bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the owner ImFontAtlas (will delete memory itself).

    // Options
    bool            MergeMode;              // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
    bool            PixelSnapV;             // true     // Align Scaled GlyphOffset.y to pixel boundaries.
    ImS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
    ImS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
    ImWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
    float           SizePixels;             //          // Size in pixels for rasterizer (more or less maps to the resulting font height).
    const ImWchar*  GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
    const ImWchar*  GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
    ImVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
    float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
    float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
    float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
    ImU32           FontNo;                 // 0        // Index of font within TTF/OTF file
    unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
    float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when ImGuiBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.

    // [Internal]
    ImFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
    ImFont*         DstFont;                // Target font (as we merging fonts, multiple ImFontConfig may target the same font)
    const ImFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in ImFontAtlas)
    void*           FontLoaderData;         // Font loader opaque storage (per font config)

    IMGUI_API ImFontConfig();
};

struct ImFontGlyph
{
    unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int    SourceIdx : 4;      // Index of source in parent font
    unsigned int    Codepoint : 26;     // 0x0000..0x10FFFF
    float           AdvanceX;           // Horizontal distance to advance cursor/layout position.
    float           X0, Y0, X1, Y1;     // Glyph corners. Offsets from current cursor/layout position.
    float           U0, V0, U1, V1;     // Texture coordinates for the current value of ImFontAtlas->TexRef. Cached equivalent of calling GetCustomRect() with PackId.
    int             PackId;             // [Internal] ImFontAtlasRectId value (FIXME: Cold data, could be moved elsewhere?)

    ImFontGlyph()   { memset(this, 0, sizeof(*this)); PackId = -1; }
};

struct ImFontGlyphRangesBuilder
{
    ImVector<ImU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)

    ImFontGlyphRangesBuilder()              { Clear(); }
    inline void     Clear()                 { int size_in_bytes = (IM_UNICODE_CODEPOINT_MAX + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(ImU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
    inline bool     GetBit(size_t n) const  { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
    inline void     SetBit(size_t n)        { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
    inline void     AddChar(ImWchar c)      { SetBit(c); }                      // Add character
    IMGUI_API void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
    IMGUI_API void  AddRanges(const ImWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(ImFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
    IMGUI_API void  BuildRanges(ImVector<ImWchar>* out_ranges);                 // Output new ranges
};

typedef int ImFontAtlasRectId;
#define ImFontAtlasRectId_Invalid -1

struct ImFontAtlasRect
{
    unsigned short  x, y;               // Position (in current texture)
    unsigned short  w, h;               // Size
    ImVec2          uv0, uv1;           // UV coordinates (in current texture)

    ImFontAtlasRect() { memset(this, 0, sizeof(*this)); }
};

enum ImFontAtlasFlags_
{
    ImFontAtlasFlags_None               = 0,
    ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    ImFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    ImFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

struct ImFontAtlas
{
    IMGUI_API ImFontAtlas();
    IMGUI_API ~ImFontAtlas();
    IMGUI_API ImFont*           AddFont(const ImFontConfig* font_cfg);
    IMGUI_API ImFont*           AddFontDefault(const ImFontConfig* font_cfg = NULL);
    IMGUI_API ImFont*           AddFontFromFileTTF(const char* filename, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
    IMGUI_API ImFont*           AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
    IMGUI_API ImFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
    IMGUI_API ImFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
    IMGUI_API void              RemoveFont(ImFont* font);

    IMGUI_API void              Clear();                    // Clear everything (input fonts, output glyphs/textures).
    IMGUI_API void              CompactCache();             // Compact cached glyphs and texture.
    IMGUI_API void              SetFontLoader(const ImFontLoader* font_loader); // Change font loader at runtime.

    IMGUI_API void              ClearInputData();           // [OBSOLETE] Clear input data (all ImFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
    IMGUI_API void              ClearFonts();               // [OBSOLETE] Clear input+output font data (same as ClearInputData() + glyphs storage, UV coordinates).
    IMGUI_API void              ClearTexData();             // [OBSOLETE] Clear CPU-side copy of the texture data. Saves RAM once the texture has been copied to graphics memory.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_API bool  Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
    IMGUI_API void  GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 1 byte per-pixel
    IMGUI_API void  GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
    void            SetTexID(ImTextureID id)    { IM_ASSERT(TexRef._TexID == ImTextureID_Invalid); TexRef._TexData->TexID = id; }                               // Called by legacy backends. May be called before texture creation.
    void            SetTexID(ImTextureRef id)   { IM_ASSERT(TexRef._TexID == ImTextureID_Invalid && id._TexData == NULL); TexRef._TexData->TexID = id._TexID; } // Called by legacy backends.
    bool            IsBuilt() const { return Fonts.Size > 0 && TexIsBuilt; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent..
#endif

    //-------------------------------------------
    //-------------------------------------------

    IMGUI_API const ImWchar*    GetGlyphRangesDefault();                // Basic Latin, Extended Latin
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_API const ImWchar*    GetGlyphRangesGreek();                  // Default + Greek and Coptic
    IMGUI_API const ImWchar*    GetGlyphRangesKorean();                 // Default + Korean characters
    IMGUI_API const ImWchar*    GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
    IMGUI_API const ImWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
    IMGUI_API const ImWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
    IMGUI_API const ImWchar*    GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
    IMGUI_API const ImWchar*    GetGlyphRangesThai();                   // Default + Thai characters
    IMGUI_API const ImWchar*    GetGlyphRangesVietnamese();             // Default + Vietnamese characters
#endif

    //-------------------------------------------
    //-------------------------------------------

    IMGUI_API ImFontAtlasRectId AddCustomRect(int width, int height, ImFontAtlasRect* out_r = NULL);// Register a rectangle. Return -1 (ImFontAtlasRectId_Invalid) on error.
    IMGUI_API void              RemoveCustomRect(ImFontAtlasRectId id);                             // Unregister a rectangle. Existing pixels will stay in texture until resized / garbage collected.
    IMGUI_API bool              GetCustomRect(ImFontAtlasRectId id, ImFontAtlasRect* out_r) const;  // Get rectangle coordinates for current texture. Valid immediately, never store this (read above)!

    //-------------------------------------------
    // Members
    //-------------------------------------------

    // Input
    ImFontAtlasFlags            Flags;              // Build flags (see ImFontAtlasFlags_)
    ImTextureFormat             TexDesiredFormat;   // Desired texture format (default to ImTextureFormat_RGBA32 but may be changed to ImTextureFormat_Alpha8).
    int                         TexGlyphPadding;    // FIXME: Should be called "TexPackPadding". Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    int                         TexMinWidth;        // Minimum desired texture width. Must be a power of two. Default to 512.
    int                         TexMinHeight;       // Minimum desired texture height. Must be a power of two. Default to 128.
    int                         TexMaxWidth;        // Maximum desired texture width. Must be a power of two. Default to 8192.
    int                         TexMaxHeight;       // Maximum desired texture height. Must be a power of two. Default to 8192.
    void*                       UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // Output
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImTextureRef                TexRef;             // Latest texture identifier == TexData->GetTexRef().
#else
    union { ImTextureRef TexRef; ImTextureRef TexID; }; // Latest texture identifier == TexData->GetTexRef(). // RENAMED TexID to TexRef in 1.92.0.
#endif
    ImTextureData*              TexData;            // Latest texture.

    // [Internal]
    ImVector<ImTextureData*>    TexList;            // Texture list (most often TexList.Size == 1). TexData is always == TexList.back(). DO NOT USE DIRECTLY, USE GetDrawData().Textures[]/GetPlatformIO().Textures[] instead!
    bool                        Locked;             // Marked as locked during ImGui::NewFrame()..EndFrame() scope if TexUpdates are not supported. Any attempt to modify the atlas will assert.
    bool                        RendererHasTextures;// Copy of (BackendFlags & ImGuiBackendFlags_RendererHasTextures) from supporting context.
    bool                        TexIsBuilt;         // Set when texture was built matching current font input. Mostly useful for legacy IsBuilt() call.
    bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format or conversion process.
    ImVec2                      TexUvScale;         // = (1.0f/TexData->TexWidth, 1.0f/TexData->TexHeight). May change as new texture gets created.
    ImVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel. May change as new texture gets created.
    ImVector<ImFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
    ImVector<ImFontConfig>      Sources;            // Source/configuration data
    ImVec4                      TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines
    int                         TexNextUniqueID;    // Next value to be stored in TexData->UniqueID
    int                         FontNextUniqueID;   // Next value to be stored in ImFont->FontID
    ImVector<ImDrawListSharedData*> DrawListSharedDatas; // List of users for this atlas. Typically one per Dear ImGui context.
    ImFontAtlasBuilder*         Builder;            // Opaque interface to our data that doesn't need to be public and may be discarded when rebuilding.
    const ImFontLoader*         FontLoader;         // Font loader opaque interface (default to use FreeType when IMGUI_ENABLE_FREETYPE is defined, otherwise default to use stb_truetype). Use SetFontLoader() to change this at runtime.
    const char*                 FontLoaderName;     // Font loader name (for display e.g. in About box) == FontLoader->Name
    void*                       FontLoaderData;     // Font backend opaque storage
    unsigned int                FontLoaderFlags;    // Shared flags (for all fonts) for font loader. THIS IS BUILD IMPLEMENTATION DEPENDENT (e.g. Per-font override is also available in ImFontConfig).
    int                         RefCount;           // Number of contexts using this atlas
    ImGuiContext*               OwnerContext;       // Context which own the atlas will be in charge of updating and destroying it.

    // [Obsolete]
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImFontAtlasRect             TempRect;           // For old GetCustomRectByIndex() API
    inline ImFontAtlasRectId    AddCustomRectRegular(int w, int h)                                                          { return AddCustomRect(w, h); }                             // RENAMED in 1.92.0
    inline const ImFontAtlasRect* GetCustomRectByIndex(ImFontAtlasRectId id)                                                { return GetCustomRect(id, &TempRect) ? &TempRect : NULL; } // OBSOLETED in 1.92.0
    inline void                 CalcCustomRectUV(const ImFontAtlasRect* r, ImVec2* out_uv_min, ImVec2* out_uv_max) const    { *out_uv_min = r->uv0; *out_uv_max = r->uv1; }             // OBSOLETED in 1.92.0
    IMGUI_API ImFontAtlasRectId AddCustomRectFontGlyph(ImFont* font, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));                            // OBSOLETED in 1.92.0: Use custom ImFontLoader in ImFontConfig
    IMGUI_API ImFontAtlasRectId AddCustomRectFontGlyphForSize(ImFont* font, float font_size, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));    // ADDED AND OBSOLETED in 1.92.0
#endif
};

struct ImFontBaked
{
    ImVector<float>             IndexAdvanceX;      // 12-16 // out // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this info, and are often bottleneck in large UI).
    float                       FallbackAdvanceX;   // 4     // out // FindGlyph(FallbackChar)->AdvanceX
    float                       Size;               // 4     // in  // Height of characters/line, set during loading (doesn't change after loading)
    float                       RasterizerDensity;  // 4     // in  // Density this is baked at

    ImVector<ImU16>             IndexLookup;        // 12-16 // out // Sparse. Index glyphs by Unicode code-point.
    ImVector<ImFontGlyph>       Glyphs;             // 12-16 // out // All glyphs.
    int                         FallbackGlyphIndex; // 4     // out // Index of FontFallbackChar

    float                       Ascent, Descent;    // 4+4   // out // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize] (unscaled)
    unsigned int                MetricsTotalSurface:26;// 3  // out // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    unsigned int                WantDestroy:1;         // 0  //     // Queued for destroy
    unsigned int                LoadNoFallback:1;      // 0  //     // Disable loading fallback in lower-level calls.
    unsigned int                LoadNoRenderOnLayout:1;// 0  //     // Enable a two-steps mode where CalcTextSize() calls will load AdvanceX *without* rendering/packing glyphs. Only advantagous if you know that the glyph is unlikely to actually be rendered, otherwise it is slower because we'd do one query on the first CalcTextSize and one query on the first Draw.
    int                         LastUsedFrame;         // 4  //     // Record of that time this was bounds
    ImGuiID                     BakedId;            // 4     //     // Unique ID for this baked storage
    ImFont*                     OwnerFont;          // 4-8   // in  // Parent font
    void*                       FontLoaderDatas;    // 4-8   //     // Font loader opaque storage (per baked font * sources): single contiguous buffer allocated by imgui, passed to loader.

    // Functions
    IMGUI_API ImFontBaked();
    IMGUI_API void              ClearOutputData();
    IMGUI_API ImFontGlyph*      FindGlyph(ImWchar c);               // Return U+FFFD glyph if requested glyph doesn't exists.
    IMGUI_API ImFontGlyph*      FindGlyphNoFallback(ImWchar c);     // Return NULL if glyph doesn't exist
    IMGUI_API float             GetCharAdvance(ImWchar c);
    IMGUI_API bool              IsGlyphLoaded(ImWchar c);
};

enum ImFontFlags_
{
    ImFontFlags_None                    = 0,
    ImFontFlags_NoLoadError             = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
    ImFontFlags_NoLoadGlyphs            = 1 << 2,   // [Internal] Disable loading new glyphs.
    ImFontFlags_LockBakedSizes          = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
};

struct ImFont
{
    ImFontBaked*                LastBaked;          // 4-8   // Cache last bound baked. NEVER USE DIRECTLY. Use GetFontBaked().
    ImFontAtlas*                OwnerAtlas;         // 4-8   // What we have been loaded into.
    ImFontFlags                 Flags;              // 4     // Font flags.
    float                       CurrentRasterizerDensity;    // Current rasterizer density. This is a varying state of the font.

    ImGuiID                     FontId;             // Unique identifier for the font
    float                       LegacySize;         // 4     // in  // Font size passed to AddFont(). Use for old code calling PushFont() expecting to use that size. (use ImGui::GetFontBaked() to get font baked at current bound size).
    ImVector<ImFontConfig*>     Sources;            // 16    // in  // List of sources. Pointers within OwnerAtlas->Sources[]
    ImWchar                     EllipsisChar;       // 2-4   // out // Character used for ellipsis rendering ('...').
    ImWchar                     FallbackChar;       // 2-4   // out // Character used if a glyph isn't found (U+FFFD, '?')
    ImU8                        Used8kPagesMap[(IM_UNICODE_CODEPOINT_MAX+1)/8192/8]; // 1 bytes if ImWchar=ImWchar16, 16 bytes if ImWchar==ImWchar32. Store 1-bit for each block of 4K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.
    bool                        EllipsisAutoBake;   // 1     //     // Mark when the "..." glyph needs to be generated.
    ImGuiStorage                RemapPairs;         // 16    //     // Remapping pairs when using AddRemapChar(), otherwise empty.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    float                       Scale;              // 4     // in  // Legacy base font scale (~1.0f), multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
#endif

    // Methods
    IMGUI_API ImFont();
    IMGUI_API ~ImFont();
    IMGUI_API bool              IsGlyphInFont(ImWchar c);
    bool                        IsLoaded() const                { return OwnerAtlas != NULL; }
    const char*                 GetDebugName() const            { return Sources.Size ? Sources[0]->Name : "<unknown>"; } // Fill ImFontConfig::Name.

    IMGUI_API ImFontBaked*      GetFontBaked(float font_size, float density = -1.0f);  // Get or create baked data for given size
    IMGUI_API ImVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** out_remaining = NULL);
    IMGUI_API const char*       CalcWordWrapPosition(float size, const char* text, const char* text_end, float wrap_width);
    IMGUI_API void              RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, ImWchar c, const ImVec4* cpu_fine_clip = NULL);
    IMGUI_API void              RenderText(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, ImDrawTextFlags flags = 0);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline const char*          CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) { return CalcWordWrapPosition(LegacySize * scale, text, text_end, wrap_width); }
#endif

    IMGUI_API void              ClearOutputData();
    IMGUI_API void              AddRemapChar(ImWchar from_codepoint, ImWchar to_codepoint); // Makes 'from_codepoint' character points to 'to_codepoint' glyph.
    IMGUI_API bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

inline ImTextureID ImTextureRef::GetTexID() const
{
    IM_ASSERT(!(_TexData != NULL && _TexID != ImTextureID_Invalid));
    return _TexData ? _TexData->TexID : _TexID;
}

inline ImTextureID ImDrawCmd::GetTexID() const
{
    ImTextureID tex_id = TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID; // == TexRef.GetTexID() above.
    if (TexRef._TexData != NULL)
        IM_ASSERT(tex_id != ImTextureID_Invalid && "ImDrawCmd is referring to ImTextureData that wasn't uploaded to graphics system. Backend must call ImTextureData::SetTexID() after handling ImTextureStatus_WantCreate request!");
    return tex_id;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum ImGuiViewportFlags_
{
    ImGuiViewportFlags_None                     = 0,
    ImGuiViewportFlags_IsPlatformWindow         = 1 << 0,   // Represent a Platform Window
    ImGuiViewportFlags_IsPlatformMonitor        = 1 << 1,   // Represent a Platform Monitor (unused yet)
    ImGuiViewportFlags_OwnedByApp               = 1 << 2,   // Platform Window: Is created/managed by the application (rather than a dear imgui backend)
};

struct ImGuiViewport
{
    ImGuiID             ID;                     // Unique identifier for the viewport
    ImGuiViewportFlags  Flags;                  // See ImGuiViewportFlags_
    ImVec2              Pos;                    // Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)
    ImVec2              Size;                   // Main Area: Size of the viewport.
    ImVec2              FramebufferScale;       // Density of the viewport for Retina display (always 1,1 on Windows, may be 2,2 etc on macOS/iOS). This will affect font rasterizer density.
    ImVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    ImVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)

    void*               PlatformHandle;         // void* to hold higher-level, platform window handle (e.g. HWND, GLFWWindow*, SDL_Window*)
    void*               PlatformHandleRaw;      // void* to hold lower-level, platform-native window handle (under Win32 this is expected to be a HWND, unused for other platforms)

    ImGuiViewport()     { memset(this, 0, sizeof(*this)); }

    // Helpers
    ImVec2              GetCenter() const       { return ImVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    ImVec2              GetWorkCenter() const   { return ImVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ImGuiPlatformIO
{
    IMGUI_API ImGuiPlatformIO();

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    const char* (*Platform_GetClipboardTextFn)(ImGuiContext* ctx);
    void        (*Platform_SetClipboardTextFn)(ImGuiContext* ctx, const char* text);
    void*       Platform_ClipboardUserData;

    bool        (*Platform_OpenInShellFn)(ImGuiContext* ctx, const char* path);
    void*       Platform_OpenInShellUserData;

    void        (*Platform_SetImeDataFn)(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data);
    void*       Platform_ImeUserData;

    ImWchar     Platform_LocaleDecimalPoint;     // '.'

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    int         Renderer_TextureMaxWidth;
    int         Renderer_TextureMaxHeight;

    void*       Renderer_RenderState;

    //------------------------------------------------------------------
    // Output
    //------------------------------------------------------------------

    ImVector<ImTextureData*>        Textures;           // List of textures used by Dear ImGui (most often 1) + contents of external texture list is automatically appended into this.

    //------------------------------------------------------------------
    // Functions
    //------------------------------------------------------------------

    IMGUI_API void ClearPlatformHandlers();    // Clear all Platform_XXX fields. Typically called on Platform Backend shutdown.
    IMGUI_API void ClearRendererHandlers();    // Clear all Renderer_XXX fields. Typically called on Renderer Backend shutdown.
};

struct ImGuiPlatformImeData
{
    bool    WantVisible;            // A widget wants the IME to be visible.
    bool    WantTextInput;          // A widget wants text input, not necessarily IME to be visible. This is automatically set to the upcoming value of io.WantTextInput.
    ImVec2  InputPos;               // Position of input cursor (for IME).
    float   InputLineHeight;        // Line height (for IME).
    ImGuiID ViewportId;             // ID of platform window/viewport.

    ImGuiPlatformImeData()          { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
namespace ImGui
{
    inline void         PushFont(ImFont* font)                                  { PushFont(font, font ? font->LegacySize : 0.0f); }
    IMGUI_API void      SetWindowFontScale(float scale);                        // Set font scale factor for current window. Prefer using PushFont(NULL, style.FontSizeBase * factor) or use style.FontScaleMain to scale all windows.
    IMGUI_API void      Image(ImTextureRef tex_ref, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col); // <-- 'border_col' was removed in favor of ImGuiCol_ImageBorder. If you use 'tint_col', use ImageWithBg() instead.
    inline void         PushButtonRepeat(bool repeat)                           { PushItemFlag(ImGuiItemFlags_ButtonRepeat, repeat); }
    inline void         PopButtonRepeat()                                       { PopItemFlag(); }
    inline void         PushTabStop(bool tab_stop)                              { PushItemFlag(ImGuiItemFlags_NoTabStop, !tab_stop); }
    inline void         PopTabStop()                                            { PopItemFlag(); }
    IMGUI_API ImVec2    GetContentRegionMax();                                  // Content boundaries max (e.g. window boundaries including scrolling, or current column boundaries). You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    IMGUI_API ImVec2    GetWindowContentRegionMin();                            // Content boundaries min for the window (roughly (0,0)-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    IMGUI_API ImVec2    GetWindowContentRegionMax();                            // Content boundaries max for the window (roughly (0,0)+Size-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    inline bool         BeginChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags window_flags = 0)  { return BeginChild(id, size, ImGuiChildFlags_FrameStyle, window_flags); }
    inline void         EndChildFrame()                                                                     { EndChild(); }
    inline void         ShowStackToolWindow(bool* p_open = NULL)                { ShowIDStackToolWindow(p_open); }
    IMGUI_API bool      Combo(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int popup_max_height_in_items = -1);
    IMGUI_API bool      ListBox(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int height_in_items = -1);

}

typedef ImFontAtlasRect ImFontAtlasCustomRect;
/*struct ImFontAtlasCustomRect
{
    unsigned short  X, Y;           // Output   // Packed position in Atlas
    unsigned short  Width, Height;  // Input    // [Internal] Desired rectangle dimension
    unsigned int    GlyphID:31;     // Input    // [Internal] For custom font glyphs only (ID < 0x110000)
    unsigned int    GlyphColored:1; // Input    // [Internal] For custom font glyphs only: glyph is colored, removed tinting.
    float           GlyphAdvanceX;  // Input    // [Internal] For custom font glyphs only: glyph xadvance
    ImVec2          GlyphOffset;    // Input    // [Internal] For custom font glyphs only: glyph display offset
    ImFont*         Font;           // Input    // [Internal] For custom font glyphs only: target font
    ImFontAtlasCustomRect()         { X = Y = 0xFFFF; Width = Height = 0; GlyphID = 0; GlyphColored = 0; GlyphAdvanceX = 0.0f; GlyphOffset = ImVec2(0, 0); Font = NULL; }
    bool IsPacked() const           { return X != 0xFFFF; }
};*/

//typedef ImDrawFlags ImDrawCornerFlags;
//{
//};

//enum ImGuiModFlags_ { ImGuiModFlags_None = 0, ImGuiModFlags_Ctrl = ImGuiMod_Ctrl, ImGuiModFlags_Shift = ImGuiMod_Shift, ImGuiModFlags_Alt = ImGuiMod_Alt, ImGuiModFlags_Super = ImGuiMod_Super };
//enum ImGuiKeyModFlags_ { ImGuiKeyModFlags_None = 0, ImGuiKeyModFlags_Ctrl = ImGuiMod_Ctrl, ImGuiKeyModFlags_Shift = ImGuiMod_Shift, ImGuiKeyModFlags_Alt = ImGuiMod_Alt, ImGuiKeyModFlags_Super = ImGuiMod_Super };

#define IM_OFFSETOF(_TYPE,_MEMBER)  offsetof(_TYPE, _MEMBER)    // OBSOLETED IN 1.90 (now using C++11 standard version)

#endif // #ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#ifdef IMGUI_DISABLE_METRICS_WINDOW
#error IMGUI_DISABLE_METRICS_WINDOW was renamed to IMGUI_DISABLE_DEBUG_TOOLS, please use new name.
#endif

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_INCLUDE_IMGUI_USER_H
#ifdef IMGUI_USER_H_FILENAME
#include IMGUI_USER_H_FILENAME
#else
#include "imgui_user.h"
#endif
#endif

#endif // #ifndef IMGUI_DISABLE
