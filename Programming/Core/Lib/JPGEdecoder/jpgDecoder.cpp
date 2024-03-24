/*
 * jpgDecoder.cpp
 *
 *  Created on: Mar 23, 2024
 *      Author: lamqu
 */


#include "jpgDecoder.h"

// bool (*JpgDecoder::renderFun)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data);
RenderFunction JpgDecoder::renderFun;
const uint8_t* JpgDecoder::data = 0;
uint32_t JpgDecoder::dataIdx = 0;
uint32_t JpgDecoder::dataLen = 0;
int16_t JpgDecoder::startX = 0;
int16_t JpgDecoder::startY = 0;

// JpgDecoder::JpgDecoder(bool (*fun)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data)) {
//   renderFun = fun;
// }

JpgDecoder::JpgDecoder(RenderFunction fun) {
  renderFun = fun;
	f_mount(&fs, (TCHAR*)"", 1);
}

uint16_t JpgDecoder::loadMem(JDEC* jdec, uint8_t* buf, uint16_t len) {
  if(dataIdx + len > dataLen) len = dataLen - dataIdx;
  if(buf) {
	  memcpy(buf, (const uint8_t *)(data + dataIdx), len);
  }
  dataIdx += len;
  return len;
}

uint16_t JpgDecoder::renderLCD(JDEC* jdec, void* bitmap, JRECT* jrect) {
  int16_t  x = jrect->left + startX;
  int16_t  y = jrect->top  + startY;
  uint16_t w = jrect->right  + 1 - jrect->left;
  uint16_t h = jrect->bottom + 1 - jrect->top;
  return renderFun ? (*renderFun)(x, y, w, h, (uint16_t*)bitmap) : 0;
}
// --------------------------------------------------

FIL JpgDecoder::file;
FATFS JpgDecoder::fs;

uint16_t JpgDecoder::loadSD(JDEC *jdec, uint8_t* buf, uint16_t len) {
  if(buf) {
    f_read((&file), buf, len, (UINT*)&len);
  }
  else {
    f_lseek(&file, f_tell(&file) + len);
  }
  return len;
}

JRESULT JpgDecoder::show(int16_t x, int16_t y, const char *fileName) {
  f_open(&file, (TCHAR*)fileName, FA_READ);

  JDEC jdec;
  JRESULT res = JDR_OK;
  startX = x;
  startY = y;

  jdec.swap = _swap;

  res = jd_prepare(&jdec, reinterpret_cast<size_t (*)(JDEC*, uint8_t*, size_t)>(loadSD), workspace, TJPGD_WORKSPACE_SIZE, 0);
  if(res == JDR_OK) {
    res = jd_decomp(&jdec, reinterpret_cast<int (*)(JDEC*, void*, JRECT*)>(renderLCD), jpgScale);
  }
  f_close(&file);
  return res;
}

JRESULT JpgDecoder::getDim(uint16_t *w, uint16_t *h, const char *fileName) {
  f_open(&file, (TCHAR*)fileName, FA_READ);
  JDEC jdec;
  JRESULT res;
  *w = *h = 0;
  res = jd_prepare(&jdec, reinterpret_cast<size_t (*)(JDEC*, uint8_t*, size_t)>(loadSD), workspace, TJPGD_WORKSPACE_SIZE, 0);

  if(res == JDR_OK) {
    *w = jdec.width;
    *h = jdec.height;
  }

  f_close(&file);
  return res;
}


/***************************************************************************************
** Function name:           setJpgScale
** Description:             Set the reduction scale factor (1, 2, 4 or 8)
***************************************************************************************/
void JpgDecoder::setJpgScale(uint8_t scaleFactor) {
  switch (scaleFactor) {
    case 1: jpgScale = 0; break;
    case 2: jpgScale = 1; break;
    case 4: jpgScale = 2; break;
    case 8: jpgScale = 3; break;
    default: jpgScale = 0;
  }
}
