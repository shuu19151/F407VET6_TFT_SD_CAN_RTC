/*
 * mySDCard.cpp
 *
 *  Created on: Mar 25, 2024
 *      Author: lamqu
 */


#include "mySDCard.h"
#include <cstring>
#include <algorithm>
#include <stack>


#define BUFFER_SIZE     (128)
#define PATH_SIZE       (32)
#define MAXLINE         (12)

char buffer[BUFFER_SIZE];  // to store strings..
char path[PATH_SIZE];  // buffer to store path

static std::string mFileList;

SDCard::SDCard() {
    f_mount(&fs, (TCHAR*)"", 1);
}

FRESULT SDCard::writeToFile(const char* fileName, std::string& data) {
    FRESULT e;
    FILINFO stat;
    e = f_stat((TCHAR*)fileName, &stat);
    if(e != FR_OK) {
        // check if the file is already created, if not then create it
        e = f_open(&file, (TCHAR*)fileName, FA_CREATE_ALWAYS | FA_WRITE);
    } else {
        // if it already exists, open and append the data
        e = f_open(&file, (TCHAR*)fileName, FA_OPEN_ALWAYS | FA_WRITE);
        if (e == FR_OK) {
            // Move the file pointer to the end of the file
            f_lseek(&file, f_size(&file));
        }
    }
    if(e != FR_OK) {
        f_close(&file);
        return e;
    }

    UINT bytesWritten;
    e = f_write(&file, data.c_str(), data.length(), &bytesWritten);
    if (e != FR_OK) {
        f_close(&file);
        return e;
    }

    f_close(&file);
    return FR_OK;
}

UINT SDCard::readTextFromFile(std::string fileName, std::string& data, UINT dataSize, UINT startLine, UINT endLine) {
    UINT bytesRead = 0;
    BYTE buffer[256]; // Assuming maximum line length is 256 bytes
    UINT totalBytesRead = 0;
    UINT currentLineNumber = 0;

    if (f_open(&file, (TCHAR*)fileName.c_str(), FA_READ) == FR_OK) {    // Open the file in read mode
        /* Read lines from the file until the buffer is full or the end of the file is reached */
        while ((f_gets((TCHAR*)buffer, sizeof(buffer), &file) != NULL) && (totalBytesRead < dataSize - 1)) {
            currentLineNumber++;
            if (currentLineNumber >= startLine && currentLineNumber <= endLine) {
                UINT len = strlen((char*)buffer);
                if (totalBytesRead + len < dataSize - 1) {    // Check if the buffer can hold the new line and a null terminator
                    data.append(reinterpret_cast<const char*>(buffer), len);    // Append the line data to the provided string
                    totalBytesRead += len;
                } else { break;    // Not enough space in the buffer for the new line
                }
            }
        }
        bytesRead = totalBytesRead;
        f_close(&file);
    }
    return bytesRead;
}

FRESULT SDCard::makeDir(const char* dirName) {
	return f_mkdir((TCHAR*)dirName);
}

FRESULT SDCard::deleteFile(const char* fileName) {
    return f_unlink((TCHAR*)fileName);
}

static void getFileSizeUnit(uint64_t& fileSize, std::string& sizeUnit) {
    if (fileSize >= 1024 * 1024 * 1024) {
        fileSize /= 1024 * 1024 * 1024;
        sizeUnit = "GB";
    } else if (fileSize >= 1024 * 1024) {
        fileSize /= 1024 * 1024;
        sizeUnit = "MB";
    } else if (fileSize >= 1024) {
        fileSize /= 1024;
        sizeUnit = "kB";
    } else {
        sizeUnit = "B";
    }
}

void SDCard::listEntryOfDir(const char* pat) {
    std::string fileList;
    scanFiles(pat, fileList);
    setFilesMap(fileList);
}

