#include "cv_types.h"
#include "LuaIntf.h"
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>

using namespace LuaIntf;
using namespace CVLib;

namespace CVLib {

/**
 * Read image from file path.
 * Stub implementation - returns dummy image for testing.
 */
Image imread(const std::string& path) {
    // TODO: Real implementation with image decoding library
    // For now, create a 640x480 dummy BGR image
    Image img(640, 480, 3);
    
    // Fill with gradient pattern for testing
    uint8_t* p = img.data();
    for (int y = 0; y < img.getHeight(); y++) {
        for (int x = 0; x < img.getWidth(); x++) {
            int idx = (y * img.getWidth() + x) * 3;
            p[idx + 0] = (y * 255) / img.getHeight();  // B
            p[idx + 1] = (x * 255) / img.getWidth();   // G
            p[idx + 2] = 128;                           // R
        }
    }
    
    return img;
}

/**
 * Convert BGR to RGB color space.
 * Swaps B and R channels in-place.
 */
Image bgr2rgb(const Image& img) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    size_t pixels = img.getWidth() * img.getHeight();
    for (size_t i = 0; i < pixels; i++) {
        dst[i * 3 + 0] = src[i * 3 + 2];  // R
        dst[i * 3 + 1] = src[i * 3 + 1];  // G
        dst[i * 3 + 2] = src[i * 3 + 0];  // B
    }
    
    return result;
}

/**
 * Apply letterbox padding to resize image while maintaining aspect ratio.
 * Returns table with {image=padded_image, pad={top=N, left=N, bottom=N, right=N}}
 * Uses lua_CFunction convention to access lua_State.
 */
int letterbox(lua_State* L) {
    // Get arguments: Image, target_w, target_h
    Image* img = Lua::get<Image*>(L, 1);
    int target_w = lua_tointeger(L, 2);
    int target_h = lua_tointeger(L, 3);
    
    if (!img) {
        return luaL_error(L, "letterbox: expected Image argument");
    }
    
    float scale = std::min(
        static_cast<float>(target_w) / img->getWidth(),
        static_cast<float>(target_h) / img->getHeight()
    );
    
    int new_w = static_cast<int>(img->getWidth() * scale);
    int new_h = static_cast<int>(img->getHeight() * scale);
    
    int pad_left = (target_w - new_w) / 2;
    int pad_top = (target_h - new_h) / 2;
    int pad_right = target_w - new_w - pad_left;
    int pad_bottom = target_h - new_h - pad_top;
    
    // Create padded image with gray background (114)
    Image padded(target_w, target_h, img->getChannels());
    std::memset(padded.data(), 114, padded.size());
    
    // Simple nearest-neighbor resize and copy
    const uint8_t* src = img->data();
    uint8_t* dst = padded.data();
    
    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            int src_x = (x * img->getWidth()) / new_w;
            int src_y = (y * img->getHeight()) / new_h;
            
            int src_idx = (src_y * img->getWidth() + src_x) * 3;
            int dst_idx = ((y + pad_top) * target_w + (x + pad_left)) * 3;
            
            dst[dst_idx + 0] = src[src_idx + 0];
            dst[dst_idx + 1] = src[src_idx + 1];
            dst[dst_idx + 2] = src[src_idx + 2];
        }
    }
    
    // Build return table {image=..., pad={...}}
    LuaRef result = LuaRef::createTable(L);
    result.set("image", padded);
    
    LuaRef pad_info = LuaRef::createTable(L);
    pad_info.set("top", pad_top);
    pad_info.set("left", pad_left);
    pad_info.set("bottom", pad_bottom);
    pad_info.set("right", pad_right);
    
    result.set("pad", pad_info);
    
    result.pushToStack();
    return 1;
}

/**
 * Normalize image pixels: (pixel / 255.0 - mean) / std
 * Returns Lua table of flattened float array (still HWC order).
 * Uses lua_CFunction convention to access lua_State.
 */
int normalize(lua_State* L) {
    // Get Image argument from stack
    Image* img = Lua::get<Image*>(L, 1);
    if (!img) {
        return luaL_error(L, "normalize: expected Image argument");
    }
    
    // Check for empty image
    if (img->empty() || img->getWidth() <= 0 || img->getHeight() <= 0) {
        // Return empty table for empty image
        LuaRef result = LuaRef::createTable(L);
        result.pushToStack();
        return 1;
    }
    
    // YOLOv5 normalization parameters
    const float mean[] = {0.485f, 0.456f, 0.406f};
    const float std[] = {0.229f, 0.224f, 0.225f};
    
    std::vector<float> data;
    data.reserve(img->size());
    
    const uint8_t* src = img->data();
    size_t pixels = img->getWidth() * img->getHeight();
    
    for (size_t i = 0; i < pixels; i++) {
        for (int c = 0; c < 3; c++) {
            float val = src[i * 3 + c] / 255.0f;
            val = (val - mean[c]) / std[c];
            data.push_back(val);
        }
    }
    
    // Convert to Lua table
    LuaRef result = LuaRef::createTable(L);
    for (size_t i = 0; i < data.size(); i++) {
        result[i + 1] = data[i];  // Lua uses 1-based indexing
    }
    
    result.pushToStack();
    return 1;
}

/**
 * Convert HWC (Height-Width-Channel) to CHW (Channel-Height-Width) layout.
 * Input: Image object
 * Output: Lua table of CHW flattened array
 * Uses lua_CFunction convention to access lua_State.
 */
