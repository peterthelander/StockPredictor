#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#pragma warning(disable:4786)

/*
        US data goes: 1-Jan-1968  ... 30-sep-2002
        ASX data goes: Tue 3-Jan-1984 ... 26-Jun-2002
        Common Range: US Mon 2-Jan-1984 ... Thu 20-Jun-2002
                     ASX Tue 3-Jan-1984 ... Fri 21-Jun-2002
*/

typedef int DateLong;

constexpr DateLong DATE_START = 19840102; // Monday 02-Jan-1984
constexpr DateLong DATE_END   = 20020621; // Friday 21-Jun-2002

constexpr const char* PATH_ASX_Text = "C:\\Peter\\Pisa\\Data\\ASX_Text";
constexpr const char* PATH_US_Text  = "C:\\Peter\\Pisa\\Data\\US_Text\\index";
constexpr const char* PATH_ASX_Data = "C:\\Peter\\Pisa\\Data\\ASX_Data";
constexpr const char* PATH_US_Data  = "C:\\Peter\\Pisa\\Data\\US_Data";

constexpr int numASXStocks = 69;
constexpr int numUSStocks = 813;

constexpr int NUM_DATES = 3856 * 5/4; // number of days from start to end (inclusive), taking four weekdays per week

vector<DateLong> US_DateGuide(NUM_DATES);
vector<DateLong> ASX_DateGuide(NUM_DATES);

constexpr int daysInMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

constexpr bool isLeap(int year)
{
    return (year % 4 == 0); // simplified leap year calculation since we only care about the 20th century
}

constexpr void incrementDateLong(DateLong& d)
{
    int year = d / 10000;
    int month = (d % 10000) / 100;
    int day = (d % 100);

    day++;

    int days = daysInMonths[month - 1];
    if (month == 2 && isLeap(year))
        days = 29;

    if (day > days)
    {
        day = 1;
        month++;
        if (month == 13)
        {
            month = 1;
            year++;
        }
    }

    d = (year * 100 + month) * 100 + day;
}

int US_index = 0;
int ASX_index = 0;

void generateDateGuides()
{
    DateLong d = DATE_START; // first Monday, for US stocks

    while (d < DATE_END)
    {
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

typedef float Daily;

typedef vector<Daily> Stock;

constexpr Daily undefined = 99999.0f;

enum StockType { ASX, US };

void loadDataStock(const char* stockName, Stock& stockData)
{
    FILE* fp = fopen(stockName, "rb");
    fread(&stockData[0], NUM_DATES, sizeof(Daily), fp);
    fclose(fp);
}

DateLong StrToDateLong(const char* s) // Not constexpr anymore due to string processing functions
{
    char* dash = strstr(const_cast<char*>(s), "-");
    char* p = dash + 1;
    *dash = 0;

    char* dash2 = strstr(p, "-");
    *dash2 = 0;

    int day = ::atol(s);

    int mon;
    if (strcmp(p, "Jan") == 0) mon = 1;
    else if (strcmp(p, "Feb") == 0) mon = 2;
    else if (strcmp(p, "Mar") == 0) mon = 3;
    else if (strcmp(p, "Apr") == 0) mon = 4;
    else if (strcmp(p, "May") == 0) mon = 5;
    else if (strcmp(p, "Jun") == 0) mon = 6;
    else if (strcmp(p, "Jul") == 0) mon = 7;
    else if (strcmp(p, "Aug") == 0) mon = 8;
    else if (strcmp(p, "Sep") == 0) mon = 9;
    else if (strcmp(p, "Oct") == 0) mon = 10;
    else if (strcmp(p, "Nov") == 0) mon = 11;
    else if (strcmp(p, "Dec") == 0) mon = 12;

    p = dash2 + 1;
    int year = ::atol(p);
    if (year < 50) year += 100;
    year += 1900;

    return (year * 100 + mon) * 100 + day;
}

void loadStock(StockType stockType, const char* stockName, Stock& stockData)
{
    stockData.resize(NUM_DATES, undefined);

    if (strstr(stockName, ".DAT"))
    {
        loadDataStock(stockName, stockData);
        return;
    }

    const char* path;
    const char* ext;
    vector<DateLong>* dateGuide;

    if (stockType == ASX)
    {
        dateGuide = &ASX_DateGuide;
        path = PATH_ASX_Text;
        ext = ".CSV";
    }
    else
    {
        dateGuide = &US_DateGuide;
        path = PATH_US_Text;
        ext = ".ASC";
    }

    string file;

    if (strstr(stockName, "\\"))
    {
        file = stockName; // full filename provided
    }
    else
    {
        file = path;
        file += "\\";
        file += stockName;
        file += ext;
    }

    const int buf_size = 500000;
    static char buf[buf_size];

    memset(buf, 0, buf_size);

    FILE* fp = fopen(file.c_str(), "r");
    fread(buf, buf_size, 1, fp);
    fclose(fp);

    const char* delims = ",\n";
    char* p = strtok(buf, delims);

    if (strcmp(p, "Date") == 0)
    {
        p = strtok(0, "\n"); // skip to data
        p = strtok(0, "\n"); // first data line

        int index = NUM_DATES - 1;
        while (index >= 0)
        {
            char* dateStr = strtok(0, delims);
            if (dateStr == 0) break;

            DateLong d = StrToDateLong(dateStr);
            char* open = strtok(0, delims);
            char* close = strtok(0, delims);

            if (d > (*dateGuide)[index]) continue;
            while (d < (*dateGuide)[index] && index > 0) index--;

            if (index >= 0)
            {
                double logReturn = log(::atof(close) / ::atof(open));
                stockData[index--] = Daily(logReturn);
            }
        }
    }
    else
    {
        // Process non-Yahoo-format stocks here
    }
}

void saveStock(const Stock& stockData, const char* stockName)
{
    FILE* fp = fopen(stockName, "wb");
    fwrite(&stockData[0], NUM_DATES, sizeof(Daily), fp);
    fclose(fp);
}

void convertFiles(StockType stockType)
{
    string root = PATH_US_Text;
    string outPath;

    if (stockType == ASX)
    {
        outPath = PATH_ASX_Data;
    }
    else
    {
        outPath = PATH_US_Data;
    }

    // Use C++17 filesystem to replace dirent.h
    for (const auto& entry : fs::directory_iterator(root))
    {
        const auto& path = entry.path();
        if (path.extension() == ".CSV" || path.extension() == ".ASC")
        {
            Stock stockData;
            loadStock(stockType, path.filename().string().c_str(), stockData);

            auto outFile = path.filename().replace_extension(".DAT").string();
            saveStock(stockData, outFile.c_str());
        }
    }
}

int main()
{
    generateDateGuides();
    convertFiles(US);
    convertFiles(ASX);
    return 0;
}
