#ifndef STOCK_CONVERTER_H
#define STOCK_CONVERTER_H

#include "DateHandler.h"
#include "FileProcessor.h"

class StockConverter {
public:
    StockConverter();
    void convertFiles() const;

private:
    DateHandler dateHandler;
    FileProcessor fileProcessorASX;
    FileProcessor fileProcessorUS;
};

#endif // STOCK_CONVERTER_H
