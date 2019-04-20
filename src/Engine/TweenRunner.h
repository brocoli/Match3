#pragma once

#include <functional>
#include <set>
#include <forward_list>

#include "Engine.h"
#include "MessageBus.h"
#include "Tween.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

template<class T>
class TweenRunner {
public:
    TweenRunner() : timeScale_(_engine_->GetEngineConfiguration()["msPerFrame"]), livingTweenCount_(0) {
        onEngineTick_ = std::make_shared<MessageBus::Callback>(
            [this](const MessageBus::Key&, MessageBus::Data) -> void {
                std::forward_list<std::shared_ptr<Tween<T>>> tweensToRemove;

                for (auto tt = tweens_.begin(); tt != tweens_.end(); ++tt) {
                    auto tween = *tt;
                    if (tween->Update()) {
                        tweensToRemove.push_front(tween);
                        --livingTweenCount_;
                    }
                }

                for (auto tt = tweensToRemove.begin(); tt != tweensToRemove.end(); ++tt) {
                    auto tween = *tt;
                    tweens_.erase(tween);
                }
            }
        );
        _messageBus_->Attach("/Engine/Tick", onEngineTick_);
    }

    ~TweenRunner() {
        _messageBus_->Detach("/Engine/Tick", onEngineTick_);
    }

    void StartTween(
        int duration,
        const T& startData,
        std::function<T(const T& data, int currentTime, int duration)> updater,
        std::function<void(const T& data)> effector,
        std::function<void(void)> onComplete
    ) {
        ++livingTweenCount_;
        std::shared_ptr<Tween<T>> tween = std::make_shared<Tween<T>>(
            duration, startData, timeScale_,
            updater, effector,
            onComplete
        );
        tweens_.emplace(tween);
    }

    int GetLivingTweenCount() { return livingTweenCount_; }

private:
    MessageBus::CallbackPtr onEngineTick_;

    std::set<std::shared_ptr<Tween<T>>> tweens_;

    int timeScale_;
    int livingTweenCount_;
};

} // namespace Match3
