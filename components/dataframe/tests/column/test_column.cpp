#include <catch2/catch.hpp>

#include <memory>
#include <memory_resource>
#include <random>
#include <algorithm>

#include "dataframe/column/column.hpp"
#include "dataframe/column/column_view.hpp"
#include "dataframe/column/make.hpp"
#include "dataframe/bitmask.hpp"
#include "dataframe/type_dispatcher.hpp"
#include "dataframe/types.hpp"

#include "dataframe/tests/tools.hpp"

using namespace components::dataframe;
using namespace components::dataframe::column;

/*
bool equal(std::pmr::memory_resource* resource,column_view const& lhs,column_view const& rhs) {
    auto lhs_indices = generate_all_row_indices(resource,lhs.size());
    auto rhs_indices = generate_all_row_indices(resource,rhs.size());
    return type_dispatcher(
        lhs.type(),
        column_comparator<true>{},
        lhs,
        rhs,
        *lhs_indices,
        *rhs_indices);
}
*/
template<typename T>
struct gen_column {
    data_type type() { return data_type{type_to_id<T>()}; }

    gen_column()
        : resource_(std::pmr::get_default_resource())
        , data(resource_, _num_elements * size_of(type()))
        , mask(resource_, bitmask_allocation_size_bytes(_num_elements))
        , all_valid_mask(resource_, create_null_mask(resource_,num_elements(), mask_state::all_valid))
        , all_null_mask(resource_, create_null_mask(resource_,num_elements(), mask_state::all_null)) {
        test::sequence(data);
        test::sequence(mask);
    }

    size_type num_elements() { return _num_elements; }

    std::pmr::memory_resource* resource_;
    std::random_device r;
    std::default_random_engine generator{r()};
    std::uniform_int_distribution<size_type> distribution{200, 1000};
    size_type _num_elements{distribution(generator)};
    core::buffer data;
    core::buffer mask;
    core::buffer all_valid_mask;
    core::buffer all_null_mask;
};

void verify_column_views(column_t col) {
    column_view view = col;
    mutable_column_view mutable_view = col;
    REQUIRE(col.type() == view.type());
    REQUIRE(col.type() == mutable_view.type());
    REQUIRE(col.size() == view.size());
    REQUIRE(col.size() == mutable_view.size());
    REQUIRE(col.null_count() == view.null_count());
    REQUIRE(col.null_count() == mutable_view.null_count());
    REQUIRE(col.nullable() == view.nullable());
    REQUIRE(col.nullable() == mutable_view.nullable());
    REQUIRE(col.num_children() == view.num_children());
    REQUIRE(col.num_children() == mutable_view.num_children());
    REQUIRE(view.head() == mutable_view.head());
    REQUIRE(view.data<char>() == mutable_view.data<char>());
    REQUIRE(view.offset() == mutable_view.offset());
}

TEMPLATE_TEST_CASE("default null count no mask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data),core::buffer(resource));
    REQUIRE_FALSE(col.nullable());
    REQUIRE_FALSE(col.has_nulls());
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("default null count empty mask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), core::buffer(resource));
    REQUIRE_FALSE(col.nullable());
    REQUIRE_FALSE(col.has_nulls());
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("default null count all valid", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    REQUIRE(col.nullable());
    REQUIRE_FALSE(col.has_nulls());
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("explicit null count all valid", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask), 0);
    REQUIRE(col.nullable());
    REQUIRE_FALSE(col.has_nulls());
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("default null count all null", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_null_mask));
    REQUIRE(col.nullable());
    REQUIRE(col.has_nulls());
    REQUIRE(gen.num_elements() == col.null_count());
}

TEMPLATE_TEST_CASE("ExplicitNullCountAllNull", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_null_mask), gen.num_elements());
    REQUIRE(col.nullable());
    REQUIRE(col.has_nulls());
    REQUIRE(gen.num_elements() == col.null_count());
}

TEMPLATE_TEST_CASE("SetNullCountNoMask", "[column][template]", std::int32_t) {
    gen_column<TestType> gen;
    column_t col{gen.type(), gen.num_elements(), std::move(gen.data)};
    REQUIRE_THROWS_AS(col.set_null_count(1), std::logic_error);
}

TEMPLATE_TEST_CASE("SetEmptyNullMaskNonZeroNullCount", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col{gen.type(), gen.num_elements(), std::move(gen.data)};
    core::buffer empty_null_mask(resource);
    REQUIRE_THROWS_AS(col.set_null_mask(empty_null_mask, gen.num_elements()), std::logic_error);
}

TEMPLATE_TEST_CASE("SetInvalidSizeNullMaskNonZeroNullCount", "[column][template]", std::int32_t) {
    gen_column<TestType> gen;
    column_t col{gen.type(), gen.num_elements(), std::move(gen.data)};
    auto invalid_size_null_mask = create_null_mask(std::min(gen.num_elements() - 50, 0), mask_state::all_valid);
    REQUIRE_THROWS_AS(col.set_null_mask(invalid_size_null_mask, gen.num_elements()), std::logic_error);
}

