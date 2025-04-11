#pragma once

#include <algorithm>
#include <array>
#include <exception>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <source_location>
#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

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
        template<class T1, class T2>
        std::string fail_format(
            const std::string& message,
            const std::string& check_type,
            const std::source_location& location,
            T1 expected,
            T2 actual)
        {
            std::stringstream result;
            result << std::boolalpha;
            result.precision(10);

            result <<
                "[FAIL ] Failed check : " << check_type << '\n' <<
                "        File         : " << std::string(location.file_name()) << '\n' <<
                "        Line         : " << location.line() << '\n' <<
                "        Expected     : " << expected << '\n' <<
                "        Actual       : " << actual << '\n' <<
                "        Message      : " << message << "\n\n";

            return result.str();
        }

        /**
        * @struct test_fail
        * @brief struct to throw when test fails
        */
        struct test_fail
        {
            test_fail() = default;

            template<class T1, class T2>
            test_fail(
                const std::string& message,
                const std::string& check_type,
                const std::source_location& location,
                T1 expected,
                T2 actual)
            {
                msg = fail_format(message, check_type, location, expected, actual);
            }

            std::string msg = "not initialized";
        };

        /**
        * @brief prints formatted check fail message
        * @param values values used in the test
        */
        template<class T>
        void fail_print(const test_fail& fail)
        {
            std::cerr << "" << fail.msg;
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_equal(T actual, T expected, const std::string& message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        bool equal = false;

        // floats
        if constexpr (std::is_floating_point_v<T>)
        {
            equal = (detail::sign_epsilon(actual - expected) == 0);
        }
        // other
        else
        {
            equal = (actual == expected);
        }

        if (!equal)
        {
            detail::test_fail fail(message, "check_equal", location, expected, actual);

            if constexpr (M::value) detail::fail_print(fail);
            if constexpr (E::value) throw fail;
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_true(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (!value)
        {
            detail::test_fail fail(message, "check_true", location, true, value);

            if constexpr (M::value) detail::fail_print(fail);
            if constexpr (E::value) throw fail;
        }

        return value;
    }

    /**
    * @brief use this to check if all values in the list are true
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_false(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value)
        {
            detail::test_fail fail(message, "check_false", location, false, value);

            if constexpr (M::value) detail::fail_print(fail);
            if constexpr (E::value) throw fail;
        }

        return !value;
    }

    /**
    * @brief use this to check if all values in the list are false
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_null(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value != nullptr)
        {
            detail::test_fail fail(message, "check_null", location, nullptr, value);

            if constexpr (M::value) detail::fail_print(fail);
            if constexpr (E::value) throw fail;
        }

        return static_cast<bool>(!value);
    }

    /**
    * @brief use this to check if all values in the list are null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_not_null(T value, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        if (value == nullptr)
        {
            detail::test_fail fail(message, "check_not_null", location, "not null", value);

            if constexpr (M::value) detail::fail_print(fail);
            if constexpr (E::value) throw fail;
        }

        return static_cast<bool>(value);
    }

    /**
    * @brief use this to check if all values in the list are not null
    * @param list list to test
    * @param message message that is printed when check fails
    * @param location location of check fail in source, don't change this unless you have a good reason to
    */
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
    inline bool check_near(T first, T second, T tolerance, const std::string & message = std::string(),
        const std::source_location& location = std::source_location::current())
    {
        T diff = std::abs(first - second);
        if (diff <= tolerance) return true;

        detail::test_fail fail(message, "check_near", location, tolerance, diff);

        if constexpr (M::value) detail::fail_print(fail);
        if constexpr (E::value) throw fail;

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
    template<detail::log_mode M = silent, detail::except_mode E = except_on, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
    template<detail::log_mode M = silent, class T>
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
        * @brief array of reserved characters
        */
        const std::array<char, 2> reserved_chars{ '!', ',' };
        /**
        * @brief char with which reserved chars are replaced
        */
        const char replace_char = '_';

        /**
        * @brief replaces reserved characters from string
        */
        template <typename S>
        std::string sanitize_tag(S&& tag) 
        {
            std::string str = std::forward<S>(tag);
            for (char c : reserved_chars) 
            {
                std::replace(str.begin(), str.end(), c, replace_char);
            }
            return str;
        }

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
        void uset_insert(std::unordered_set<T>& set, Args... args) 
        {
            (set.insert(args), ...);
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
        include_tags inc(const First& first, const Rest&... rest)
    {
        include_tags tags;
        detail::uset_insert(tags.set, first, rest...);
        return tags;
    }
    include_tags inc() { return include_tags(); }

    /**
    * @brief helper function for excluding tags in filter
    */
    template<typename First, typename... Rest>
        requires
    (std::convertible_to<First, std::string> &&
        (std::convertible_to<Rest, std::string> && ...))
        exclude_tags exc(const First& first, const Rest&... rest)
    {
        exclude_tags tags;
        detail::uset_insert(tags.set, first, rest...);
        return tags;
    }
    exclude_tags exc() { return exclude_tags(); }

    class suite;
    class test;

    /**
    * @class test
    * @brief represents a single test
    */
    class test
    {
        friend class suite;

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
            detail::uset_insert(tag_set, detail::sanitize_tag(first), detail::sanitize_tag(rest)...);
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
        bool run()
        {
            if (function)
            {
                try
                {
                    start_print();
                    function();
                    result_print(true);
                    return true;
                }
                catch (const detail::test_fail& f)
                {
                    result_print(false, f);
                    return false;
                }
                catch (const std::exception& e)
                {
                    error_print(e.what());
                    return false;
                }
                catch (...)
                {
                    error_print();
                    return false;
                }
            }
            return false;
        }

    private:
        /**
        * @brief set owner suite
        */
        void owner(const std::string& name) noexcept
        {
            owner_name = name;
        }

        /**
        * @brief prints test run start message
        */
        void start_print() const noexcept
        {
            std::stringstream sstr;
            sstr << "[RUN  ] " << owner_name << " :: " << test_name << '\n';
            std::cout << sstr.str();
        }

        /**
        * @brief print test result
        */
        void result_print(bool success, detail::test_fail fail = {}) const noexcept
        {
            std::stringstream sstr;
            if (success)
                sstr << "[PASS ] " << owner_name << " :: " << test_name << '\n';
            else
                sstr << fail.msg;

            std::cout << sstr.str();
        }

        /**
        * @brief print error if a non-test_fail exception if thrown
        */
        void error_print(const std::string& msg = std::string()) const noexcept
        {
            std::stringstream sstr;
            sstr << "[ERROR] Test '" << test_name << "' threw an exception: " <<
                (msg.empty() ? "unknown exception" : msg) << '\n';
            std::cerr << sstr.str();
        }

    private:
        std::unordered_set<std::string> tag_set;
        std::function<void()> function = nullptr;
        std::string test_name;
        std::string owner_name;
    };

    /**
    * @class suite
    * @brief test suite class
    */
    class suite
    {
        friend test;

    public:
        /**
        * @struct stats
        * @brief holds stats for a suite run
        */
        struct stats
        {
            std::vector<std::string> failed;
            int run = 0,
                pass = 0,
                fail = 0;
        };

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
            detail::uset_insert(tag_set, detail::sanitize_tag(first), detail::sanitize_tag(rest)...);
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
            ntst.owner(suite_name);

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
        * @brief get list of tests
        */
        const std::vector<test>& tests() const noexcept
        {
            return test_list;
        }

        /**
        * @brief run all tests in a suite
        */
        stats run()
        {
            start_print();
            stats st;
            for (auto& test : test_list)
            {
                if (setup_function) setup_function();
                
                if (test.run()) st.pass++;
                else
                {
                    st.fail++;
                    st.failed.push_back(test.name());
                }
                
                if (teardown_function) teardown_function();

                st.run++;
            }
            summary_print(st);
            return st;
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
        stats run(
            const include_tags& inc_tags,
            const exclude_tags& exc_tags = {})
        {
            stats st;
            start_print();
            for (auto& test : test_list)
            {
                // skip test with excluded tag
                if (detail::uset_have_common(test.tags(), exc_tags.set)) continue;

                // run if has at least one required tag or if no include tags are specified
                if (inc_tags.set.empty() || detail::uset_have_common(test.tags(), inc_tags.set))
                {
                    if (setup_function) setup_function();
                    if (test.run()) st.pass++;
                    else
                    {
                        st.fail++;
                        st.failed.push_back(test.name());
                    }
                    if (teardown_function) teardown_function();

                    st.run++;
                }
            }
            summary_print(st);
            return st;
        }

    private:
        /**
        * @brief print suite start message
        */
        void start_print()
        {
            std::stringstream sstr;
            sstr << "[SUITE] " << suite_name << " started" << '\n';
            std::cout << sstr.str();
        }

        /**
        * @brief print suite start message
        */
        void finish_print()
        {
            std::stringstream sstr;
            sstr << "[SUITE  ] " << suite_name << " finished" << '\n';
            std::cout << sstr.str();
        }

        /**
        * @brief prints suite run summary
        */
        void summary_print(const stats& st)
        {
            if (st.run == 0) return;

            std::stringstream sstr;
            sstr << "\n[=== SUITE: " << suite_name << " ===]\n" <<
                "    Run      : " << st.run << '\n' <<
                "    Pass     : " << st.pass << '\n' <<
                "    Fail     : " << st.fail << '\n';
            if (st.fail > 0)
            {
                sstr << "    Failures : \n";
                for (int i = 0; i < st.failed.size(); ++i)
                    sstr << "     - " << st.failed[i] << (i == st.failed.size() - 1 ? "" : "\n");
            }
            sstr << "\n\n";
            std::cout << sstr.str();
        }

    private:
        std::unordered_set<std::string> tag_set;
        std::function<void()> setup_function = nullptr;
        std::function<void()> teardown_function = nullptr;
        std::string suite_name;
        std::vector<test> test_list;
    };

    namespace detail
    {
        /**
        * @brief message that is printed on help command
        */
        const char* help_message =
            "Print help\n"
            "    ./tests --help\n"
            "    ./tests -h\n"
            "\n"
            "Run all tests. Any include tags are ignored\n"
            "    ./tests\n"
            "    ./tests -a\n"
            "    ./tests --all\n"
            "\n"
            "Run specific suites \n"
            "(comma-separated, spaces around commas are ignored)\n"
            "    ./tests --suites=\"database, math vec\"\n"
            "    ./tests -s \"database, math vec\"\n"
            "\n"
            "Filter by tags (exclude with '!')\n"
            "    ./tests --tags=\"fast, !network\"\n"
            "    ./tests -t \"fast, !network\"\n"
            "\n"
            "List all available tags\n"
            "    ./tests --list\n"
            "    ./tests -l\n"
            "\n"
            "Combine to apply filter to specific suites\n"
            "    ./tests --suites=\"database\" --tags=\"!fast\"\n"
            "\n"
            "If a command to run tests is combined with --help or --list,\n"
            "the latter takes priority. E.g., here only the help will be \n"
            "prined, but no tests will run\n"
            "    ./tests --all --help\n"
            "\n"
            "If both --help and --list are used, the first command listed\n"
            "takes priority. Here, only --list will be executed\n"
            "    ./tests --list --help\n";

        /**
        * @brief trim string from the left
        */
        inline void trim_l(std::string& s) 
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
                [](unsigned char ch) { return !std::isspace(ch); }));
        }

        /**
        * @brief trim string from the right
        */
        inline void trim_r(std::string& s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), 
                [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        }

        /**
        * @brief trim string from both sides
        */
        inline void trim(std::string& s) 
        {
            trim_l(s);
            trim_r(s);
        }

        /**
        * @struct cli_command
        * @brief parsed cli command
        */
        struct cli_command
        {
            std::unordered_set<std::string> inc_tags;
            std::unordered_set<std::string> exc_tags;
            std::string error_msg;
            std::vector<std::string> suites;
            bool list = false;
            bool help = false;
            bool run_all = false;
        };

        /**
        * @brief format cli error
        */
        std::string cli_error_format(const std::string& str)
        {
            return std::format("[CLI  ] Error: {}", str);
        }

        /**
        * @brief parse suites
        */
        void cli_parse_suites(cli_command& cmd, const std::string& value)
        {
            std::vector<std::string> suites;
            std::stringstream sstr(value);
            std::string part;
            while (std::getline(sstr, part, ','))
            {
                trim(part);
                cmd.suites.push_back(part);
            }
        }

        /**
        * @brief parse tags
        */
        void cli_parse_tags(cli_command& cmd, const std::string& value)
        {
            std::vector<std::string> suites;
            std::stringstream sstr(value);
            std::string part;
            while (std::getline(sstr, part, ','))
            {
                trim(part);
                if (part.starts_with('!')) cmd.exc_tags.insert(part.substr(1));
                else cmd.inc_tags.insert(part);
            }
        }

        /**
        * @brief parses cl args into a command
        */
        cli_command cli_parse(int argc, char** argv)
        {
            cli_command command;
            std::vector<std::string> arguments(argv + 1, argv + argc);
            size_t arg_size = arguments.size();

            auto get_value = [&](const std::string& arg) -> std::optional<std::string>
                {
                    auto ind = arg.find('=');
                    if (ind == arg.npos)
                    {
                        command.error_msg = cli_error_format(
                            std::format("missing '=' when passing values in '{}'", arg));
                        return std::nullopt;
                    }
                    return arg.substr(ind + 1);
                };

            if (arg_size == 0)
            {
                command.run_all = true;
                return command; // run all if no args passed
            }

            for (size_t i = 0; i < arg_size; ++i)
            {
                if (arguments[i] == "-h" || arguments[i] == "--help")
                {
                    command.help = true;
                    return command; // help overrides all other args
                }

                else if (arguments[i] == "-l" || arguments[i] == "--list")
                {
                    command.list = true;
                    return command; // list also overrides if is first
                }

                else if (arguments[i] == "-s")
                {
                    if (i == arg_size - 1)
                    {
                        command.error_msg = cli_error_format("missing argument after '-s'");
                        return command; // no reason to parse after the error
                    }
                    // pass the next arg since they are delim'ed by space: -s "suite"
                    cli_parse_suites(command, arguments[++i]);
                }
                else if (arguments[i].starts_with("--suites"))
                {
                    // get value from the same arg
                    auto value = get_value(arguments[i]);
                    if (value) cli_parse_suites(command, value.value());
                    else return command; // return with error from get_value
                }

                else if (arguments[i] == "-t")
                {
                    if (i == arg_size - 1)
                    {
                        command.error_msg = cli_error_format("missing argument after '-t'");
                        return command; // no reason to parse after the error
                    }
                    cli_parse_tags(command, arguments[++i]);
                }
                else if (arguments[i].starts_with("--tags"))
                {
                    // get value from the same arg
                    auto value = get_value(arguments[i]);
                    if (value) cli_parse_tags(command, value.value());
                    else return command; // return with error from get_value
                }

                else if (arguments[i] == "-a" || arguments[i] == "--all")
                {
                    command.run_all = true;
                    // no return to get exclude tags
                }

                else
                {
                    command.error_msg = cli_error_format(
                        std::format("unknown argument {}", arguments[i]));
                    return command;
                }
            }

            return command;
        }
    }

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
            auto& ns = suite_list.emplace_back(detail::sanitize_tag(name));
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
            summary sum;
            for (auto& st : suite_list)
            {
                if (st.name() == suite_name)
                {
                    sum.stats[st.name()] = st.run();
                    return;
                }
            }
            summary_print(sum);
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
        * @brief run tests with filtering by tag, exclude optional
        */
        void run(
            const include_tags& inc_tags,
            const exclude_tags& exc_tags = {})
        {
            summary sum;
            for (auto& st : suite_list)
            {
                // skip suites with excluded tags
                if (detail::uset_have_common(st.tags(), exc_tags.set)) continue;

                sum.stats[st.name()] = st.run(inc_tags, exc_tags);
            }
            summary_print(sum);
        }

        /**
        * @brief run tests with filtering by tag, include optional
        */
        void run(
            const exclude_tags& exc_tags,
            const include_tags& inc_tags = {})
        {
            run(inc_tags, exc_tags);
        }

        /**
        * @brief run tests of a specific suite with filtering by tag
        */
        void run(
            const std::string& suite_name,
            const include_tags& inc_tags = {},
            const exclude_tags& exc_tags = {})
        {
            summary sum;
            for (auto& st : suite_list)
            {
                if (st.name() != suite_name) continue;

                // skip suites with excluded tags
                if (detail::uset_have_common(st.tags(), exc_tags.set)) continue;

                sum.stats[st.name()] = st.run(inc_tags, exc_tags);
            }
            summary_print(sum);
        }

        /**
        * @brief run based on command line arguments
        */
        void run(int argc, char** argv)
        {
            auto cmd = detail::cli_parse(argc, argv);
            

            if (!cmd.error_msg.empty())
            {
                std::cerr << cmd.error_msg << '\n';
                return;
            }

            if (cmd.help)
            {
                std::cout << detail::help_message << '\n';
                return;
            }

            if (cmd.list)
            {
                list_print();
                return;
            }

            if (cmd.run_all)
            {
                run(exclude_tags{ cmd.exc_tags });
                return;
            }

            if (!cmd.suites.empty())
            {
                for (const auto& s : cmd.suites)
                {
                    run(s,
                        include_tags{ cmd.inc_tags },
                        exclude_tags{ cmd.exc_tags });
                }
            }
            else
            {
                run(include_tags{ cmd.inc_tags },
                    exclude_tags{ cmd.exc_tags });
            }
        }

    private:
        /**
        * @struct summary
        * @brief whole run summary
        */
        struct summary
        {
            std::unordered_map<std::string, suite::stats> stats;
        };

        /**
        * @brief output whole summary
        */
        void summary_print(const summary& sum)
        {
            if (sum.stats.size() == 0) return;

            int run = 0,
                pass = 0,
                fail = 0;
            std::stringstream failed;

            for (const auto& [name, stat] : sum.stats)
            {
                if (stat.run == 0) continue;

                run += stat.run;
                pass += stat.pass;
                fail += stat.fail;

                for (int i = 0; i < stat.failed.size(); ++i)
                {
                    failed << "     - " << name << " :: " << stat.failed[i] << '\n';
                }
            }

            std::stringstream sstr;
            sstr << "\n ---------------------------";
            sstr << "\n[===== OVERALL SUMMARY =====]\n" <<
                " ---------------------------\n" <<
                "    Total    : " << run << '\n' <<
                "    Passed   : " << pass << '\n' <<
                "    Failed   : " << fail << '\n';

            if (fail > 0)
            {
                sstr << "    Failures :\n" << failed.str();
            }
            else
            {
                if (run > 0) sstr << "[DOUGH] All tests passed";
            }

            sstr << '\n';
            std::cout << sstr.str();
        }

        /**
        * @brief prints a list of all registeret tests
        */
        void list_print()
        {
            for (const auto& st : suite_list)
            {
                // - suite name [ tag1, tag2 ]
                std::cout << "\n- " << st.name();
                if (st.tags().size() > 0)
                {
                    std::cout << " [ ";
                    for (auto it = st.tags().begin(); it != st.tags().end(); ++it)
                    {
                        std::cout << (*it) << 
                            (it != (--st.tags().end()) ? ", " : "");
                    }
                    std::cout << " ]";
                }
                std::cout << '\n';

                const auto tab = "    ";
                if (st.tests().empty())
                {
                    std::cout << tab << "*no registered tests*";
                }
                else
                {
                    for (const auto& tst : st.tests())
                    {
                        std::cout << tab << "- " << tst.name();
                        if (tst.tags().size() > 0)
                        {
                            std::cout << " [ ";
                            for (auto it = tst.tags().begin(); it != tst.tags().end(); ++it)
                            {
                                std::cout << (*it) <<
                                    (it != (--tst.tags().end()) ? ", " : "");
                            }
                            std::cout << " ]";
                        }
                        std::cout << '\n';
                    }
                }

            }
            std::cout << '\n';
        }

    private:
        std::vector<dough::suite> suite_list;
    };
}