#pragma once

#include <vector>

namespace Match3 {
namespace Util {

// Source: https://stackoverflow.com/questions/21943621/how-to-create-a-contiguous-2d-array-in-c
// (with modifications)

template<class T>
class Array2D {
    std::vector<T> data;
    size_t rows;
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

    Array2D(size_t c, size_t r, T initialValue) : data(r*c, initialValue), rows(r), cols(c) {}

    // First-level indexing is done in this function.
    Array2DIndexer<T> operator[](size_t index) {
        return Array2DIndexer<T>(index*cols, data);
    }

    size_t GetRows() const { return rows; }
    size_t GetCols() const { return cols; }

    std::vector<size_t> FindPatternInstances(Array2D<T>& pattern) {
        std::vector<size_t> patternInstances;

        for (size_t j = 0; j < GetCols() - pattern.GetCols() + 1; ++j) {
            for (size_t i = 0; i < GetRows() - pattern.GetRows() + 1; ++i) {
                bool patternDetected = true;

                for (size_t pj = 0; pj < pattern.GetCols(); ++pj) {
                    if (!patternDetected) {
                        break;
                    }
                    for (size_t pi = 0; pi < pattern.GetRows(); ++pi) {
                        if (
                            (*this)[i + pi][j + pj]
                            !=
                            pattern[pi][pj]
                        ) {
                            patternDetected = false;
                            break;
                        }
                    }
                }

                if (patternDetected) {
                    patternInstances.emplace_back(j);
                    patternInstances.emplace_back(i);
                }
            }
        }

        return std::move(patternInstances);
    }
};

} // namespace Util
} // namespace Match3
