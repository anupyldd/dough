#include "../src/dough.hpp"

int main()
{
    using namespace dough;

    const std::string no_see{ "should NOT see this" };

    registry reg;
    
    reg.suite("checks")
        .add(
            test("equal")
            .func([&]() {
                check_equal(1.0f, 1.0f, no_see);
                check_all_equal({ 1,1,1 }, 1, no_see);
                })
        )
        .add(
            test("true")
            .func([&]() {
                check_true(true, no_see);
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
            test("not_null")
            .func([&]() {
                check_not_null(&no_see, no_see);
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
    
    reg.run();
}