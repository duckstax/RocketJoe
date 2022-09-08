#pragma once

#include <services/collection/operators/operator.hpp>
#include <services/collection/operators/predicates/predicate.hpp>

namespace services::collection::operators {

    class insert final : public operator_t {
    public:
        insert(context_collection_t* collection, std::list<document_ptr>&& documents);
        insert(context_collection_t* collection, const std::list<document_ptr>& documents);

    private:
        void on_execute_impl(components::cursor::sub_cursor_t* cursor) final;

        std::list<document_ptr> documents_;
    };

} // namespace services::operators