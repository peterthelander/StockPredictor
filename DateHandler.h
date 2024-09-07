#ifndef DATEHANDLER_H
#define DATEHANDLER_H

#include <vector>
#include <cstdint>

typedef int32_t DateLong; // Ensure DateLong is defined
const DateLong DATE_START = 19840102;
const DateLong DATE_END = 20020621;
const int NUM_DATES = 3856 * 5 / 4;

class DateHandler {
public:
    // Public methods to interact with DateHandler
    void generateDateGuides();
    const std::vector<DateLong>& getUSDateGuide() const;
    const std::vector<DateLong>& getASXDateGuide() const;

private:
    // Static members to hold date guides
    static std::vector<DateLong> US_DateGuide;
    static std::vector<DateLong> ASX_DateGuide;

    // Helper methods
    void incrementDateLong(DateLong& d);
    bool isLeap(int year) const;

    // Static array for days in months
    static const int daysInMonths[12];
};

#endif // DATEHANDLER_H
