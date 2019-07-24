#pragma once

#include <array>
#include <algorithm>
#include <switch.h>

#include "color.hpp"

// Handheld and docked
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

// width * height page-aligned
#define FB_BPP    sizeof(rgba4444_t)
#define FB_WIDTH  256 // Aligned (* bpp) to 64
#define FB_HEIGHT 128 // Aligned to 128

#define LAYER_X 0.0f
#define LAYER_Y 0.0f
#define LAYER_WIDTH  FB_WIDTH  // Can be different from fb values (will be up/downscaled)
#define LAYER_HEIGHT FB_HEIGHT

class Screen {
    private:
        ViDisplay   display;
        ViLayer     layer;
        NWindow     window;
        Framebuffer framebuf;

    public:
        Result initialize();
        void   finalize();

        inline u32    get_window_width()  const { return this->window.width; }
        inline u32    get_window_height() const { return this->window.height; }
        inline Result set_window_pos(float x, float y) { return viSetLayerPosition(&this->layer, x, y); }
        inline Result set_window_size(u64 w, u64 h)    { return viSetLayerSize(&this->layer, w, h); }

        inline u32 get_framebuffer_width()  const { return this->framebuf.width_aligned; }
        inline u32 get_framebuffer_height() const { return this->framebuf.height_aligned; }
        inline u32 get_framebuffer_size()   const { return this->framebuf.fb_size; }

        inline void *get_cur_buffer()
            { return (rgba4444_t *)this->framebuf.buf + get_framebuffer_size() * this->window.cur_slot; }

        inline void dequeue() { framebufferBegin(&this->framebuf, NULL); }
        inline void flush()   { framebufferEnd(&this->framebuf); }

        std::array<u32, 4> clamp(u32 x1, u32 y1, u32 x2, u32 y2) const {
            return {
                std::clamp<u32>(x1, 0, get_window_width()),
                std::clamp<u32>(y1, 0, get_window_height()),
                std::clamp<u32>(x2, 0, get_window_width()),
                std::clamp<u32>(y2, 0, get_window_height())
            };
        }

        inline void set_pixel(u32 off,      rgba4444_t color) { ((u16 *)get_cur_buffer())[off] = color.rgba; }
        inline void set_pixel(u32 x, u32 y, rgba4444_t color) { set_pixel(get_pixel_offset(x, y), color); }

        void fill(rgba4444_t color) {
            std::fill((rgba4444_t *)get_cur_buffer(),
                (rgba4444_t *)get_cur_buffer() + get_framebuffer_size(),
                color);
        }
        void map(u32 x1, u32 y1, u32 x2, u32 y2, const rgba4444_t *area) {
            auto [x_min, y_min, x_max, y_max] = clamp(x1, y1, x2, y2);
            for (u32 y=y_min; y<y_max; ++y)
                for (u32 x=x_min; x<x_max; ++x)
                    set_pixel(x, y, *(area++));
        }
        void clear() { fill(0); }

        inline u32 get_pixel_offset(u32 x, u32 y) const {
            // Swizzling pattern:
            //    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
            // -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
            // Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
            constexpr u32 x_mask = (__builtin_ctz(FB_WIDTH) - 1) << 5;
            const u32 swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
            const u32 swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
            return swizzled_x + swizzled_y;
        }
};
