#include "StockConverter.h"
#include "FileProcessor.h"

StockConverter::StockConverter()
    : fileProcessorASX("C:\\Peter\\Pisa\\Data\\ASX_Text", "C:\\Peter\\Pisa\\Data\\ASX_Data", dateHandler),
      fileProcessorUS("C:\\Peter\\Pisa\\Data\\US_Text\\index", "C:\\Peter\\Pisa\\Data\\US_Data", dateHandler) {}

void StockConverter::convertFiles() const {
    fileProcessorASX.convertFiles(true);
    fileProcessorUS.convertFiles(false);
}
