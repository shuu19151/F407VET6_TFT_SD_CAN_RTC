/*
 * jpgDecoder.h
 *
 *  Created on: Mar 23, 2024
 *      Author: lamqu
 */

#ifndef _JPGDECODER_H_
#define _JPGDECODER_H_

#include "fatfs.h"
#include "tjpgd.h"

typedef bool (*RenderFunction)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data);

class JpgDecoder {
private:
  static FIL file;
	static FATFS fs;
  static const uint8_t *data;

  static uint32_t dataIdx;
  static uint32_t dataLen;
  static int16_t startX;
  static int16_t startY;

  uint8_t jpgScale = 0;

  bool _swap = false;

  // Must align workspace to a 32 bit boundary
  uint8_t workspace[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4)));

  // static bool (*renderFun)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data);
  static RenderFunction renderFun;

public:
  // JpgDecoder(bool (*fun)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data));
  JpgDecoder(RenderFunction fun);
  
  uint8_t jpg_source = 0;
  JpgDecoder *thisPtr = nullptr;

  void setJpgScale(uint8_t scale);

  static uint16_t renderLCD(JDEC *jdec, void *bitmap, JRECT *jrect);
  static uint16_t loadMem(JDEC *jdec, uint8_t *buf, uint16_t len);
  // jpgScale: 0->no scale, 1->1:2, 2->1:4, 3->1:8

  static uint16_t loadSD(JDEC *jdec, uint8_t *buf, uint16_t len);
  JRESULT show(int16_t x, int16_t y, const char *fileName);
  JRESULT getDim(uint16_t *w, uint16_t *h, const char *fileName);
  void setSwapBytes(bool swap) { _swap = swap; }
};


#endif /* _JPGDECODER_H_ */
