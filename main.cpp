#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <filesystem>
#include <vector>
#include <stdexcept>

namespace fs = std::filesystem;

constexpr int NUM_DATES = 3856 * 5 / 4; // number of days from start end (inclusive), taking four week days per week
/*
        US data goes: 1-Jan-1968  ... 30-sep-2002
        ASX data goes: Tue 3-Jan-1984 ... 26-Jun-2002
        Common Range: US Mon 2-Jan-1984 ... Thu 20-Jun-2002
                     ASX Tue 3-Jan-1984 ... Fri 21-Jun-2002
*/

typedef int DateLong;

constexpr DateLong DATE_START = 19840102; // Monday 02-Jan-1984
constexpr DateLong DATE_END = 20020621; // Friday 21-Jun-2002

const char* PATH_ASX_Text = "C:\\Peter\\Pisa\\Data\\ASX_Text";
const char* PATH_US_Text = "C:\\Peter\\Pisa\\Data\\US_Text\\index";
const char* PATH_ASX_Data = "C:\\Peter\\Pisa\\Data\\ASX_Data";
const char* PATH_US_Data = "C:\\Peter\\Pisa\\Data\\US_Data";

constexpr int numASXStocks = 69;
constexpr int numUSStocks = 813;

using DateLong = int;
using Daily = float;
using Stock = std::vector<Daily>;

std::vector<DateLong> US_DateGuide(NUM_DATES);
std::vector<DateLong> ASX_DateGuide(NUM_DATES);

//                        J   F   M   A   M   J   J   A   S   O   N   D;
constexpr int daysInMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

constexpr bool isLeap(int year) {
    // Data only covering ~1984 to 2002, so don't care about other centuries like 1900
    return (year % 4 == 0);
}

void incrementDateLong(DateLong& d) {
    int year = d / 10000;
    int month = (d % 10000) / 100;
    int day = (d % 100);

    day++;

    int days = daysInMonths[month - 1];
    if (month == 2 && isLeap(year))
        days = 29;

    if (day > days) {
        day = 1;
        month++;
        if (month == 13) {
            month = 1;
            year++;
        }
    }

    d = (year * 100 + month) * 100 + day;
}

void generateDateGuides() {
    DateLong d = DATE_START; // first Monday, for US stocks
    int US_index = 0;
    int ASX_index = 0;

    while (d < DATE_END) {
        US_DateGuide[US_index++] = d; // take a Monday
        incrementDateLong(d);
        US_DateGuide[US_index++] = d; // take a Tuesday
        ASX_DateGuide[ASX_index++] = d; // take a Tuesday
        incrementDateLong(d);
        US_DateGuide[US_index++] = d; // take a Wednesday
        ASX_DateGuide[ASX_index++] = d; // take a Wednesday
        incrementDateLong(d);
        US_DateGuide[US_index++] = d; // take a Thursday
        ASX_DateGuide[ASX_index++] = d; // take a Thursday
        incrementDateLong(d);
        US_DateGuide[US_index++] = d; // take a Friday
        ASX_DateGuide[ASX_index++] = d; // take a Friday
        incrementDateLong(d); // skip Saturday
        incrementDateLong(d); // skip Sunday
        incrementDateLong(d); // move to next Monday
        ASX_DateGuide[ASX_index++] = d; // take a Monday
    }
}

constexpr Daily undefined = 99999.0;

enum StockType { ASX, US };

void loadDataStock(const std::string& stockName, Stock& stockData) {
    std::ifstream file(stockName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + stockName);
    }
    file.read(reinterpret_cast<char*>(stockData.data()), NUM_DATES * sizeof(Daily));
}

DateLong StrToDateLong(const std::string& s) {
    std::istringstream iss(s);
    std::string dayStr, monStr, yearStr;
    char dash;

    if (!(iss >> dayStr >> dash >> monStr >> dash >> yearStr)) {
        throw std::invalid_argument("Invalid date format");
    }

    int day = std::stoi(dayStr);
    int year = std::stoi(yearStr);
    int mon;

    if (monStr == "Jan") mon = 1;
    else if (monStr == "Feb") mon = 2;
    else if (monStr == "Mar") mon = 3;
    else if (monStr == "Apr") mon = 4;
    else if (monStr == "May") mon = 5;
    else if (monStr == "Jun") mon = 6;
    else if (monStr == "Jul") mon = 7;
    else if (monStr == "Aug") mon = 8;
    else if (monStr == "Sep") mon = 9;
    else if (monStr == "Oct") mon = 10;
    else if (monStr == "Nov") mon = 11;
    else if (monStr == "Dec") mon = 12;
    else throw std::invalid_argument("Invalid month");

    if (year < 50) year += 100;
    year += 1900;

    return (year * 100 + mon) * 100 + day;
}

void loadStock(StockType stockType, const std::string& stockName, Stock& stockData) {
    if (stockName.find(".DAT") != std::string::npos) {
        loadDataStock(stockName, stockData);
        return;
    }

    const std::string path = (stockType == ASX) ? PATH_ASX_Text : PATH_US_Text;
    const std::string ext = (stockType == ASX) ? ".CSV" : ".ASC";
    const std::vector<DateLong>& dateGuide = (stockType == ASX) ? ASX_DateGuide : US_DateGuide;

    std::fill(stockData.begin(), stockData.end(), undefined);

    std::string file = (stockName.find("\\") != std::string::npos) ? stockName : (path + "\\" + stockName + ext);

    std::ifstream inFile(file);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading: " + file);
    }

    std::string line;
    std::getline(inFile, line); // Skip header
    std::getline(inFile, line); // First data line

    int index = NUM_DATES - 1;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string dateStr, openStr, closeStr;

        if (!(std::getline(iss, dateStr, ',') && std::getline(iss, openStr, ',') && std::getline(iss, closeStr))) {
            continue; // Skip malformed lines
        }

        DateLong d = StrToDateLong(dateStr);
        if (d > dateGuide[index]) continue;
        while (d < dateGuide[index] && index > 0) index--;

        if (index >= 0) {
            double logReturn = log(std::stof(closeStr) / std::stof(openStr));
            stockData[index--] = static_cast<Daily>(logReturn);
        }
    }
}

void saveStock(const Stock& stockData, const std::string& stockName) {
    std::ofstream file(stockName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + stockName);
    }
    file.write(reinterpret_cast<const char*>(stockData.data()), NUM_DATES * sizeof(Daily));
}

void convertFiles(StockType stockType) {
    const std::string root = (stockType == ASX) ? PATH_ASX_Text : PATH_US_Text;
    const std::string outPath = (stockType == ASX) ? PATH_ASX_Data : PATH_US_Data;
    const int numStocks = (stockType == ASX) ? numASXStocks : numUSStocks;

    for (int i = 1; i <= numStocks; ++i) {
        Stock stockData(NUM_DATES);
        std::string stockName = (stockType == ASX) ? "ASX" + std::to_string(i) : "US" + std::to_string(i);

        try {
            loadStock(stockType, stockName, stockData);
            saveStock(stockData, outPath + "\\" + stockName + ".DAT");
        } catch (const std::exception& e) {
            std::cerr << "Error processing stock " << stockName << ": " << e.what() << std::endl;
        }
    }
}

int main() {
    try {
        generateDateGuides();
        convertFiles(ASX);
        convertFiles(US);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
