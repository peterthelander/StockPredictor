#ifndef STOCK_DATA_H
#define STOCK_DATA_H

#include <vector>
#include "DateHandler.h"

class StockData {
public:
    void loadStock(const std::string& filename, const std::vector<DateLong>& dateGuide);
    void saveStock(const std::string& filename) const;

private:
    std::vector<Daily> data;
};

#endif // STOCK_DATA_H
