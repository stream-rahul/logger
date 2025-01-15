#include <iostream>
#include <stdexcept>

class MyClass {
public:
    int value;
    MyClass(int v) : value(v) {}
    
    bool operator<(const MyClass& other)
    {
        return value < other.value;
    }
};

// Error: No operator< defined for MyClass

template <typename T>
T min() {
    throw std::invalid_argument("min() requires at least one argument");
}

template <typename T>
T min(T a) {
    return a;
}

template <typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T, typename... Args>
T min(T a, Args... args) {
    return min(a, min(args...));
}

int main() {
    // MyClass test(1);
    min(MyClass(5), MyClass(3)); // Compile-time error: no operator<

    // ouput the results of my class 
    std::cout << "MyClass(5) < MyClass(3): " << (  MyClass(5) < MyClass(3)  ) << std::endl;

    // Empty argument test
    try {
        min<int>(); // Throws std::invalid_argument
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Existing functionality still works
    int min_int = min(10, 2, 5, 3);
    std::cout << "Minimum int: " << min_int << std::endl;

    return 0;
}