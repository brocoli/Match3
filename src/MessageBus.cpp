#include "MessageBus.h"


namespace Match3 {

void MessageBus::Attach(const Key& key, const CallbackPtr callback) {
    if (listeners_.find(key) == listeners_.end()) {
        listeners_[key] = std::unordered_set<std::shared_ptr<Callback>>();
    }
    listeners_[key].insert(callback);
}

void MessageBus::Detach(const Key& key, const CallbackPtr callback) {
    listeners_[key].erase(callback);
    //TODO: Don't treat the root event as a special case
    if ((key != "/") && listeners_[key].empty()) {
        listeners_.erase(key);
    }
}

void MessageBus::Notify(const Key& key, Data data) {
    //TODO: Don't treat the root event as a special case
    //      implement message signal tiers instead
    if (key != "/") {
        for (auto listener : listeners_["/"]) {
            if (listener.use_count() < 3) { // original owner + listeners_ + this
                std::cout << "Leak detected in MessageBus. Did you forget to Detach from \"/\"?" << std::endl;
                leakDetected_ = true;
            } else {
                (*listener)(key, data);
            }
        }
    }

    if (listeners_.find(key) != listeners_.end()) {
        for (auto listener : listeners_[key]) {
            if (listener.use_count() < 3) { // original owner + listeners_ + this
                std::cout << "Leak detected in MessageBus. Did you forget to Detach from \"" << key << "\"?" << std::endl;
                leakDetected_ = true;
            } else {
                (*listener)(key, data);
            }
        }
    }
}

static const std::shared_ptr<const json> dummy = std::make_shared<const json>(json{});

void MessageBus::Notify(const Key& key) {
    Notify(key, dummy);
}

} // namespace Match3
