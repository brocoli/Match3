#pragma once

namespace Match3 {
namespace Util {

template <typename T>
int signum(T value) {
    return (T(0) < value) - (value < T(0));
}

} // namespace Util
} // namespace Match3