void SDCard::listFilesOnPage(uint8_t page, std::string& lf) {
    uint8_t totalLine = 0;          // Keep track of the number of lines added to the current page
    totalLine = mFilesMap.size();   // Get the number of elements in the mFilesMap map
    uint8_t startLine = (page - 1) * MAXLINE;
    uint8_t endLine = page * MAXLINE;
    if (endLine > totalLine) {
        endLine = totalLine;
    }
    for (uint8_t i = startLine; i < endLine; i++) {
        lf += "  " + mFilesMap[i] + "\n";
    }
}

FRESULT SDCard::scanFiles(const char* pat, std::string& lf) {
    FRESULT res;
    DIR dir;
    static FILINFO stat;
    std::string path(pat);
    res = f_opendir(&dir, pat);                       /* Open the directory */
    if (res == FR_OK) {
        lf += "[" + path + "]\n";
        for (;;) {
            res = f_readdir(&dir, &stat);                   /* Read a directory item */
            if (res != FR_OK || stat.fname[0] == 0) break;  /* Break on error or end of dir */
            if (stat.fattrib & AM_DIR) {                    /* It is a directory */
                if (strcmp("System Volume Information", (char*)stat.fname) != 0) {  // Skip the system directory
                    lf += "[" + std::string(stat.fname) + "]\n";  // Append the directory name to the provided string
                }
            } else {                                       /* It is a file. */
                lf += std::string(stat.fname) + "\n";
            }
        }
        f_closedir(&dir);
    }
    return res;
}

//FRESULT SDCard::listFilesWithSize(const char* pat, uint8_t page) {
//}

void SDCard::getPagesAndLastRow(const char* pat, uint8_t& page, uint8_t& lastRow) {
    std::string fileList;
    FRESULT res = scanFiles(pat, fileList);
    if (res == FR_OK) {
        uint32_t numFiles = std::count(fileList.begin(), fileList.end(), '\n');
        if(numFiles % MAXLINE == 0) {
            page = numFiles / MAXLINE;
            lastRow = MAXLINE;
        } else {
            page = numFiles / MAXLINE + 1;
            lastRow = (numFiles - 1) - (page - 1) * MAXLINE;
        }
    } else {
        // Handle error
        page = 0;
        lastRow = 0;
    }
}

void SDCard::setFilesMap(std::string& fileList) {
    mFilesMap.clear();
    int fileNumber = 0;
    size_t startPos = fileList.find_first_not_of(" \t\n\r\f\v");
    while (startPos != std::string::npos) {
        size_t endPos = fileList.find('\n', startPos);
        std::string line = fileList.substr(startPos, endPos - startPos);

        size_t fileNameStartPos = line.find_first_not_of(" \t");
        size_t fileNameEndPos = line.find_first_of(" \t", fileNameStartPos);
        std::string fileName = line.substr(fileNameStartPos, fileNameEndPos - fileNameStartPos);

        mFilesMap[fileNumber] = fileName;
        fileNumber++;

        startPos = fileList.find_first_not_of(" \t\n\r\f\v", endPos + 1);
    }
}

void SDCard::getFilesMap(std::string& strMap) {
    for(const auto& file : mFilesMap) {
        strMap += " {" + std::to_string(file.first) + "} [" + file.second + "]\n";
    }
}

std::string& SDCard::getFileList(void) {
    return mFileList;
}

file_type_t SDCard::checkSelectedFileType(uint8_t fileOrder, std::string& fn) {
    if (mFilesMap.find(fileOrder) == mFilesMap.end()) return FTYP_DIR;
    std::string fileName = mFilesMap[fileOrder];
    fn += "/";
    fn += fileName;
    if(fileName.find(".TXT") != std::string::npos || fileName.find(".txt") != std::string::npos) {
        return FTYP_TXT;
    }
    else if(fileName.find(".JPG") != std::string::npos || fileName.find(".jpg") != std::string::npos) {
        return FTYP_JPG;
    }
    else {
        return FTYP_DIR;
    }
}
