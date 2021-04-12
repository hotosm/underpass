#include <boost/asio.hpp>
#include <iostream>

void foo(void) {
    std::cout << "Foooo!" << std::endl;
};


int main() {
    std::string bstr = " world";
    // auto f2 = [&bstr](const std::string &x) {
    auto f2 = [&bstr](const std::string &x) {
        std::cout << "Foo: " << x << std::endl;
        foo();
    };

    int large_object = 10;
    auto f1 = [&large_object](int x) {
        std::cout << "this is a very large object that needs passed by ref " << large_object << std::endl;
        std::cout << x << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds {3});
        std::cout << "DONE SLEEPING!" << std::endl;
    };

    int processor_count = 2;  // consider that this CPU has two processors

    // Launch the pool with n threads.
    boost::asio::thread_pool pool(processor_count);

    int x = 2;
    // Submit a function to the pool.
    boost::asio::post(pool, [x, f1]{f1(x);});
    std::string y = "hello";
    boost::asio::post(pool, [y, f2]{f2(y);});
    // boost::asio::post(pool, foo);

    pool.join();

    return 0;
}

