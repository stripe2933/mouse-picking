#include <iostream>
#include <stdexcept>

#include <fmt/ostream.h>

#include "AppWindow.hpp"

int main() {
    try {
        AppWindow window{};
        window.run();
    }
    catch (const std::runtime_error &e) {
        fmt::println(std::cerr, "{}", e.what());
        return 1;
    }

    return 0;
}
