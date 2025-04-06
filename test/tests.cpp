#include "../src/dough.hpp"

int main()
{
    using namespace dough;

    registry reg;
    
    reg .suite("suite 1")
        .tags("tag 1","tag 2")
        .setup([]() {std::cout << "first suite setup\n"; })
        .teardown([]() {std::cout << "first suite teardown\n"; })
        .add(
            test("test 1")
            .tags("tag 3")
            .func([]() { check_equal(1, 2, "test 1 fail"); })
        )
        .add(
            test("test 2")
            .func([]() { check_true(false, "test 2 fail"); })
        );

    reg.suite("suite 2")
        .tags("tag 1", "tag 4")
        .setup([]() {std::cout << "second suite setup\n"; })
        .teardown([]() {std::cout << "second suite teardown\n"; })
        .add(
            test("test 3")
            .tags("tag 3")
            .func([]() { check_false(true, "test 3 fail"); })
        )
        .add(
            test("test 4")
            .func([]() { check_not_null(nullptr, "test 4 fail"); })
        );

    reg.run();
    std::cout << "--------------------\n";
    reg.run("suite 1");
    std::cout << "--------------------\n";
    reg.run("suite 1", "test 1");
    
}