TEMPLATE_TEST_CASE("SetNullCountEmptyMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col{gen.type(), gen.num_elements(), std::move(gen.data), core::buffer{}};
    REQUIRE_THROWS_AS(col.set_null_count(1), std::logic_error);
}

TEMPLATE_TEST_CASE("SetNullCountAllValid", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    REQUIRE_NOTHROW(col.set_null_count(0));
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("SetNullCountAllNull", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_null_mask));
    REQUIRE_NOTHROW(col.set_null_count(gen.num_elements()));
    REQUIRE(gen.num_elements() == col.null_count());
}

TEMPLATE_TEST_CASE("ResetNullCountAllNull", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_null_mask));

    REQUIRE(gen.num_elements() == col.null_count());
    REQUIRE_NOTHROW(col.set_null_count(unknown_null_count));
    REQUIRE(gen.num_elements() == col.null_count());
}

TEMPLATE_TEST_CASE("ResetNullCountAllValid", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    REQUIRE(0 == col.null_count());
    REQUIRE_NOTHROW(col.set_null_count(unknown_null_count));
    REQUIRE(0 == col.null_count());
}

TEMPLATE_TEST_CASE("CopyDataNoMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data),core::buffer(resource));
    REQUIRE(gen.type() == col.type());
    REQUIRE_FALSE(col.nullable());
    REQUIRE(0 == col.null_count());
    REQUIRE(gen.num_elements() == col.size());
    REQUIRE(0 == col.num_children());

    verify_column_views(col);

    column_view v = col;
    REQUIRE(v.head() != gen.data.data());
    REQUIRE(equal(v.head(), gen.data.data(), gen.data.size()));
}

TEMPLATE_TEST_CASE("MoveDataNoMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    void* original_data = gen.data.data();
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data));
    REQUIRE(gen.type() == col.type());
    REQUIRE_FALSE(col.nullable());
    REQUIRE(0 == col.null_count());
    REQUIRE(gen.num_elements() == col.size());
    REQUIRE(0 == col.num_children());

    verify_column_views(col);

    // Verify shallow copy
    column_view v = col;
    REQUIRE(v.head() == original_data);
}

TEMPLATE_TEST_CASE("CopyDataAndMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;

    column_t col(resource,gen.type(),
                 gen.num_elements(),
                 core::buffer{resource, gen.data},
                 core::buffer{resource, gen.all_valid_mask});

    REQUIRE(gen.type() == col.type());
    REQUIRE(col.nullable());
    REQUIRE(0 == col.null_count());
    REQUIRE(gen.num_elements() == col.size());
    REQUIRE(0 == col.num_children());

    verify_column_views(col);

    // Verify deep copy
    column_view v = col;
    REQUIRE(v.head() != gen.data.data());
    REQUIRE(v.null_mask() != gen.all_valid_mask.data());
    REQUIRE(equal(v.head(), gen.data.data(), gen.data.size()));
    REQUIRE(equal(v.null_mask(), gen.all_valid_mask.data(), gen.mask.size()));
}

