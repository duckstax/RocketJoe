#include <catch2/catch.hpp>
#include <components/ql/aggregate.hpp>
#include <components/ql/aggregate/match.hpp>
#include <components/ql/aggregate/group.hpp>
#include <components/ql/aggregate/sort.hpp>

using namespace components;
using namespace components::ql;
using namespace components::ql::aggregate;
using components::ql::experimental::make_project_expr;
using components::ql::experimental::project_expr_type;
using core::parameter_id_t;

TEST_CASE("aggregate::match") {
    auto match = make_match(experimental::make_expr(condition_type::eq, "key", parameter_id_t(1)));
    REQUIRE(debug(match) == R"_($match: {"key": {$eq: #1}})_");
}

TEST_CASE("aggregate::group") {
    {
        group_t group;
        auto expr = make_project_expr(project_expr_type::get_field, ql::key_t("_id"));
        expr->append_param(ql::key_t("date"));
        append_expr(group, std::move(expr));
        expr = make_project_expr(project_expr_type::sum, ql::key_t("total"));
        auto expr_multiply = make_project_expr(project_expr_type::multiply);
        expr_multiply->append_param(ql::key_t("price"));
        expr_multiply->append_param(ql::key_t("quantity"));
        expr->append_param(std::move(expr_multiply));
        append_expr(group, std::move(expr));
        expr = make_project_expr(project_expr_type::avg, ql::key_t("avg_quantity"));
        expr->append_param(ql::key_t("quantity"));
        append_expr(group, std::move(expr));
        REQUIRE(debug(group) == R"_($group: {_id: "$date", total: {$sum: {$multiply: ["$price", "$quantity"]}}, avg_quantity: {$avg: "$quantity"}})_");
    }
    {
        group_t group;
        auto expr = make_project_expr(project_expr_type::get_field, ql::key_t("_id"));
        expr->append_param(ql::key_t("date"));
        append_expr(group, std::move(expr));
        expr = make_project_expr(project_expr_type::multiply, ql::key_t("count_4"));
        expr->append_param(parameter_id_t(1));
        expr->append_param(ql::key_t("count"));
        append_expr(group, std::move(expr));
        REQUIRE(debug(group) == R"_($group: {_id: "$date", count_4: {$multiply: [#1, "$count"]}})_");
    }
}

TEST_CASE("aggregate::sort") {
    sort_t sort;
    append_sort(sort, ql::key_t("name"), sort_order::asc);
    append_sort(sort, ql::key_t("count"), sort_order::desc);
    append_sort(sort, ql::key_t("_id"), sort_order::asc);
    REQUIRE(debug(sort) == R"_($sort: {name: 1, count: -1, _id: 1})_");
}

TEST_CASE("aggregate") {
    SECTION("aggregate::only_match") {
        aggregate_statement aggregate("database", "collection");
        aggregate.append(operator_type::match, make_match(experimental::make_expr(condition_type::eq, "key", parameter_id_t(1))));
        REQUIRE(debug(aggregate) == R"_($aggregate: {$match: {"key": {$eq: #1}}})_");
    }
    SECTION("aggregate::only_group") {
        aggregate_statement aggregate("database", "collection");
        group_t group;
        auto expr = make_project_expr(project_expr_type::get_field, ql::key_t("_id"));
        expr->append_param(ql::key_t("date"));
        append_expr(group, std::move(expr));
        expr = make_project_expr(project_expr_type::multiply, ql::key_t("count_4"));
        expr->append_param(parameter_id_t(1));
        expr->append_param(ql::key_t("count"));
        append_expr(group, std::move(expr));
        aggregate.append(operator_type::group, std::move(group));
        REQUIRE(debug(aggregate) == R"_($aggregate: {$group: {_id: "$date", count_4: {$multiply: [#1, "$count"]}}})_");
    }
    SECTION("aggregate::only_sort") {
        aggregate_statement aggregate("database", "collection");
        sort_t sort;
        append_sort(sort, ql::key_t("name"), sort_order::asc);
        append_sort(sort, ql::key_t("count"), sort_order::desc);
        aggregate.append(operator_type::sort, std::move(sort));
        REQUIRE(debug(aggregate) == R"_($aggregate: {$sort: {name: 1, count: -1}})_");
    }
    SECTION("aggregate::all") {
        aggregate_statement aggregate("database", "collection");

        aggregate.append(operator_type::match, make_match(experimental::make_expr(condition_type::eq, "key", parameter_id_t(1))));

        group_t group;
        auto expr = make_project_expr(project_expr_type::get_field, ql::key_t("_id"));
        expr->append_param(ql::key_t("date"));
        append_expr(group, std::move(expr));
        expr = make_project_expr(project_expr_type::multiply, ql::key_t("count_4"));
        expr->append_param(parameter_id_t(1));
        expr->append_param(ql::key_t("count"));
        append_expr(group, std::move(expr));
        aggregate.append(operator_type::group, std::move(group));

        sort_t sort;
        append_sort(sort, ql::key_t("name"), sort_order::asc);
        append_sort(sort, ql::key_t("count"), sort_order::desc);
        aggregate.append(operator_type::sort, std::move(sort));

        REQUIRE(debug(aggregate) == R"_($aggregate: {)_"
                                    R"_($match: {"key": {$eq: #1}}, )_"
                                    R"_($group: {_id: "$date", count_4: {$multiply: [#1, "$count"]}}, )_"
                                    R"_($sort: {name: 1, count: -1})_"
                                    R"_(})_");
    }
}