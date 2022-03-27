#include "insert_many.hpp"

insert_many_t::~insert_many_t() = default;

insert_many_t::insert_many_t(const std::string& database, const std::string& collection, std::list<components::document::document_ptr> documents)
        : statement_t(statement_type::insert_many, database, collection)
        , documents_(std::move(documents)){}
