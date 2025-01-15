
#include <iostream>
#include <cstdarg>

template <typename... Args>
struct Promote
{
    using type = std::common_type_t<Args...>;
};

template <typename T>
T min(int count, ...)
{
    va_list args;
    va_start(args, count);

    using promoted_type = std::conditional_t<std::is_same_v<T, int>, int, double>;

    promoted_type val = static_cast<promoted_type>(va_arg(args, double));
    // T val = va_arg(args, T);
    for (int i = 1; i < count; i++)
    {

        promoted_type  n  = static_cast<promoted_type>(va_arg(args, T));
        if (n < val)
            val = n;
    }

    va_end(args);

    return val;
}

int main()
{
    // Test with integers
    std::cout << "Testing with integers:" << std::endl;
    int min_int = min<int>(4, 10, 5, 8, 3);
    std::cout << "min(10,5,8,3) = " << min_int << std::endl;
    if (min_int != 3)
        std::cout << "Test failed!" << std::endl;

    // Test with doubles
    std::cout << "\nTesting with doubles:" << std::endl;
    double min_double = min<double>(3, 3.14, 2.718, 1.414);
    std::cout << "min(3.14,2.718,1.414) = " << min_double << std::endl;
    if (min_double != 1.414)
        std::cout << "Test failed!" << std::endl;

    // Test with single value
    std::cout << "\nTesting with single value:" << std::endl;
    int single = min<int>(1, 42);
    std::cout << "min(42) = " << single << std::endl;
    if (single != 42)
        std::cout << "Test failed!" << std::endl;

    // Test with negative numbers
    std::cout << "\nTesting with negative numbers:" << std::endl;
    int min_neg = min<int>(5.0, -1.0, -5, 0, -3, -2);
    std::cout << "min(-1.0,-5,0,-3,-2) = " << min_neg << std::endl;
    if (min_neg != -5)
        std::cout << "Test failed!" << std::endl;

    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}