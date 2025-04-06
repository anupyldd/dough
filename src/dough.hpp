#pragma once

#include <string>
#include <sstream>
#include <source_location>
#include <iostream>
#include <format>
#include <functional>
#include <unordered_set>
#include <exception>
#include <initializer_list>

/**
* @brief main dough namespace
*/
namespace dough
{
    /**
    * @brief implementation details
    */
    namespace detail
    {
        /**
        * @brief recursively appends values to stream
        */
        template<class T, class... Rest>
        void append_values(std::stringstream& sstr, T value, Rest... rest)
        {
            sstr << value;

            if constexpr (sizeof...(rest) > 0)
            {
                sstr << ' ';
                append_values(sstr, rest...);
            }
        }

        /**
        * @brief boolean type for message output configuration
        */
        template<class T>
        concept log_mode = std::is_same_v<T, std::true_type> || std::is_same_v<T, std::false_type>;

        /**
        * @brief boolean type for exception throw configuration
        */
        template<class T>
        concept except_mode = std::is_same_v<T, std::true_type> || std::is_same_v<T, std::false_type>;

        /**
        * @brief formats fail message
        */
        template<class... T>
        std::string fail_format(const std::string& message, const std::string& check_type,
            const std::source_location& location, T... values)
        {
            std::stringstream valsstr;
            valsstr.precision(10);
            detail::append_values(valsstr, "values:", values...);

            std::stringstream result;
            result << "[fail] [" <<
                check_type << "] [file: " <<
                std::string(location.file_name()) << ", line: " <<
                location.line() << "] [" << valsstr.str() << "] message: " <<
                message << "\n";

            return result.str();
        }

        /**
        * @brief prints formatted check fail message
        * @param values values used in the test
        */
        template<class... T>
        void fail_print(const std::string& message, const std::string& check_type,
            const std::source_location& location, T... values)
        {
            std::cerr << fail_format(message, check_type, location, values...);
        }

        /**
        * @brief epsilon constant
        */
        constexpr float EPSILON = 0.000030517578125f;

        /**
        * @brief get value sign with epsilon
        */
        template<std::floating_point T>
        int sign_epsilon(T value, float epsilon = EPSILON)
        {
            return (value > epsilon) ? 1 : (value < -epsilon) ? -1 : 0;
        }

        /**
        * @struct test_fail
        * @brief struct to throw when test fails
        */
        struct test_fail {};
    }

    /**
    * @brief pass this as a template parameter to checks or requirements to enable message logging (default)
    */
    using loud = std::true_type;
    /**
    * @brief pass this as a template parameter to checks or requirements to disable message logging
    */
    using silent = std::false_type;

    /**
    * @brief pass this as a template parameter to checks to enable exception throwing (default)
    */
    using except_on = std::true_type;
    /**
    * @brief pass this as a template parameter to checks to disable exception throwing
    */
    using except_off = std::false_type;

    /**
        * @brief use this to check if two values are equal. can compare floats, treats difference in range [-eps, eps] as equal
        * @param first first value
        * @param second second value
        * @param message message that is printed when check fails
        * @param location location of check fail in source, don't change this unless you have a good reason to
        */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_equal(T first, T second, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        bool equal = false;

        // floats
        if constexpr (std::is_floating_point_v<T>)
        {
            equal = (detail::sign_epsilon(first - second) == 0);
        }
        // other
        else
        {
            equal = (first == second);
        }

        if (!equal)
        {
            if constexpr (M::value) detail::fail_print(message, "equal", location, first, second);
            if constexpr (E::value) throw detail::test_fail{};
        }

        return equal;
    }

