/*
 * DFPlayer.h
 *
 */

#ifndef DFPLAYER_H_
#define DFPLAYER_H_

#include "main.h" // Chứa các định nghĩa HAL và kiểu dữ liệu (uint8_t...)

// Hàm gửi lệnh RAW chuẩn 10-byte
void DFPlayer_SendCommand(uint8_t command, uint8_t param1, uint8_t param2);

// Các hàm thao tác nhanh (Wrapper functions)
void DFPlayer_Init(uint8_t volume);
void DFPlayer_PlayTrack(uint16_t track_number);
void DFPlayer_Stop(void);

#endif /* DFPLAYER_H_ */
