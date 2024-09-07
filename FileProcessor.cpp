#include "FileProcessor.h"
#include "StockData.h"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

FileProcessor::FileProcessor(const std::string& rootPath, const std::string& outPath, const DateHandler& dateHandler)
    : rootPath(rootPath), outPath(outPath), dateHandler(dateHandler) {}

void FileProcessor::convertFiles(bool isASX) const {
    const auto& dateGuide = isASX ? dateHandler.getASXDateGuide() : dateHandler.getUSDateGuide();

    for (const auto& entry : fs::directory_iterator(rootPath)) {
        if (entry.is_regular_file()) {
            std::cout << "File: " << entry.path().filename().string() << std::endl;
            std::string path2 = rootPath + "/" + entry.path().filename().string();
            StockData stockData;
            stockData.loadStock(entry.path().filename().string(), dateGuide);

            std::string outFile = outPath + "/" + entry.path().filename().replace_extension(".DAT").string();
            stockData.saveStock(outFile);
        }
    }
}
