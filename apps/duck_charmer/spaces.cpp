#include "spaces.hpp"
#include <components/log/log.hpp>
#include <goblin-engineer/core.hpp>
#include <services/dispatcher/dispatcher.hpp>
#include <services/wal/manager_wal_replicate.hpp>

namespace duck_charmer {
    wrapper_dispatcher_t* spaces::dispatcher() {
        return wrapper_dispatcher_.get();
    }

    using services::dispatcher::manager_dispatcher_t;
    constexpr static char* name_dispatcher = "dispatcher";
    spaces::spaces() {
        std::string log_dir("/tmp/docker_logs/");
        log_ = initialization_logger("duck_charmer", log_dir);
        log_.set_level(log_t::level::trace);
        trace(log_, "spaces::spaces()");

        trace(log_, "manager_wal start");
        manager_wal_ = goblin_engineer::make_manager_service<manager_wal_replicate_t>(log_, 1, 1000);
        trace(log_, "manager_wal finish");

        trace(log_, "manager_database start");
        manager_database_ = goblin_engineer::make_manager_service<services::storage::manager_database_t>(log_, 1, 1000);
        trace(log_, "manager_database finish");

        trace(log_, "manager_dispatcher start");
        manager_dispatcher_ = goblin_engineer::make_manager_service<manager_dispatcher_t>(log_, 1, 1000);
        trace(log_, "manager_dispatcher finish");

        wrapper_dispatcher_ = std::make_unique<wrapper_dispatcher_t>(log_, name_dispatcher);
        goblin_engineer::link(manager_database_, manager_dispatcher_);
        goblin_engineer::send(manager_dispatcher_, goblin_engineer::address_t::empty_address(), "create", components::session::session_id_t(), std::string(name_dispatcher));
        trace(log_, "manager_dispatcher create dispatcher");
        auto tmp = wrapper_dispatcher_->address();
        auto tmp_1 = manager_dispatcher_->address();
        goblin_engineer::link(tmp_1, tmp);

        debug(log_, "spaces::spaces() final");
    }

    log_t& spaces::get_log() {
        return log_;
    }

} // namespace duck_charmer