    /**
    * @brief use this to check if all values in list are equal to value are equal.can compare floats, treats difference in range[-eps, eps] as equal
    * @param list list of values to compare
    * @param second value to which compare
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_equal(const std::initializer_list<T>&list, T value,
        const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_equal<M, E>(val, value, message, location)) return false;
        }
        return true;
    }

    /**
    * @brief use this to check if value is true
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_true(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!value)
        {
            if constexpr (M::value) detail::fail_print(message, "true", location, value);
            if constexpr (E::value) throw detail::test_fail{};
        }

        return value;
    }

    /**
    * @brief use this to check if all values in the list are true
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_true(std::initializer_list<T> list, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_true<M, E>(val, message, location)) return false;
        }
        return true;
    }

    /**
    * @brief use this to check if value is false
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_false(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value)
        {
            if constexpr (M::value) detail::fail_print(message, "false", location, value);
            if constexpr (E::value) throw detail::test_fail{};
        }

        return !value;
    }

    /**
    * @brief use this to check if all values in the list are false
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_false(std::initializer_list<T> list, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_false<M, E>(val, message, location)) return false;
        }
        return true;
    }

    /**
    * @brief use this to check if value is null
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_null(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value != nullptr)
        {
            if constexpr (M::value) detail::fail_print(message, "null", location, value);
            if constexpr (E::value) throw detail::test_fail{};
        }

        return static_cast<bool>(!value);
    }

    /**
    * @brief use this to check if all values in the list are null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_null(std::initializer_list<T> list, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_null<M, E>(val, message, location)) return false;
        }
        return true;
    }

    /**
    * @brief use this to check if value is not null
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_not_null(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value == nullptr)
        {
            if constexpr (M::value) detail::fail_print(message, "not_null", location, value);
            if constexpr (E::value) throw detail::test_fail{};
        }

        return static_cast<bool>(value);
    }

    /**
    * @brief use this to check if all values in the list are not null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_not_null(std::initializer_list<T> list, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_not_null<M, E>(val, message, location)) return false;
        }
        return true;
    }

    /**
    * @brief use this to compare values that can have rounding errors
    * @param first first value
    * @param second second value
    * @param tolerance allowed difference
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_near(T first, T second, T tolerance, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (std::abs(first - second) <= tolerance) return true;

        if constexpr (M::value) detail::fail_print(message, "near", location, first, second, tolerance);
        if constexpr (E::value) throw detail::test_fail{};

        return false;
    }

    /**
    * @brief use this to compare values that can have rounding errors
    * @param list list values from which are tested
    * @param value value to which list values are compared
    * @param tolerance allowed difference
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, detail::except_mode E = except_on, class T>
    inline bool check_all_near(std::initializer_list<T> list, T value, T tolerance,
        const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            if (!check_near<M, E>(val, value, tolerance, message, location)) return false;
        }
        return true;
    }
       
    /************************************************************************************/

    /**
    * @brief callback that is called when a requirement fails. calls std::terminate() by default
    */
    inline std::function<void()> on_require_fail = [&]() { std::terminate(); };

    /************************************************************************************/

    /**
    * @brief requires two values to be equal. terminate on fail
    * @param first first value
    * @param second second value
    * @param message message that is printed when requirement fails
    * @param location location of requirement fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_equal(T first, T second, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_equal<M, except_off>(first, second, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to require all values in list to be equal to value are equal. can compare floats, treats difference in range [-eps, eps] as equal
    * @param list list of values to compare
    * @param second value to which compare
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_equal(const std::initializer_list<T>& list, T value,
        const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_equal<M>(val, value, message, location);
        }
    }

    /**
    * @brief requires value to be true. terminate on fail
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of requirement fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_true(T value, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_true<M, except_off>(value, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to require all values in the list to be true
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_true(std::initializer_list<T> list, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_true<M>(val, message, location);
        }
    }

    /**
    * @brief requires value to be false. terminate on fail
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of requirement fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_false(T value, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_false<M, except_off>(value, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to require all values in the list to be false
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_false(std::initializer_list<T> list, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_false<M>(val, message, location);
        }
    }

    /**
    * @brief requires value to be null. terminate on fail
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of requirement fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_null(T value, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_null<M, except_off>(value, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to require all values in the list to be null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_null(std::initializer_list<T> list, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_null<M>(val, message, location);
        }
    }

    /**
    * @brief requires value to be not null. terminate on fail
    * @param value value to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_not_null(T value, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_not_null<M, except_off>(value, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to require all values in the list to be not null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_not_null(std::initializer_list<T> list, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_not_null<M>(val, message, location);
        }
    }

    /**
    * @brief use this to compare values that can have rounding errors. terminate on fail
    * @param first first value
    * @param second second value
    * @param tolerance allowed difference
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    bool require_near(T first, T second, T tolerance, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!check_near<M, except_off>(first, second, tolerance, message, location))
        {
            if (on_require_fail) on_require_fail();
            else std::terminate();
        }
    }

    /**
    * @brief use this to compare values that can have rounding errors
    * @param list list values from which are tested
    * @param value value to which list values are compared
    * @param tolerance allowed difference
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = loud, class T>
    inline void require_all_near(std::initializer_list<T> list, T value, T tolerance,
        const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        for (const auto& val : list)
        {
            require_near<M>(val, value, tolerance, message, location);
        }
    }

    /************************************************************************************/

