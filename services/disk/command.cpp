#include "command.hpp"
#include "route.hpp"
#include <actor-zeta.hpp>

namespace services::disk {

    command_t::command_name_t command_t::name() const {
        return std::visit(
            [](const auto& c) {
                using command_type = std::decay_t<decltype(c)>;
                if constexpr (std::is_same_v<command_type, command_write_documents_t>) {
                    return static_cast<uint64_t>(route::write_documents);
                } else if constexpr (std::is_same_v<command_type, command_remove_documents_t>) {
                    return static_cast<uint64_t>(route::remove_documents);
                }
                static_assert(true, "Not valid command type");
            },
            command_);
    }

    void append_command(command_storage_t &storage, const components::session::session_id_t &session, const command_t &command) {
        auto it = storage.find(session);
        if (it != storage.end()) {
            it->second.push_back(command);
        } else {
            std::vector<command_t> commands = {command};
            storage.emplace(session, commands);
        }
    }

} //namespace services::disk