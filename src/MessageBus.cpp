#include "MessageBus.h"


namespace Match3 {

//TODO: dynamically generate keys from strings, for ease of use.
//      (i.e. using a std::vector or a trie).
const char* MessageBus::KeyPretty[] = {
    "/",
    "/Engine/Started",
};

void MessageBus::Attach(Key key, const std::shared_ptr<Callback> callback) {
    if (listeners_.find(key) == listeners_.end()) {
        listeners_[key] = std::unordered_set<std::shared_ptr<Callback>>();
    }
    listeners_[key].insert(callback);
}

void MessageBus::Detach(Key key, const std::shared_ptr<Callback> callback) {
    listeners_[key].erase(callback);
    if (listeners_[key].empty()) {
        listeners_.erase(key);
    }
}

void MessageBus::Notify(Key key, Data data) {
    //TODO: Don't treat the root event as a special case
    //      this is a placeholder for a true tiered-message observer system
    for (auto listener : listeners_[Key::_]) {
        if (listener.use_count() < 3) { // original owner + listeners_ + this
            std::cout << "Leak detected in MessageBus. Did you forget to Detach from \"" << KeyPretty[Key::_] << "\"?" << std::endl;
        } else {
            (*listener)(key, data);
        }
    }

    if (listeners_.find(key) != listeners_.end()) {
        for (auto listener : listeners_[key]) {
            if (listener.use_count() < 3) { // original owner + listeners_ + this
                std::cout << "Leak detected in MessageBus. Did you forget to Detach from \"" << KeyPretty[key] << "\"?" << std::endl;
            } else {
                (*listener)(key, data);
            }
        }
    }
}

static const std::shared_ptr<const json> dummy = std::make_shared<const json>(json{});

void MessageBus::Notify(Key key) {
    Notify(key, dummy);
}

} // namespace Match3
