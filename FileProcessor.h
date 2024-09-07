#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <string>
#include "DateHandler.h"

class FileProcessor {
public:
    FileProcessor(const std::string& rootPath, const std::string& outPath, const DateHandler& dateHandler);
    void convertFiles(bool isASX) const;

private:
    std::string rootPath;
    std::string outPath;
    const DateHandler& dateHandler;
};

#endif // FILE_PROCESSOR_H
