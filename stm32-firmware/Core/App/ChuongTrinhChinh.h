/*
 * ChuongTrinhChinh.h
 *
 * Created on: Apr 9, 2026
 * Author: vinhv
 */

#ifndef APP_CHUONGTRINHCHINH_H_
#define APP_CHUONGTRINHCHINH_H_

// Bao gom thu vien main.h de nhan dien cac cau hinh chan GPIO va ngat tu CubeMX
#include "main.h"
#include "DHT22.h"

// Cho phep cac file khac doc duoc du lieu nhiet do do am cua noi
extern volatile DHT22_Data_t Nanny_EnvData;

// Khai bao nguyen mau hai ham cot loi de main.c co the nhin thay va theu goi
void ChuongTrinh_Init(void);
void ChuongTrinh_Loop(void);

#endif /* APP_CHUONGTRINHCHINH_H_ */
