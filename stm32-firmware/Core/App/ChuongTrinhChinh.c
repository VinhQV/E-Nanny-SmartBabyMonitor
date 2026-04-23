/*
 * ChuongTrinhChinh.c
 *
 * Created on: Apr 9, 2026
 * Author: vinhv
 */

#include "ChuongTrinhChinh.h"
#include "DFPlayer.h"

extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

volatile DHT22_Data_t Nanny_EnvData;

// ================== BIEN CHO UART STATE MACHINE ==================
uint8_t rx_byte;
uint8_t rx_buffer[16];
uint8_t rx_index = 0;
uint8_t frame_len = 0;
uint8_t frame_ready = 0;

typedef enum {
    WAIT_HEADER_1, WAIT_HEADER_2, WAIT_LENGTH, WAIT_PAYLOAD_AND_CRC
} RX_STATE;
RX_STATE rx_state = WAIT_HEADER_1;

// ================== BIEN CHO CHUC NANG RU NOI ==================
uint8_t is_rocking = 0;
int16_t current_angle = 90;
int8_t rock_dir = 1;
uint32_t last_rock_time = 0;

// ================== HAM DIEU KHIEN SERVO CO BAN ==================
void Set_Servo_Angle(uint8_t angle) {
    if (angle > 180) angle = 180;
    uint32_t ccr_val = 1000 + (angle * 1000 / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_val);
}

// ================== HAM GUI DATA DHT22 LEN ESP32 ==================
// Ham nay dong goi nhiet do va do am roi gui sang ESP32 thong qua UART
void Send_DHT22_To_ESP32(float temp, float hum) {
    uint8_t tx_buf[8];
    uint8_t t_int = (uint8_t)temp; // Lay phan nguyen nhiet do de gui cho nhanh
    uint8_t h_int = (uint8_t)hum;  // Lay phan nguyen do am

    uint8_t frame_len = 3;         // Chieu dai payload = Command(1) + Temp(1) + Hum(1)
    uint8_t command = 0x03;        // Ma lenh 0x03 tuong ung voi data moi truong
    uint8_t crc = frame_len ^ command ^ t_int ^ h_int; // Tinh CRC theo chuan ESP32

    tx_buf[0] = 0xAA;       // Header 1
    tx_buf[1] = 0x55;       // Header 2
    tx_buf[2] = frame_len;  // Length
    tx_buf[3] = command;    // Command
    tx_buf[4] = t_int;      // Data 1 (Nhiet do)
    tx_buf[5] = h_int;      // Data 2 (Do am)
    tx_buf[6] = crc;        // Checksum CRC

    // Gui mang array gom 7 byte qua UART sang ESP32
    HAL_UART_Transmit(&huart1, tx_buf, 7, 100);
}

// ================== XU LY LENH TU ESP32 ==================
void Process_Command(void) {
    uint8_t command = rx_buffer[0];
    uint8_t data = rx_buffer[1];

    switch (command) {
        case 0x01: // Lenh dieu khien DFPlayer Phat nhac / Dung nhac
            if (data > 0) {
                DFPlayer_PlayTrack(data);
            } else if (data == 0) {
                DFPlayer_Stop();
            }
            break;

        case 0x02: // Lenh Servo quay goc co dinh
            if (data <= 180) {
                is_rocking = 0;
                Set_Servo_Angle(data);
            }
            break;

        case 0x03: // Yeu cau tra nhiet do do am ve ESP32 bat dong bo
            Send_DHT22_To_ESP32(Nanny_EnvData.Temperature, Nanny_EnvData.Humidity);
            break;

        case 0x04: // Lenh Bat/Tat chuc nang lac lu ru noi
            if (data == 1) {
                is_rocking = 1;
            } else {
                is_rocking = 0;
                Set_Servo_Angle(90);
            }
            break;
    }
}

// ================== HAM NGAT NHAN UART to ESP32 ==================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        switch (rx_state) {
            case WAIT_HEADER_1:
                if (rx_byte == 0xAA) rx_state = WAIT_HEADER_2;
                break;
            case WAIT_HEADER_2:
                if (rx_byte == 0x55) rx_state = WAIT_LENGTH;
                else rx_state = WAIT_HEADER_1;
                break;
            case WAIT_LENGTH:
                frame_len = rx_byte;
                rx_index = 0;
                if (frame_len > 0 && frame_len < 12) rx_state = WAIT_PAYLOAD_AND_CRC;
                else rx_state = WAIT_HEADER_1;
                break;
            case WAIT_PAYLOAD_AND_CRC:
                rx_buffer[rx_index++] = rx_byte;
                if (rx_index == (frame_len + 1)) {
                    uint8_t calc_crc = frame_len;
                    for (int i = 0; i < frame_len; i++) {
                        calc_crc ^= rx_buffer[i];
                    }
                    if (calc_crc == rx_buffer[frame_len]) frame_ready = 1;
                    rx_state = WAIT_HEADER_1;
                }
                break;
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

// ================== HAM CHAY NEN CHO SERVO LAC LU ==================
void Servo_Rocking_Task(void) {
    if (!is_rocking) return;

    // Giam toc do lac lu xuong thanh 20ms moi buoc de em be khong bi chong mat
    if (HAL_GetTick() - last_rock_time >= 20) {
        last_rock_time = HAL_GetTick();
        current_angle += rock_dir;

        // Gioi han goc quay an toan cua noi tu 60 do den 120 do
        // Khong de quay het vong 0 -> 180 de chong lat noi em be
        int16_t safe_max_angle = 120;
        int16_t safe_min_angle = 60;

        if (current_angle >= safe_max_angle) {
            current_angle = safe_max_angle;
            rock_dir = -1;
        } else if (current_angle <= safe_min_angle) {
            current_angle = safe_min_angle;
            rock_dir = 1;
        }

        Set_Servo_Angle(current_angle);
    }
}

// ================== CHUONG TRINH CHINH ==================
void ChuongTrinh_Init(void) {
    HAL_TIM_Base_Start(&htim11);

    Nanny_EnvData.Temperature = 0.0f;
    Nanny_EnvData.Humidity = 0.0f;

    // 1. Khoi tao Servo
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    Set_Servo_Angle(90);

    DFPlayer_Init(20);

    DFPlayer_PlayTrack(1);

    // Bat ngat nhan UART tu ESP32
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void ChuongTrinh_Loop(void) {
    static uint32_t last_read_time = 0;

    // Doc DHT22 moi 2 giay de lay thong tin moi truong
    if (HAL_GetTick() - last_read_time >= 2000) {
        last_read_time = HAL_GetTick();
        if (DHT22_Read(&Nanny_EnvData) == 1) {
            // Chu dong day du lieu len ESP32 theo chu ky
            Send_DHT22_To_ESP32(Nanny_EnvData.Temperature, Nanny_EnvData.Humidity);
        }
    }

    if (frame_ready) {
        Process_Command();
        frame_ready = 0;
    }

    Servo_Rocking_Task();
}
