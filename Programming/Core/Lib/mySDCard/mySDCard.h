/*
 * mySDCard.h
 *
 *  Created on: Mar 25, 2024
 *      Author: lamqu
 */

#ifndef _MYSDCARD_H_
#define _MYSDCARD_H_

#include <cstdarg>
#include <iostream>
#include <map>
#include <string>

#include "fatfs.h"

typedef enum {
    FTYP_DIR = 0,
    FTYP_TXT,
    FTYP_JPG,
} file_type_t;

class SDCard {
private:
    FIL file;
	FATFS fs;
    std::map<int, std::string> mFilesMap;
public:
	SDCard();
    ~SDCard();
    FRESULT writeToFile(const char* fileName, const std::string& fmt, ...);
    UINT readTextFromFile(std::string fileName, std::string& data, UINT dataSize);
    FRESULT makeDir(const char* dirName);
    FRESULT deleteFile(const char* fileName);
    FRESULT scanFiles(const char* pat, std::string& lf);
    void listEntryOfDir(const char* pat);
    void listFilesOnPage(uint8_t page, std::string& lf);
    FRESULT listFilesWithSize(const char* pat, uint8_t page);
    void getPagesAndLastRow(const char* pat, uint8_t& page, uint8_t& lastRow);
    void setFilesMap(std::string& fileList);
    void getFilesMap(std::string& strMap);
    std::string& getFileList(void);
    file_type_t checkSelectedFileType(uint8_t fileOrder, std::string& fn);
};

#endif /* _MYSDCARD_H_ */
