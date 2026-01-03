#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

namespace CVLib {

/**
 * Image represents a computer vision image with width, height, and pixel data.
 * Used throughout the preprocessing pipeline.
 */
class Image {
private:
    int width_;
    int height_;
    int channels_;
    std::shared_ptr<std::vector<uint8_t>> data_;

public:
    Image() : width_(0), height_(0), channels_(0) {}
    
    Image(int width, int height, int channels = 3)
        : width_(width), height_(height), channels_(channels) {
        data_ = std::make_shared<std::vector<uint8_t>>(width * height * channels);
    }
    
    // Lua-accessible properties
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getChannels() const { return channels_; }
    
    // Data access
    uint8_t* data() { return data_->data(); }
    const uint8_t* data() const { return data_->data(); }
    size_t size() const { return data_->size(); }
    
    bool empty() const { return width_ == 0 || height_ == 0; }
    
    // Utility methods
    Image clone() const {
        Image img(width_, height_, channels_);
        if (data_) {
            std::copy(data_->begin(), data_->end(), img.data_->begin());
        }
        return img;
    }
    
    void copyFrom(const Image& other) {
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        data_ = std::make_shared<std::vector<uint8_t>>(*other.data_);
    }
    
    void fill(uint8_t value) {
        if (data_) {
            std::fill(data_->begin(), data_->end(), value);
        }
    }
    
    // Pixel access (y, x, c)
    uint8_t at(int y, int x, int c) const {
        if (y < 0 || y >= height_ || x < 0 || x >= width_ || c < 0 || c >= channels_) {
            throw std::out_of_range("Image::at - index out of range");
        }
        return (*data_)[(y * width_ + x) * channels_ + c];
    }
    
    uint8_t& at(int y, int x, int c) {
        if (y < 0 || y >= height_ || x < 0 || x >= width_ || c < 0 || c >= channels_) {
            throw std::out_of_range("Image::at - index out of range");
        }
        return (*data_)[(y * width_ + x) * channels_ + c];
    }
};

/**
 * Tensor represents multi-dimensional array data (typically float).
 * Used for model input/output.
 */
class Tensor {
private:
    std::vector<int> shape_;
    std::shared_ptr<std::vector<double>> data_;

public:
    Tensor() {}
    
    explicit Tensor(const std::vector<int>& shape)
        : shape_(shape) {
        size_t total = 1;
        for (int dim : shape) total *= dim;
        data_ = std::make_shared<std::vector<double>>(total, 0.0);
    }
    
    // Lua-accessible methods
    std::vector<int> getShapeCpp() const { return shape_; }
    size_t length() const { return data_ ? data_->size() : 0; }
    int ndim() const { return shape_.size(); }
    
    // Data access
    double* data() { return data_->data(); }
    const double* data() const { return data_->data(); }
    
    bool empty() const { return !data_ || data_->empty(); }
    
    // Utility methods
    Tensor clone() const {
        Tensor t;
        t.shape_ = shape_;
        if (data_) {
            t.data_ = std::make_shared<std::vector<double>>(*data_);
        }
        return t;
    }
    
    void fill(double value) {
        if (data_) {
            std::fill(data_->begin(), data_->end(), value);
        }
    }
    
    void reshapeCpp(const std::vector<int>& new_shape) {
        size_t new_size = 1;
        for (int dim : new_shape) new_size *= dim;
        if (new_size != length()) {
            throw std::runtime_error("Tensor::reshape - incompatible shape");
        }
        shape_ = new_shape;
    }
    
    // Element access (flattened index)
    double at(size_t idx) const {
        if (idx >= length()) {
            throw std::out_of_range("Tensor::at - index out of range");
        }
        return (*data_)[idx];
    }
    
    double& at(size_t idx) {
        if (idx >= length()) {
            throw std::out_of_range("Tensor::at - index out of range");
        }
        return (*data_)[idx];
    }
};

/**
 * PadInfo is represented as plain Lua table (no C++ class needed).
 * Structure: {top=N, left=N, bottom=N, right=N}
 */

} // namespace CVLib
