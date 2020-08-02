#pragma once

enum class TextureFormat : int {
    RGB = 3,
    RGBA = 4,
};

struct texture_t {
    size_t size;
    size_t bpp;
    TextureFormat format;
    size_t pixelCount;
    size_t dataSize;
    uint8_t* data;
};

typedef std::unique_ptr<texture_t, std::function<void(texture_t*)>> TextureGuard_t;

texture_t* texture_alloc(size_t size, TextureFormat format);
void texture_free(texture_t* t);

texture_t* texture_copy_matrix(texture_t* t, matrix_t* m);
