/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *c
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

void UART2_Init(void);
void Keypad_Init(void);
void GPIO_Init(void);
void TIM2_Init(void);

UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;
GPIO_InitTypeDef GPIO_InitStruct;

char keymap[3][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'}
};

uint32_t row_pins[3] = {GPIO_PIN_11, GPIO_PIN_10, GPIO_PIN_9};
uint32_t col_pins[3] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_5};

String contrasena = "123456";
String cadena = "";

int contador = 0;
long inicio = 0;
bool ya_paso_un_minuto = false;
char command;
bool auto_apagado = true;
bool sasactivado = false;
int puerta_izquierda;
int puerta_derecha;
int cofre;
int cajuela;
int check;

void delay(){
    for(int i = 0; i < 400000; i++);
}

int main(void){
    HAL_Init();
    UART2_Init();
    Keypad_Init();
    GPIO_Init();
    TIM2_Init();

    while(1){
        check = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

        if(check){
            auto_apagado = false;
            check = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
            printf("prendido\n");
        } else {
            printf("apagado\n");
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            auto_apagado = true;
        }

        if(auto_apagado){
            int valor = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);

            while(valor){
                sasactivado = true;
                valor = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
            }

            while(sasactivado){
                puerta_izquierda = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
                puerta_derecha = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A3);
                cajuela = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A0);
                cofre = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A1);

                if(puerta_izquierda || puerta_derecha || cajuela || cofre){
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

                    while(true){
                        if(Serial.available() > 0){
                            command = Serial.read();

                            if(command == '1'){
                                cadena = "";
                                sasactivado = false;
                                contador = 0;
                                break;
                            } else if(command == '0'){
                                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_A2, GPIO_PIN_RESET);
                            }
                        }

                        if(contador == 6){
                            if(cadena == contrasena){
                                cadena = "";
                                sasactivado = false;
                                contador = 0;
                                break;
                            } else {
                                cadena = "";
                                printf("CONTRASENA EQUIVOCADA\n");
                                contador = 0;
                            }
                        } else {
                            char keypressed = myKeypad.getKey();
                            if (keypressed != NO_KEY){
                                cadena += keypressed;
                                contador++;
                            }
                        }
                    }
                    printf("CONTRASENA CORRECTA\n");
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
                }
            }
        } else {
            int valor = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);

            while(valor){
                sasactivado = true;
                valor = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
            }

            while(sasactivado){
                puerta_izquierda = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
                puerta_derecha = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A3);
                cajuela = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A0);
                cofre = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_A1);

                if(puerta_izquierda || puerta_derecha || cajuela || cofre){
                    while(true){
                        if(inicio >= 9900 && !ya_paso_un_minuto){
                            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
                            ya_paso_un_minuto = true;
                        }
                        if(contador == 6){
                            if(cadena == contrasena){
                                cadena = "";
                                sasactivado = false;
                                contador = 0;
                                break;
                            } else {
                                cadena = "";
                                printf("CONTRASENA EQUIVOCADA\n");
                                contador = 0;
                            }
                        } else {
                            char keypressed = myKeypad.getKey();
                            if (keypressed != NO_KEY){
                                cadena += keypressed;
                                contador++;
                            }
                        }
                        inicio++;
                        printf("%ld\n", inicio);
                    }

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
                    printf("CONTRASENA CORRECTA\n");
                    inicio = 0;
                    ya_paso_un_minuto = false;
                }
            }
        }
    }
    return 0;
}

void UART2_Init(void){
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    if(HAL_UART_Init(&huart2) != HAL_OK){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
        while(1);
    }
}

void Keypad_Init(void){
    // Initialize GPIO pins for the keypad
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    for(int i = 0; i < 3; i++){
        GPIO_InitStruct.Pin = row_pins[i];
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = col_pins[i];
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void GPIO_Init(void){
    // Initialize GPIO pins for the system
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = GPIO_PIN_A0 | GPIO_PIN_A1 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_12 | GPIO_PIN_A3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_8 | GPIO_PIN_A2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM2_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if(HAL_TIM_Base_Init(&htim2) != HAL_OK){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
        while(1);
    }
}
