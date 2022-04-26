#pragma once

#include <goblin-engineer/core.hpp>
#include <excutor.hpp>
#include <log/log.hpp>
#include "disk.hpp"
#include "command.hpp"

namespace services::disk {

    using name_t = std::string;
    using session_id_t = ::components::session::session_id_t;
    using manager_t = goblin_engineer::basic_manager_service_t<goblin_engineer::base_policy_light>;

    class manager_disk_t final : public manager_t {
    public:
        manager_disk_t(path_t path_db, log_t& log, size_t num_workers, size_t max_throughput);
        void create_agent();

        auto append_database(session_id_t& session, const database_name_t &database) -> void;
        auto remove_database(session_id_t& session, const database_name_t &database) -> void;

        auto append_collection(session_id_t& session, const database_name_t &database, const collection_name_t &collection) -> void;
        auto remove_collection(session_id_t& session, const database_name_t &database, const collection_name_t &collection) -> void;

        auto write_documents(session_id_t& session, const database_name_t &database, const collection_name_t &collection, const std::vector<document_ptr> &documents) -> void;
        auto remove_documents(session_id_t& session, const database_name_t &database, const collection_name_t &collection, const std::vector<document_id_t> &documents) -> void;

        auto flush(session_id_t& session, wal::id_t wal_id) -> void;

        auto read_databases(session_id_t& session) -> void;
        auto read_collections(session_id_t& session, const database_name_t &database) -> void;
        auto read_documents(session_id_t& session, const database_name_t &database, const collection_name_t &collection) -> void;

    protected:
        auto executor_impl() noexcept -> goblin_engineer::abstract_executor* final;
        auto enqueue_base(goblin_engineer::message_ptr msg, actor_zeta::execution_device*) -> void final;
        auto add_actor_impl(goblin_engineer::actor a) -> void final;
        auto add_supervisor_impl(goblin_engineer::supervisor) -> void final;

    private:
        path_t path_db_;
        log_t log_;
        goblin_engineer::executor_ptr e_;
        std::vector<goblin_engineer::actor> actor_storage_;
        std::vector<goblin_engineer::address_t> agents_;
        command_storage_t commands_;

        auto agent() -> goblin_engineer::address_t&;
    };


    class agent_disk_t final : public goblin_engineer::abstract_service {
    public:
        agent_disk_t(goblin_engineer::supervisor_t *manager, const path_t &path_db, const name_t &name, log_t& log);
        ~agent_disk_t() final;

        auto append_database(const command_t &command) -> void;
        auto remove_database(const command_t &command) -> void;

        auto append_collection(const command_t &command) -> void;
        auto remove_collection(const command_t &command) -> void;

        auto write_documents(const command_t &command) -> void;
        auto remove_documents(const command_t &command) -> void;

        auto fix_wal_id(wal::id_t wal_id) -> void;

        auto read_databases(session_id_t& session) -> void;
        auto read_collections(session_id_t& session, const database_name_t &database) -> void;
        auto read_documents(session_id_t& session, const database_name_t &database, const collection_name_t &collection) -> void;

    private:
        log_t log_;
        disk_t disk_;
    };

} //namespace services::disk