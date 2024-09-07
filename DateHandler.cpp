#include "DateHandler.h"
#include <algorithm> // For std::fill
#include <stdexcept> // For std::runtime_error

std::vector<DateLong> US_DateGuide(NUM_DATES);
std::vector<DateLong> ASX_DateGuide(NUM_DATES);

void DateHandler::generateDateGuides() {
    DateLong d = DATE_START; // Start date
    int usIndex = 0;
    int asxIndex = 0;

    while (d < DATE_END) {
        if (usIndex < NUM_DATES) {
            US_DateGuide[usIndex++] = d; // US stocks
        }
        if (asxIndex < NUM_DATES) {
            ASX_DateGuide[asxIndex++] = d; // ASX stocks
        }
        incrementDateLong(d);
        if (usIndex < NUM_DATES) {
            US_DateGuide[usIndex++] = d; // US stocks
        }
        if (asxIndex < NUM_DATES) {
            ASX_DateGuide[asxIndex++] = d; // ASX stocks
        }
        incrementDateLong(d);
        if (usIndex < NUM_DATES) {
            US_DateGuide[usIndex++] = d; // US stocks
        }
        if (asxIndex < NUM_DATES) {
            ASX_DateGuide[asxIndex++] = d; // ASX stocks
        }
        incrementDateLong(d);
        if (usIndex < NUM_DATES) {
            US_DateGuide[usIndex++] = d; // US stocks
        }
        if (asxIndex < NUM_DATES) {
            ASX_DateGuide[asxIndex++] = d; // ASX stocks
        }
        incrementDateLong(d);
        incrementDateLong(d); // skip Saturday and Sunday
        incrementDateLong(d); // move to next Monday
        if (asxIndex < NUM_DATES) {
            ASX_DateGuide[asxIndex++] = d; // ASX stocks
        }
    }
}

const std::vector<DateLong>& DateHandler::getUSDateGuide() {
    return US_DateGuide;
}

const std::vector<DateLong>& DateHandler::getASXDateGuide() {
    return ASX_DateGuide;
}

void DateHandler::incrementDateLong(DateLong& d) {
    int year = d / 10000;
    int month = (d % 10000) / 100;
    int day = (d % 100);

    day++;

    int days = daysInMonths[month - 1];
    if (month == 2 && isLeap(year)) {
        days = 29;
    }

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

bool DateHandler::isLeap(int year) {
    // Simple leap year check
    return (year % 4 == 0);
}
