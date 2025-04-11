## dough

**Dough** is a header-only C++20 testing library. It includes a structure for organizing and running tests, as well as many functions for checking values.

### Registry

The `registry` stores all defined `suite`s and their associated `test`s.
- Tests are grouped into suites.
- Both suites and tests can be assigned **tags** for filtering.
- Tests automatically inherit their suite’s tags, but can also define additional tags.
- Tags or suite names cannot include exclamation marks (`!`) or commas (`,`); these characters are replaced with underscores (`_`).

For usage example, see below.

### Functions

To check a value, use one of `check_` functions listed below.

All checks have a `require_` version.
- checks are non-blocking: on fail, they output an error message and throws an exception. In tests these exceptions are handled automatically, but you should keep them in mind if you decide to use checks outside `test` objects. You can manually disable exception throwing by passing `except_off` as a template parameter (not recommended, unless you know what you're doing).
- requires are blocking: on fail, they output an error message and call std::terminate(). You can modify this behaviour by providing your own `on_require_fail()` callback implementation.

All checks and requires have a `check_all` / `require_all` version, which runs the same check but for a list of values instead of a single value.

By default, checks and requires output messages on fail. You can disable this by passing `silent` as a template parameter.

#### Function list:

- `check_equal` - check for equality, works for floats as well, treats difference in range [-eps, eps] as equal;
- `check_true` - checks if the value is true;
- `check_false` - checks if the value is false;
- `check_null` - checks if the value is nullptr;
- `check_not_null` - checks if the value is not nullptr;
- `check_near` - checks if two values are within specified tolerance of each other.

### CLI

Command-line interface:

- `--help` / `-h` - print help
- `--all` / `-a` / `no arg` - run all tests
- `--suites` / `-s` - run specific suites
- `--tags` / `-t` - filter by tags
- `--list` / `-l` - print the list of all registered tests

```bash
# Print help
./tests --help
./tests -h

# Run all tests. Any include tags are ignored
./tests 
./tests -a
./tests --all

# Run specific suites 
#(comma-separated, spaces around commas are ignored)
./tests --suites="database,math vec"
./tests -s "database,math vec"

# Filter by tags (exclude with '!')
./tests --tags="fast,!network"
./tests -t "fast,!network"

# List all available tags
./tests --list
./tests -l

# Combine to apply filter to specific suites
./tests --suites="database" --tags="!fast"

# If a command to run tests is combined with --help or --list,
# the latter takes priority. E.g., here only the help will be 
# prined, but no tests will run
./tests --all --help

# If both --help and --list are used, the first command listed
# takes priority. Here, only --list will be executed
./tests --list --help

```
    
### Example

```cpp

#include "dough.hpp"

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
            // 'tag 1' and 'tag 2' inherited from the suite
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

### Output example

*This is NOT the output from the example above, but a separate one.*

```
[SUITE] checks started
[RUN  ] checks :: equal
[PASS ] checks :: equal
[RUN  ] checks :: true
[FAIL ] Failed check : check_true
        File         : D:\Projects\dough\test\tests.cpp
        Line         : 24
        Expected     : true
        Actual       : false
        Message      : custom fail message

[RUN  ] checks :: false
[PASS ] checks :: false
[RUN  ] checks :: null
[PASS ] checks :: null
[RUN  ] checks :: not null
[FAIL ] Failed check : check_not_null
        File         : D:\Projects\dough\test\tests.cpp
        Line         : 45
        Expected     : not null
        Actual       : nullptr
        Message      : another custom message

[RUN  ] checks :: near
[PASS ] checks :: near

[=== SUITE: checks ===]
    Run      : 6
    Pass     : 4
    Fail     : 2
    Failures :
     - true
     - not null

[SUITE] io started
[RUN  ] io :: output
[PASS ] io :: output
[RUN  ] io :: input
[ERROR] Test 'input' threw an exception: unknown exception

[=== SUITE: io ===]
    Run      : 2
    Pass     : 1
    Fail     : 1
    Failures :
     - input


 ---------------------------
[===== OVERALL SUMMARY =====]
 ---------------------------
    Total    : 8
    Passed   : 5
    Failed   : 3
    Failures :
     - checks :: true
     - checks :: not null
     - io :: input
```












