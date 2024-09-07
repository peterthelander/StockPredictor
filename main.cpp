#include "StockConverter.h"
#include <iostream>

int main() {
    try {
        StockConverter converter;
        converter.convertFiles();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
