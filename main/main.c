#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "hal/uart_types.h"

#define TX_BUFFER_SIZE      1024
#define RX_BUFFER_SIZE      1024

// Physically jumper these two pins together with a wire:
// GPIO17 (UART1 TX)  -----wire-----  GPIO16 (UART2 RX)
#define UART1_TX_PIN        17
#define UART2_RX_PIN        16

void app_main(void)
{
    uart_port_t uart1 = UART_NUM_1;
    uart_port_t uart2 = UART_NUM_2;
    QueueHandle_t queue1, queue2;

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // ---- UART1 (transmitter) ----
    esp_err_t res1 = uart_driver_install(uart1, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 10, &queue1, 0);
    if (res1 == ESP_OK) {
        printf("UART driver 1 installed successfully\n");
    }

    uart_param_config(uart1, &uart_config);

    esp_err_t res2 = uart_set_pin(uart1, UART1_TX_PIN, UART_PIN_NO_CHANGE,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (res2 == ESP_OK) {
        printf("UART1 TX pin (GPIO%d) configured correctly\n", UART1_TX_PIN);
    }

    // ---- UART2 (receiver) ----
    esp_err_t res3 = uart_driver_install(uart2, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 10, &queue2, 0);
    if (res3 == ESP_OK) {
        printf("UART driver 2 installed successfully\n");
    }

    uart_param_config(uart2, &uart_config);

    esp_err_t res4 = uart_set_pin(uart2, UART_PIN_NO_CHANGE, UART2_RX_PIN,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (res4 == ESP_OK) {
        printf("UART2 RX pin (GPIO%d) configured correctly\n", UART2_RX_PIN);
    }

    uint8_t data[100];
    static const char *MSG = "Hello from uart1!\n";

    while (true) {
        uart_write_bytes(uart1, MSG, strlen(MSG));
        printf("UART1 sent : %s\n", MSG);

        int len = uart_read_bytes(uart2, data, sizeof(data) - 1, pdMS_TO_TICKS(1000));

        if (len > 0) {
            data[len] = '\0';
            printf("UART2 received data: %s\n", data);
        } else {
            printf("No data received on uart 2\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}