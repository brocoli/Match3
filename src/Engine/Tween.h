#pragma once

#include <functional>

#include "Engine.h"
#include "MessageBus.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

template<class T>
class Tween {
public:
    Tween(
        int duration,
        const T& startData,
        std::function<T(const T& data, int currentTime, int duration)> updater,
        std::function<void(const T& data)> effector,
        std::function<void(void)> onComplete,
    ) :
        updater_(updater), effector_(effector), onComplete_(onComplete),
        data_(updater(startData, 0, duration)),
        currentTime_(0), duration_(duration),
        timeScale_(_engine_->GetEngineConfiguration()["msPerFrame"])
    {
        onEngineTick_ = std::make_shared<MessageBus::Callback>(
            [this](const MessageBus::Key&, MessageBus::Data) -> void {
                currentTime_ += timeScale_;
                data_ = updater_(data_, currentTime_, duration_);
                effector_(data_);
            }
        );
        _messageBus_->Attach("/Engine/Tick", onEngineTick_);
    }

    ~Tween() {
        _messageBus_->Detach("/Engine/Tick", onEngineTick_);
    }
private:
    MessageBus::CallbackPtr onEngineTick_;

    std::function<T(const T& data, int currentTime, int duration)> updater_;
    std::function<void(const T& data)> effector_;
    std::function<void(void)> onComplete_;

    T data_;
    int currentTime_;
    int duration_;

    int timeScale_;
};

} // namespace Match3
