#pragma once

#include "components/ql/ql_statement.hpp"
#include <msgpack.hpp>
#include <msgpack/adaptor/list.hpp>
#include <msgpack/zone.hpp>

namespace components::ql {

    struct drop_database_t final : ql_statement_t {
        explicit drop_database_t(const database_name_t& database);
        drop_database_t() = default;
        drop_database_t(const drop_database_t&) = default;
        drop_database_t& operator=(const drop_database_t&) = default;
        drop_database_t(drop_database_t&&) = default;
        drop_database_t& operator=(drop_database_t&&) = default;
        ~drop_database_t() final = default;

        std::string to_string() const final {
            std::stringstream s;
            s << "drop_database: " << database_;
            return s.str();
        }
    };

} // namespace components::ql

// User defined class template specialization
namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {

            template<>
            struct convert<components::ql::drop_database_t> final {
                msgpack::object const& operator()(msgpack::object const& o, components::ql::drop_database_t& v) const {
                    if (o.type != msgpack::type::ARRAY) {
                        throw msgpack::type_error();
                    }
                    if (o.via.array.size != 1) {
                        throw msgpack::type_error();
                    }
                    v.database_ = o.via.array.ptr[0].as<database_name_t>();
                    return o;
                }
            };

            template<>
            struct pack<components::ql::drop_database_t> final {
                template<typename Stream>
                packer<Stream>& operator()(msgpack::packer<Stream>& o, components::ql::drop_database_t const& v) const {
                    o.pack_array(1);
                    o.pack(v.database_);
                    return o;
                }
            };

            template<>
            struct object_with_zone<components::ql::drop_database_t> final {
                void operator()(msgpack::object::with_zone& o, components::ql::drop_database_t const& v) const {
                    o.type = type::ARRAY;
                    o.via.array.size = 1;
                    o.via.array.ptr =
                        static_cast<msgpack::object*>(o.zone.allocate_align(sizeof(msgpack::object) * o.via.array.size,
                                                                            MSGPACK_ZONE_ALIGNOF(msgpack::object)));
                    o.via.array.ptr[0] = msgpack::object(v.database_, o.zone);
                }
            };

        } // namespace adaptor
    }     // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