int hwc2chw(lua_State* L) {
    // Get Image argument from stack
    Image* img = Lua::get<Image*>(L, 1);
    if (!img) {
        return luaL_error(L, "hwc2chw: expected Image argument");
    }
    
    // Check for empty image
    if (img->empty() || img->getWidth() <= 0 || img->getHeight() <= 0) {
        // Return empty table for empty image
        LuaRef result = LuaRef::createTable(L);
        result.pushToStack();
        return 1;
    }
    
    int H = img->getHeight();
    int W = img->getWidth();
    int C = img->getChannels();
    
    // First normalize the image
    const float mean[] = {0.485f, 0.456f, 0.406f};
    const float std[] = {0.229f, 0.224f, 0.225f};
    
    std::vector<float> hwc_data;
    hwc_data.reserve(H * W * C);
    
    const uint8_t* src = img->data();
    for (size_t i = 0; i < static_cast<size_t>(H * W); i++) {
        for (int c = 0; c < 3; c++) {
            float val = src[i * 3 + c] / 255.0f;
            val = (val - mean[c]) / std[c];
            hwc_data.push_back(val);
        }
    }
    
    // Then transpose to CHW
    std::vector<float> chw_data(H * W * C);
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < H; h++) {
            for (int w = 0; w < W; w++) {
                int hwc_idx = (h * W + w) * C + c;
                int chw_idx = c * (H * W) + h * W + w;
                chw_data[chw_idx] = hwc_data[hwc_idx];
            }
        }
    }
    
    // Convert to Lua table
    LuaRef result = LuaRef::createTable(L);
    for (size_t i = 0; i < chw_data.size(); i++) {
        result[i + 1] = chw_data[i];  // Lua uses 1-based indexing
    }
    
    result.pushToStack();
    return 1;
}

/**
 * Crop image to specified region.
 */
Image crop(const Image& img, int x, int y, int w, int h) {
    if (x < 0 || y < 0 || x + w > img.getWidth() || y + h > img.getHeight()) {
        throw std::runtime_error("crop: region out of bounds");
    }
    
    Image result(w, h, img.getChannels());
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    for (int row = 0; row < h; row++) {
        const uint8_t* src_row = src + ((y + row) * img.getWidth() + x) * img.getChannels();
        uint8_t* dst_row = dst + row * w * img.getChannels();
        std::memcpy(dst_row, src_row, w * img.getChannels());
    }
    
    return result;
}

/**
 * Resize image using nearest neighbor interpolation.
 */
Image resize(const Image& img, int new_w, int new_h) {
    Image result(new_w, new_h, img.getChannels());
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            int src_x = (x * img.getWidth()) / new_w;
            int src_y = (y * img.getHeight()) / new_h;
            
            int src_idx = (src_y * img.getWidth() + src_x) * img.getChannels();
            int dst_idx = (y * new_w + x) * img.getChannels();
            
            for (int c = 0; c < img.getChannels(); c++) {
                dst[dst_idx + c] = src[src_idx + c];
            }
        }
    }
    
    return result;
}

/**
 * Flip image horizontally.
 */
Image flipHorizontal(const Image& img) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    for (int y = 0; y < img.getHeight(); y++) {
        for (int x = 0; x < img.getWidth(); x++) {
            int src_idx = (y * img.getWidth() + x) * img.getChannels();
            int dst_idx = (y * img.getWidth() + (img.getWidth() - 1 - x)) * img.getChannels();
            
            for (int c = 0; c < img.getChannels(); c++) {
                dst[dst_idx + c] = src[src_idx + c];
            }
        }
    }
    
    return result;
}

/**
 * Flip image vertically.
 */
Image flipVertical(const Image& img) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    for (int y = 0; y < img.getHeight(); y++) {
        const uint8_t* src_row = src + y * img.getWidth() * img.getChannels();
        uint8_t* dst_row = dst + (img.getHeight() - 1 - y) * img.getWidth() * img.getChannels();
        std::memcpy(dst_row, src_row, img.getWidth() * img.getChannels());
    }
    
    return result;
}

/**
 * Convert grayscale to BGR.
 */
Image gray2bgr(const Image& img) {
    if (img.getChannels() != 1) {
        throw std::runtime_error("gray2bgr: image must be grayscale (1 channel)");
    }
    
    Image result(img.getWidth(), img.getHeight(), 3);
    const uint8_t* src = img.data();
    uint8_t* dst = result.data();
    
    for (size_t i = 0; i < img.getWidth() * img.getHeight(); i++) {
        uint8_t gray = src[i];
        dst[i * 3 + 0] = gray;  // B
        dst[i * 3 + 1] = gray;  // G
        dst[i * 3 + 2] = gray;  // R
    }
    
    return result;
}

} // namespace CVLib

/**
 * Register CVLib module to Lua.
 */
extern "C" int luaopen_CVLib(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    
    LuaBinding(mod)
        .beginClass<Image>("Image")
            .addConstructor(LUA_ARGS(_opt<int>, _opt<int>, _opt<int>))
            .addProperty("width", &Image::getWidth)
            .addProperty("height", &Image::getHeight)
            .addProperty("channels", &Image::getChannels)
            .addFunction("empty", &Image::empty)
            .addFunction("clone", &Image::clone)
            .addFunction("copyFrom", &Image::copyFrom)
            .addFunction("fill", &Image::fill)
            .addFunction("at", static_cast<uint8_t(Image::*)(int,int,int)const>(&Image::at))
        .endClass()
        
        .addFunction("imread", &imread)
        .addFunction("bgr2rgb", &bgr2rgb)
        .addFunction("letterbox", &letterbox)
        .addFunction("normalize", &normalize)
        .addFunction("hwc2chw", &hwc2chw)
        .addFunction("crop", &crop)
        .addFunction("resize", &resize)
        .addFunction("flipHorizontal", &flipHorizontal)
        .addFunction("flipVertical", &flipVertical)
        .addFunction("gray2bgr", &gray2bgr);
    
    mod.pushToStack();
    return 1;
}
