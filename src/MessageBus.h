#ifndef MATCH3_MESSAGE_BUS_H
#define MATCH3_MESSAGE_BUS_H

#include <functional>
#include <iostream>
#include <map>
#include <unordered_set>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace Match3 {

class MessageBus {
public:
    //TODO: auto-generate keys from strings, for ease of use.
    //      (i.e. using a std::vector or a trie).
    enum Key {
        _,
        _Engine_Started,
    };
    static const char* KeyPretty[];

    using Data = const std::shared_ptr<const json>;
    using Callback = std::function<void(Key, Data)>;

    MessageBus() {
        //TODO: remove this
        Attach(Key::_, printer_);
    };

    void Attach(Key key, const std::shared_ptr<Callback> callback);
    void Detach(Key key, const std::shared_ptr<Callback> callback);
    void Notify(Key key, Data data);
    void Notify(Key key);

private:
    std::unordered_map<Key, std::unordered_set<std::shared_ptr<Callback>>> listeners_{};

    //TODO: remove this
    std::shared_ptr<Callback> printer_ = std::make_shared<Callback>([](Key key, Data data) -> void {
        std::cout << "[MessageBus] " << KeyPretty[key] << ": " << data->dump();
    });
};

} // namespace Match3

#endif // !MATCH3_MESSAGE_BUS_H
