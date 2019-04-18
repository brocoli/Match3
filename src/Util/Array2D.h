#pragma once

#include <vector>

namespace Match3 {
namespace Util {

// Source: https://stackoverflow.com/questions/21943621/how-to-create-a-contiguous-2d-array-in-c
// (has modifications)

template<class T>
class Array2D {
    std::vector<T> data;
    size_t cols;
public:
    // This is the surrogate object for the second-level indexing
    template <class U>
    class Array2DIndexer {
        size_t offset;
        std::vector<U> &data;
    public:
        Array2DIndexer(size_t o, std::vector<U> &dt) : offset(o), data(dt) {}
        // Second-level indexing is done in this function
        T& operator[](size_t index) {
            return data[offset+index];
        }
    };
    Array2D(size_t r, size_t c) : data (r*c), cols(c) {}
    // First-level indexing is done in this function.
    Array2DIndexer<T> operator[](size_t index) {
        return Array2DIndexer<T>(index*cols, data);
    }
};

} // namespace Util
} // namespace Match3
