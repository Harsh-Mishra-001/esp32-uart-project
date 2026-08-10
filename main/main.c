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
#define UART2_TX_PIN        17
#define UART2_RX_PIN        16
#define UART1_TX_PIN		25
#define UART1_RX_PIN		26

uart_port_t uart1 = UART_NUM_1;
uart_port_t uart2 = UART_NUM_2;
QueueHandle_t queue1, queue2;
    
void uart1_to_uart2_task(void *arg)
{
	uint8_t data[100];
	static const char *MSG = "Hello from uart 1!\n";
	
	while (true) {
		uart_write_bytes(uart1, MSG, strlen(MSG));
		printf("UART1 send data : %s\n", MSG);
		int len = uart_read_bytes(uart2, data, sizeof(data) - 1, pdMS_TO_TICKS(1000));
		
		if (len > 0) {
			data[len] = '\0';
			printf("UART2 received : %s\n", data);
		}
		
		else {
			printf("No data received on uart 2\n");
		}
		
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void uart2_to_uart1_task(void *arg)
{
	uint8_t data[100];
	static const char *MSG = "Hello from uart 2!\n";
	
	while (true) {
		uart_write_bytes(uart2, MSG, strlen(MSG));
		printf("UART2 send data : %s\n", MSG);
		int len = uart_read_bytes(uart1, data, sizeof(data) - 1, pdMS_TO_TICKS(1000));
		
		if (len > 0) {
			data[len] = '\0';
			printf("UART1 received : %s\n", data);
		}
		
		else {
			printf("No data received on uart 1\n");
		}
		
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void app_main(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    esp_err_t res1 = uart_driver_install(uart1, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 10, &queue1, 0);
    if (res1 == ESP_OK) {
        printf("UART driver 1 installed successfully\n");
    }

    uart_param_config(uart1, &uart_config);

    esp_err_t res2 = uart_set_pin(uart1, UART1_TX_PIN, UART1_RX_PIN,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (res2 == ESP_OK) {
        printf("UART1 TX pin (GPIO%d) configured correctly\n", UART1_TX_PIN);
    }
    else {
    printf("UART1 TX pin (GPIO%d) not configured\n", UART1_TX_PIN);
	}

    esp_err_t res3 = uart_driver_install(uart2, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 10, &queue2, 0);
    if (res3 == ESP_OK) {
        printf("UART driver 2 installed successfully\n");
    }

    uart_param_config(uart2, &uart_config);

    esp_err_t res4 = uart_set_pin(uart2, UART2_TX_PIN, UART2_RX_PIN,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (res4 == ESP_OK) {
        printf("UART2 RX pin (GPIO%d) configured correctly\n", UART2_RX_PIN);
    }
    else {
		printf("UART2 RX pin (GPIO%d) not configured\n", UART2_RX_PIN);
	}

 	printf("\n");
    printf("====================================\n");
    printf(" UART1 <--> UART2 Full Duplex Test\n");
    printf("====================================\n\n");

    // --------------------------------------------------
    // Start both communication tasks
    // --------------------------------------------------
    
    //xTaskCreate(uart1_to_uart2_task, "uart1_to_uart2_task", 4096, NULL, 5, NULL);
	xTaskCreate(uart2_to_uart1_task, "uart2_to_uart1_task", 4096, NULL, 5, NULL);

}