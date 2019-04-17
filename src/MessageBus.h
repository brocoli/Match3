#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <unordered_set>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace Match3 {

class MessageBus {
public:
    using Key = std::string; //TODO: use a different key type to avoid multiple string allocs, equality checks etc...
    using Data = const std::shared_ptr<const json>;
    using Callback = std::function<void(Key, Data)>;
    using CallbackPtr = std::shared_ptr<Callback>;

    MessageBus() {
        //TODO: remove this
        Attach("/", printer_);
    };

    void Attach(const Key& key, const CallbackPtr callback);
    void Detach(const Key& key, const CallbackPtr callback);
    void Notify(const Key& key, Data data);
    void Notify(const Key& key);

    inline bool GetLeakDetected() { return leakDetected_;  }

private:
    std::unordered_map<Key, std::unordered_set<CallbackPtr>> listeners_{};

    bool leakDetected_ = false;

    //TODO: remove this
    CallbackPtr printer_ = std::make_shared<Callback>([](Key key, Data data) -> void {
        std::cout << "[MessageBus] " << key << ": " << data->dump();
    });
};

} // namespace Match3
