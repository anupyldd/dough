## dough

**Dough** is a minimalist header-only C++20 testing library. It includes a structure for organizing an running tests, as well as many functions for checking values.

### Registry

The main class of **dough** is a `registry`. It is a class for storing `test`s grouped into `suite`s. Each suite and test can have any number of tags, which can be used for filtering, which tests are executed. Tests inherit the tags of a suite they belong to, but can also have their own additional tags.

To see how to use the registry, see the example below.

### Functions

*All checks and requires have check_all / require_all version, which runs the same check but for a list of values instead of single value.*

*By default checks and requires output messages on fail. You can disable this by passing `silent` as a template parameter.*

- **check** (non-blocking: on fail they output an error message but still allow further tests to run)
    - `check_equal` - check for equality. works for floats as well, treats difference in range [-eps, eps] as equal;
    - `check_true` - checks if the value is true;
    - `check_false` - checks if the value is false;
    - `check_null` - checks if the value is nullptr;
    - `check_not_null` - checks if the value is not nullptr;
    - `check_near` - checks if two values are within specified tolerance of each other.
    
- **require** (blocking: on fail they output an error message and terminate)
    - `require_equal` - requires equality. works for floats as well, treats difference in range [-eps, eps] as equal;
    - `require_true` - requires the value to be true;
    - `require_false` - requires the value to be false;
    - `require_null` - requires the value to be nullptr;
    - `require_not_null` - requires the value to be nullptr;
    - `require_near` - requires two values to be within specified tolerance of each other.
    
### Example

```cpp

int main()
{
    using namespace dough;

    // create registry
    registry reg;   
    
    // register the first suite with name 'suite 1'
    reg .suite("suite 1") 
        // specify tags for suite 1 
        // (optional)
        .tags("tag 1","tag 2")
        // set the 'setup' function, it'll run before each test in this suite
        // (optional)
        .setup([]() {std::cout << "first suite setup\n"; })
        // set the 'teardown' function, it'll run before each test in this suite
        // (optional)
        .teardown([]() {std::cout << "first suite teardown\n"; })
        // add a test
        .add(
            // create test object with name 'test 1'
            test("test 1")
            // add another tag to this test on top of 
            // 'tag 1' and 'tag 2' inheritef from the suite
            // (optional)
            .tags("tag 3")
            // set the test function
            .func([]() { check_equal(1, 2, "test 1 fail"); })
        )
        // add another test the same way
        .add(
            test("test 2")
            .func([]() { check_true(false, "test 2 fail"); })
        );

    // register another suite the same way as the first
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

    // run all tests in the registry
    reg.run();
    std::cout << "--------------------\n";
    // run all tests in 'suite 1'
    reg.run("suite 1");
    std::cout << "--------------------\n";
    // run 'test 1' from 'suite 1'
    reg.run("suite 1", "test 1");
    std::cout << "--------------------\n";
    // run tests filtered by tags
    // use inc() and exc() for convenience
    // if a test has ANY tag from exc(), it'll be skipped
    // if a test has NO tags from exc() and has ANY tag from inc() it'll be run
    // if inc() is empty, all tests will be run, unless they are skipped
    // here, only 'test 2' from 'suite 1' will run
    reg.run(inc("tag 1"), exc("tag 3", "tag 4"));
}

```












