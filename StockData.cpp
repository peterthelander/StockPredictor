#include "StockData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstring>

void StockData::loadStock(const std::string& filename, const std::vector<DateLong>& dateGuide) {
    data.assign(dateGuide.size(), undefined);

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header line
    std::getline(file, line); // Read first data line

    int index = dateGuide.size() - 1;
    while (index >= 0 && std::getline(file, line)) {
        std::istringstream ss(line);
        std::string dateStr, openStr, closeStr;

        std::getline(ss, dateStr, ',');
        std::getline(ss, openStr, ',');
        std::getline(ss, closeStr, ',');

        DateLong d = StrToDateLong(dateStr.c_str());
        if (d > dateGuide[index]) continue;
        while (d < dateGuide[index] && index > 0) index--;

        if (index >= 0) {
            double open = std::stod(openStr);
            double close = std::stod(closeStr);
            double logReturn = std::log(close / open);
            data[index--] = static_cast<Daily>(logReturn);
        }
    }

    file.close();
}

void StockData::saveStock(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(Daily));
    file.close();
}