TEMPLATE_TEST_CASE("MoveDataAndMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    void* original_data = gen.data.data();
    void* original_mask = gen.all_valid_mask.data();
    column_t col(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    REQUIRE(gen.type() == col.type());
    REQUIRE(col.nullable());
    REQUIRE(0 == col.null_count());
    REQUIRE(gen.num_elements() == col.size());
    REQUIRE(0 == col.num_children());

    verify_column_views(col);

    // Verify shallow copy
    column_view v = col;
    REQUIRE(v.head() == original_data);
    REQUIRE(v.null_mask() == original_mask);
}

TEMPLATE_TEST_CASE("CopyConstructorNoMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t original(resource,gen.type(), gen.num_elements(), std::move(gen.data));
    column_t copy{resource,original};
    verify_column_views(copy);
    REQUIRE(equal(original, copy));

    // Verify deep copy
    column_view original_view = original;
    column_view copy_view = copy;
    REQUIRE(original_view.head() != copy_view.head());
}

TEMPLATE_TEST_CASE("CopyConstructorWithMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t original(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    column_t copy{resource,original};
    verify_column_views(copy);
    REQUIRE(equal(original, copy));

    // Verify deep copy
    column_view original_view = original;
    column_view copy_view = copy;
    REQUIRE(original_view.head() != copy_view.head());
    REQUIRE(original_view.null_mask() != copy_view.null_mask());
}

TEMPLATE_TEST_CASE("MoveConstructorNoMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t original(resource,gen.type(), gen.num_elements(), std::move(gen.data));

    auto original_data = original.view().head();

    column_t moved_to{std::move(original)};

    REQUIRE(0 == original.size());
    REQUIRE(data_type{type_id::empty} == original.type());

    verify_column_views(moved_to);

    // Verify move
    column_view moved_to_view = moved_to;
    REQUIRE(original_data == moved_to_view.head());
}

TEMPLATE_TEST_CASE("MoveConstructorWithMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t original(resource,gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask));
    auto original_data = original.view().head();
    auto original_mask = original.view().null_mask();
    column_t moved_to{std::move(original)};
    verify_column_views(moved_to);

    REQUIRE(0 == original.size());
    REQUIRE(data_type{type_id::empty} == original.type());

    // Verify move
    column_view moved_to_view = moved_to;
    REQUIRE(original_data == moved_to_view.head());
    REQUIRE(original_mask == moved_to_view.null_mask());
}

TEMPLATE_TEST_CASE("DeviceUvectorConstructorNoMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;

    core::uvector<TestType> original(resource, static_cast<std::size_t>(gen.num_elements()));
    copy(static_cast<TestType*>(gen.data.data()),static_cast<TestType*>(gen.data.data()) + gen.num_elements(),original.begin());

    auto original_data = original.data();
    column_t moved_to{std::move(original)};
    verify_column_views(moved_to);

    // Verify move
    column_view moved_to_view = moved_to;
    REQUIRE(original_data == moved_to_view.head());
}

TEMPLATE_TEST_CASE("DeviceUvectorConstructorWithMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;

    core::uvector<TestType> original(resource, static_cast<std::size_t>(gen.num_elements()));
    copy(static_cast<TestType*>(gen.data.data()),static_cast<TestType*>(gen.data.data()) + gen.num_elements(),original.begin());

    auto original_data = original.data();
    auto original_mask = gen.all_valid_mask.data();
    column_t moved_to{std::move(original), std::move(gen.all_valid_mask)};
    verify_column_views(moved_to);

    // Verify move
    column_view moved_to_view = moved_to;
    REQUIRE(original_data == moved_to_view.head());
    REQUIRE(original_mask == moved_to_view.null_mask());
}

TEMPLATE_TEST_CASE("ConstructWithChildren", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    std::vector<std::unique_ptr<column_t>> children;

    children.emplace_back(std::make_unique<column_t>(data_type{type_id::int8}, 42, core::buffer(resource, gen.data), core::buffer(resource,gen.all_valid_mask)));
    children.emplace_back(std::make_unique<column_t>(data_type{type_id::float64}, 314, core::buffer(resource, gen.data), core::buffer{resource, gen.all_valid_mask}));
    column_t col{gen.type(), gen.num_elements(), core::buffer(resource, gen.data), core::buffer{resource, gen.all_valid_mask}, unknown_null_count, std::move(children)};

    verify_column_views(col);
    REQUIRE(2 == col.num_children());
    REQUIRE(data_type{type_id::int8} == col.child(0).type());
    REQUIRE(42 == col.child(0).size());
    REQUIRE(data_type{type_id::float64} == col.child(1).type());
    REQUIRE(314 == col.child(1).size());
}

TEMPLATE_TEST_CASE("ReleaseNoChildren", "[column][template]", std::int32_t) {
    gen_column<TestType> gen;
    column_t col{gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask)};
    auto original_data = col.view().head();
    auto original_mask = col.view().null_mask();

    column_t::contents contents = col.release();
    REQUIRE(original_data == contents.data->data());
    REQUIRE(original_mask == contents.null_mask->data());
    REQUIRE(0u == contents.children.size());
    REQUIRE(0 == col.size());
    REQUIRE(0 == col.null_count());
    REQUIRE(data_type{type_id::empty} == col.type());
    REQUIRE(0 == col.num_children());
}

TEMPLATE_TEST_CASE("ReleaseWithChildren", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    std::vector<std::unique_ptr<column_t>> children;

    children.emplace_back(std::make_unique<column_t>(gen.type(), gen.num_elements(), core::buffer(resource, gen.data), core::buffer(resource, gen.all_valid_mask)));
    children.emplace_back(std::make_unique<column_t>(gen.type(), gen.num_elements(), core::buffer(resource, gen.data), core::buffer(resource, gen.all_valid_mask)));

    column_t col{
        gen.type(),
        gen.num_elements(),
        core::buffer(resource, gen.data),
        core::buffer(resource, gen.all_valid_mask),
        unknown_null_count,
        std::move(children)};

    auto original_data = col.view().head();
    auto original_mask = col.view().null_mask();

    column_t::contents contents = col.release();
    REQUIRE(original_data == contents.data->data());
    REQUIRE(original_mask == contents.null_mask->data());
    REQUIRE(2u == contents.children.size());
    REQUIRE(0 == col.size());
    REQUIRE(0 == col.null_count());
    REQUIRE(data_type{type_id::empty} == col.type());
    REQUIRE(0 == col.num_children());
}

TEMPLATE_TEST_CASE("ColumnViewConstructorWithMask", "[column][template]", std::int32_t) {
    std::pmr::memory_resource* resource = std::pmr::get_default_resource();
    gen_column<TestType> gen;
    column_t original{gen.type(), gen.num_elements(), std::move(gen.data), std::move(gen.all_valid_mask)};
    column_view original_view = original;
    column_t copy{original_view};
    verify_column_views(copy);
    REQUIRE(equal(original, copy));

    // Verify deep copy
    column_view copy_view = copy;
    REQUIRE(original_view.head() != copy_view.head());
    REQUIRE(original_view.null_mask() != copy_view.null_mask());
}