#include <switch.h>

#include "color.hpp"
#include "utils.hpp"

#include "screen.hpp"

extern "C" u64 __nx_vi_layer_id;

Result Screen::initialize() {
    LOG("Initializing screen\n");

    SERV_INIT(sm); // sm needed internally by libnx
    TRY_GOTO(viInitialize(ViServiceType_Manager), end);
    TRY_GOTO(viOpenDefaultDisplay(&display), close_serv);
    TRY_GOTO(viCreateManagedLayer(&display, (ViLayerFlags)0, 0, &__nx_vi_layer_id), close_display); // flag 0 allows non-fullscreen layer
    TRY_GOTO(viCreateLayer(&display, &layer), close_managed_layer);
    TRY_GOTO(viSetLayerScalingMode(&layer, ViScalingMode_FitToLayer), close_managed_layer);
    TRY_GOTO(viSetLayerZ(&layer, 100), close_managed_layer); // Arbitrary z index
    TRY_GOTO(viSetLayerSize(&layer, LAYER_WIDTH, LAYER_HEIGHT), close_managed_layer);
    TRY_GOTO(viSetLayerPosition(&layer, LAYER_X, LAYER_Y), close_managed_layer);
    TRY_GOTO(nwindowCreateFromLayer(&this->window, &this->layer), close_managed_layer);
    TRY_GOTO(framebufferCreate(&this->framebuf, &this->window, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_4444, 1), close_window);
    SERV_EXIT(sm);

    return 0;

close_window:
    nwindowClose(&this->window);
close_managed_layer:
    viDestroyManagedLayer(&this->layer);
close_display:
    viCloseDisplay(&this->display);
close_serv:
    viExit();
end:
    return 1;
}

void Screen::finalize() {
    LOG("Finalizing screen\n");
    framebufferClose(&this->framebuf);
    nwindowClose(&this->window);
    viDestroyManagedLayer(&this->layer);
    viCloseDisplay(&this->display);
    viExit();
}
