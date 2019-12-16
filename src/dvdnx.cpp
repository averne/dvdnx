#include <cstdlib>
#include <cstdio>
#include <memory>
#include <switch.h>

#include "screen.hpp"
#include "utils.hpp"
#include "im_bin.h"

FILE *g_debug_file;

extern "C" {
    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x50000
    char nx_inner_heap[INNER_HEAP_SIZE];

    u32 __nx_nv_transfermem_size = 0x15000;

    void __libnx_initheap(void);
    void __libnx_exception_handler(ThreadExceptionDump *ctx);
    void __appInit(void);
    void __appExit(void);
}

void __libnx_initheap(void) {
    extern char *fake_heap_start;
    extern char *fake_heap_end;
    fake_heap_start = nx_inner_heap;
    fake_heap_end   = nx_inner_heap + sizeof(nx_inner_heap);
}

void __libnx_exception_handler(ThreadExceptionDump *ctx) {
#ifdef DEBUG
    MemoryInfo mem_info; u32 page_info;
    svcQueryMemory(&mem_info, &page_info, ctx->pc.x);
    LOG("%#x exception with pc=%#lx\n", ctx->error_desc, ctx->pc.x - mem_info.addr);
#endif
}

void __appInit(void) {
#ifdef DEBUG
    SERV_INIT(sm);
    SERV_INIT(fs);
    TRY_FATAL(fsdevMountSdmc());
    SERV_EXIT(sm);
#endif
}

void __appExit(void) {
#ifdef DEBUG
    fsdevUnmountAll();
    SERV_EXIT(fs);
#endif
}

int main() {
#ifdef DEBUG
    g_debug_file = fopen("sdmc:/dvdnx.log", "w");
    if (g_debug_file == NULL) fatalThrow(0x00f);
#endif

    LOG("Starting\n");

    auto screen = std::make_shared<Screen>();
    TRY_RETURN(screen->initialize(), 1);
    LOG("Initialized screen\n");

    screen->dequeue();
    // screen->fill(make_color_max_alpha<rgba4444_t>(5, 5, 5));
    screen->map(0, 0, FB_WIDTH, FB_HEIGHT, (rgba4444_t *)im_bin);
    screen->flush();
    LOG("Flushed to framebuffer\n");

    for (auto &&[x, y, dir_x, dir_y]{std::tuple{1.0f, 1.0f, false, false}}; ; dir_x?--x:++x, dir_y?--y:++y) {
        if ((0 >= x) || (x + LAYER_WIDTH  >= SCREEN_WIDTH))  dir_x ^= 1;
        if ((0 >= y) || (y + LAYER_HEIGHT >= SCREEN_HEIGHT)) dir_y ^= 1;
        TRY_RETURN(screen->set_window_pos(x, y), 1);
        svcSleepThread(10'000'000);
    }

    screen->finalize();
    LOG("Exiting\n");
    fclose(g_debug_file);
    return 0;
}
