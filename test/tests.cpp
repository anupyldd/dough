#include "../src/dough.hpp"

int main()
{
    using namespace dough;

    const std::string see{ "should see this" };
    const std::string no_see{ "should NOT see this" };

    registry reg;
    
    reg.suite("checks")
        .tags("func")
        .add(
            test("equal")
            .func([&]() {
                check_equal(1.1f, 1.1f, no_see);
                check_all_equal({ 1,1,1 }, 1, no_see);
                })
        )
        .add(
            test("true")
            .func([&]() {
                check_true(false, no_see);
                check_all_true({ true,true }, no_see);
                })
        )
        .add(
            test("false")
            .func([&]() {
                check_false(false, no_see);
                check_all_false({ false,false }, no_see);
                })
        )
        .add(
            test("null")
            .func([&]() {
                check_null(nullptr, no_see);
                check_all_null({ nullptr,nullptr }, no_see);
                })
        )
        .add(
            test("not null")
            .func([&]() {
                check_not_null(nullptr, no_see);
                check_all_not_null({ &no_see,&no_see }, no_see);
                })
        )
        .add(
            test("near")
            .func([&]() {
                check_near(1.001f, 1.0015f, 0.001f, no_see);
                check_all_near({ 1.001f,1.0012f }, 1.0015f, 0.001f, no_see);
                })
        );

    on_require_fail = []() { };

    reg.suite("requires")
        .tags("func")
        .add(
            test("req equal")
            .func([&]() {
                require_equal(1.0f, 1.0f, no_see);
                require_all_equal({ 1,1,1 }, 1, no_see);
                })
        )
        .add(
            test("req true")
            .func([&]() {
                require_true(true, no_see);
                require_all_true({ true,true }, no_see);
                })
        )
        .add(
            test("req false")
            .func([&]() {
                require_false(false, no_see);
                require_all_false({ false,false }, no_see);
                })
        )
        .add(
            test("req null")
            .func([&]() {
                require_null(nullptr, no_see);
                require_all_null({ nullptr,nullptr }, no_see);
                })
        )
        .add(
            test("req not null")
            .func([&]() {
                require_not_null(&no_see, no_see);
                require_all_not_null({ &no_see,&no_see }, no_see);
                })
        )
        .add(
            test("req near")
            .func([&]() {
                require_near(1.001f, 1.0015f, 0.001f, no_see);
                require_all_near({ 1.001f,1.0012f }, 1.0015f, 0.001f, no_see);
                })
        );

    reg.suite("filter (no tags)")
        .add(
            test("no tags 1")
            .func([&]() { })
        )
        .add(
            test("no tags 2")
            .func([&]() { })
        );

    reg.suite("filter (with tags)")
        .tags("suite tag", "suite tag 2")
        .add(
            test("inherit tags")
            .func([&]() { })
        )
        .add(
            test("own tags")
            .tags("test tag", "test tag 2")
            .func([&]() { })
        );
    
    reg.run(inc("func"));

    std::cout << "\n\n--- should see 2 tests ---\n";
    reg.run(inc("suite tag"));
    
    std::cout << "--- should see 1 test ---\n";
    reg.run(inc("test tag"));

    std::cout << "--- should see 3 test ---\n";
    reg.run(inc(), exc("test tag 2", "func"));
}