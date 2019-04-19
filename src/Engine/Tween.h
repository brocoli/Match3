#pragma once

#include <functional>


namespace Match3 {

template<class T>
class Tween {
public:
    Tween(
        int duration,
        const T& startData,
        int timeScale,
        std::function<T(const T& data, int currentTime, int duration)> updater,
        std::function<void(const T& data)> effector,
        std::function<void(void)> onComplete
    ) :
        updater_(updater), effector_(effector),
        onComplete_(onComplete),
        data_(updater(startData, 0, duration)),
        currentTime_(0), duration_(duration),
        timeScale_(timeScale)
    {}

    bool Update() {
        currentTime_ = std::min(currentTime_ + timeScale_, duration_);
        data_ = updater_(data_, currentTime_, duration_);
        effector_(data_);

        if (currentTime_ == duration_) {
            onComplete_();
            return true;
        }

        return false;
    }

private:

    std::function<T(const T& data, int currentTime, int duration)> updater_;
    std::function<void(const T& data)> effector_;
    std::function<void(void)> onComplete_;

    T data_;
    int currentTime_;
    int duration_;

    int timeScale_;
};

class Easing {
public:
    static std::function<int(const int& data, int currentTime, int duration)> InQuad(int start, int delta) {
        return [start, delta](const int& data, int currentTime, int duration) -> int {
            return start + (delta * currentTime * currentTime) / (duration * duration);
        };
    }
};

} // namespace Match3