    namespace detail
    {
        /**
        * @brief find unordered set intersection
        */
        std::unordered_set<std::string> uset_intersection(
            const std::unordered_set<std::string>& first,
            const std::unordered_set<std::string>& second)
        {
            std::unordered_set<std::string> intersection;
            for (const auto& elem : first)
            {
                if (second.contains(elem)) intersection.insert(elem);
            }
            return intersection;
        }

        /**
        * @brief checks if sets have at least one common element
        */
        bool uset_have_common(
            const std::unordered_set<std::string>& first,
            const std::unordered_set<std::string>& second
        )
        {
            const auto& [smallest, largest] = (first.size() < second.size() ?
                std::tie(first, second) : std::tie(second, first));

            for (const auto& elem : smallest)
            {
                if (largest.contains(elem)) return true;
            }
            return false;
        }

        /**
        * @brief insert arbitrary number of values into uset
        */
        template<class T, class... Args>
        void uset_insert(std::unordered_set<T>& set, Args&&... args)
        {
            (set.insert(std::forward<Args>(args)), ...);
        }
    }

    /**
    * @struct include_tags
    * @brief structure that holds tags to include
    */
    struct include_tags
    {
        std::unordered_set<std::string> set;
    };

    /**
    * @struct exclude_tags
    * @brief structure that holds tags to include
    */
    struct exclude_tags
    {
        std::unordered_set<std::string> set;
    };

    /**
    * @brief helper function for including tags in filter
    */
    template<typename First, typename... Rest>
        requires
    (std::convertible_to<First, std::string> &&
        (std::convertible_to<Rest, std::string> && ...))
        include_tags inc(First&& first, Rest&&... rest)
    {
        include_tags tags;
        detail::uset_insert(tags.set, std::forward<First>(first), std::forward<Rest>(rest)...);
        return tags;
    }

    /**
    * @brief helper function for excluding tags in filter
    */
    template<typename First, typename... Rest>
        requires
    (std::convertible_to<First, std::string> &&
        (std::convertible_to<Rest, std::string> && ...))
        exclude_tags exc(First&& first, Rest&&... rest)
    {
        exclude_tags tags;
        detail::uset_insert(tags.set, std::forward<First>(first), std::forward<Rest>(rest)...);
        return tags;
    }

    /**
    * @class test
    * @brief represents a single test
    */
    class test
    {
    public:
        test(std::string name) noexcept : test_name(std::move(name)) {}
        test(const test& src) = default;

        /**
        * @brief set test function
        */
        test& func(std::function<void()> test_func) noexcept
        {
            if (test_func) function = std::move(test_func);
            return *this;
        }

        /**
        * @brief add test tags
        */
        template<typename First, typename... Rest>
            requires 
        (std::convertible_to<First, std::string> &&
        (std::convertible_to<Rest, std::string> && ...))
            test& tags(First&& first, Rest&&... rest) noexcept
        {
            detail::uset_insert(tag_set, std::forward<First>(first), std::forward<Rest>(rest)...);
            return *this;
        }

        /**
        * @brief get tag set
        */
        const std::unordered_set<std::string>& tags() const noexcept
        {
            return tag_set;
        }

        /**
        * @brief set name
        */
        test& name(std::string new_name) noexcept
        {
            test_name = std::move(new_name);
            return *this;
        }

        /**
        * @brief get name
        */
        const std::string& name() const noexcept
        {
            return test_name;
        }

        /**
        * @brief run the test
        */
        void run()
        {
            if (function)
            {
                try
                {
                    function();
                    result_print(true);
                }
                catch (detail::test_fail)
                {
                    result_print(false);
                }
                catch (const std::exception& e)
                {
                    error_print(e.what());
                }
                catch (...)
                {
                    error_print();
                }
            }
        }

    private:
        /**
        * @brief print test result
        */
        void result_print(bool success)
        {
            std::stringstream sstr;
            sstr << (success ? "[SUCCESS] " : "[FAIL   ] ") << test_name << '\n';
            std::cout << sstr.str();
        }

        /**
        * @brief print error if a non-test_fail exception if thrown
        */
        void error_print(const std::string& msg = std::string())
        {
            std::stringstream sstr;
            sstr << "[ERROR  ] test '" << test_name << "' threw an exception: " <<
                (msg.empty() ? "unknown exception" : msg) << '\n';
            std::cerr << sstr.str();
        }

    private:
        std::unordered_set<std::string> tag_set;
        std::function<void()> function = nullptr;
        std::string test_name;
    };

    /**
    * @class suite
    * @brief test suite class
    */
    class suite
    {
    public:
        suite(std::string name) noexcept : suite_name(std::move(name)) {}

        /**
        * @brief add setup function that will run before each test
        */
        suite& setup(std::function<void()> setup_func)
        {
            if (setup_func) setup_function = std::move(setup_func);
            return *this;
        }

        /**
        * @brief add teardown function that will run after each test
        */
        suite& teardown(std::function<void()> teardown_func)
        {
            if (teardown_func) teardown_function = std::move(teardown_func);
            return *this;
        }

        /**
        * @brief add suite tags. these are inherited by all test in a suite
        */
        template<typename First, typename... Rest>
            requires 
        (std::convertible_to<First, std::string> &&
        (std::convertible_to<Rest, std::string> && ...))
            suite& tags(First&& first, Rest&&... rest) noexcept
        {
            detail::uset_insert(tag_set, std::forward<First>(first), std::forward<Rest>(rest)...);
            for (auto& test : test_list) test.tags(first, rest...);
            return *this;
        }

        /**
        * @brief get tag set
        */
        const std::unordered_set<std::string>& tags() const noexcept
        {
            return tag_set;
        }

        /**
        * @brief register test
        */
        suite& add(test new_test) noexcept
        {
            auto& ntst = test_list.emplace_back(std::move(new_test));

            // inherit suite tags
            for (const auto& tag : tag_set)
            {
                ntst.tags(tag);
            }

            return *this;
        }

        /**
        * @brief set name
        */
        suite& name(std::string new_name) noexcept
        {
            suite_name = std::move(new_name);
            return *this;
        }

        /**
        * @brief get name
        */
        const std::string& name() const noexcept
        {
            return suite_name;
        }

        /**
        * @brief run all tests in a suite
        */
        void run()
        {
            for (auto& test : test_list)
            {
                if (setup_function) setup_function();
                test.run();
                if (teardown_function) teardown_function();
            }
        }

        /**
        * @brief run specific test by name
        */
        void run(std::string_view name)
        {
            for (auto& test : test_list)
            {
                if (test.name() == name)
                {
                    if (setup_function) setup_function();
                    test.run();
                    if (teardown_function) teardown_function();
                    return;
                }
            }
        }

        /**
        * @brief run test with tag filtering. test runs if at leas one of required tags is present. test is excluded by the same logic
        */
        void run(
            const include_tags& inc_tags,
            const exclude_tags& exc_tags = {})
        {
            for (auto& test : test_list)
            {
                // skip test with excluded tag
                if (detail::uset_have_common(test.tags(), exc_tags.set)) continue;

                // run if has at least one required tag or if no include tags are specified
                if (inc_tags.set.empty() || detail::uset_have_common(test.tags(), inc_tags.set))
                {
                    if (setup_function) setup_function();
                    test.run();
                    if (teardown_function) teardown_function();
                }
            }
        }

    private:
        std::unordered_set<std::string> tag_set;
        std::function<void()> setup_function = nullptr;
        std::function<void()> teardown_function = nullptr;
        std::string suite_name;
        std::vector<test> test_list;
    };

    /**
    * @class registry
    * @brief class that stores and runs tests, either individually or all at once
    */
    class registry
    {
    public:
        /**
        * @brief register a test suite
        */
        dough::suite& suite(std::string name) noexcept
        {
            auto& ns = suite_list.emplace_back(name);
            return ns;
        }

        /**
        * @brief run all suites
        */
        void run()
        {
            for (auto& st : suite_list)
            {
                st.run();
            }
        }

        /**
        * @brief run specific suite
        */
        void run(std::string_view suite_name)
        {
            for (auto& st : suite_list)
            {
                if (st.name() == suite_name)
                {
                    st.run();
                    return;
                }
            }
        }

        /**
        * @brief run specific test of specific suite
        */
        void run(std::string_view suite_name, std::string_view test_name)
        {
            for (auto& st : suite_list)
            {
                if (st.name() == suite_name)
                {
                    st.run(test_name);
                    return;
                }
            }
        }

        /**
        * @brief run tests with filtering by tag
        */
        void run(
            const include_tags& inc_tags,
            const exclude_tags& exc_tags = {})
        {
            for (auto& st : suite_list)
            {
                // skip suites with excluded tags
                if (detail::uset_have_common(st.tags(), exc_tags.set)) continue;

                // run suite if no included tags are specified or at least one suite tag satisfies the requirement
                if (inc_tags.set.empty() || detail::uset_have_common(st.tags(), inc_tags.set))
                    st.run(inc_tags, exc_tags);
            }
        }

    private:
        std::vector<dough::suite> suite_list;
    };
}