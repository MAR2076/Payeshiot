/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "SIM800C.h"
#include "Network.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

GSM_Result_t res;
bool AT, SIM;

int __io_putchar(int ch)
{
    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)&ch,
        1,
        HAL_MAX_DELAY);

    return ch;
}

void HAL_UART_RxCpltCallback(
        UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
    	//printf("%c", gsm_rx_byte);
        if(gsm_rx_index <
           GSM_RX_BUFFER_SIZE-1)
        {
            gsm_rx_buffer[gsm_rx_index++] =
                    gsm_rx_byte;

            gsm_rx_buffer[gsm_rx_index] = 0;
        }

        HAL_UART_Receive_IT(
            &huart1,
            &gsm_rx_byte,
            1);
    }
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("System Start\r\n");

  HAL_GPIO_WritePin(PWR_K_GPIO_Port, PWR_K_Pin, GPIO_PIN_SET);
  HAL_Delay(2000);
  HAL_GPIO_WritePin(PWR_K_GPIO_Port, PWR_K_Pin, GPIO_PIN_RESET);
  HAL_Delay(5000);


  HAL_UART_Receive_IT(&huart1, &gsm_rx_byte, 1);

  //HAL_Delay(5000);

  GSM_ClearBuffer();

  GSM_InitResult_t gsmResult;

  gsmResult = GSM_Init();

  if(gsmResult == GSM_INIT_OK)
  {
      printf("SYSTEM READY\r\n");
  }
  else
  {
      printf("GSM INIT FAILED : %d\r\n", gsmResult);
  }

  Network_Init();

  char op[32];

  if(GSM_GetOperator(op, sizeof(op)))
  {
      printf("Operator = %s\r\n", op);
  }

  if(GSM_GPRS_Open("mcinet", "", ""))
  {
      char ip[32];

      if(GSM_GPRS_GetIP(ip, sizeof(ip)))
      {
          printf("IP = %s\r\n", ip);
      }
  }
  else
  {
      printf("GPRS FAILED\r\n");
  }

  char httpResponse[1024];

  if(GSM_HTTP_GET(
          "https://httpbin.org/get",
          httpResponse,
          sizeof(httpResponse)))
  {
      printf("GET SUCCESS\r\n");
      printf("%s\r\n", httpResponse);
  }
  else
  {
      printf("GET FAILED\r\n");
  }

  char response[1024];

  const char *json =
  "{\n"
  "  \"protocol\": 1,\n"
  "  \"device\": {\n"
  "    \"id\": \"CAR001\",\n"
  "    \"fw\": \"1.0.0\"\n"
  "  },\n"
  "  \"records\": [\n"
  "    {\n"
  "      \"measure_at\": 1750400000,\n"
  "      \"lat\": 35.72134,\n"
  "      \"lon\": 51.33829,\n"
  "      \"distance\": 152.5,\n"
  "      \"speed\": 68.4,\n"
  "      \"fuel_usage\": 5.8,\n"
  "      \"rpm\": 2450,\n"
  "      \"cell_signal\": 27,\n"
  "      \"operator\": \"MCI\",\n"
  "      \"temp\": 26.3,\n"
  "      \"retry_count\": 0\n"
  "    }\n"
  "  ]\n"
  "}";

  if(GSM_HTTP_POST(
          "https://httpbin.org/post",
          json,
          response,
          sizeof(response)))
  {
      printf("%s\r\n", response);
  }

  res = GSM_SendCommandEx(
      "AT+GMR\r\n",
      15000,
      "OK",
      "ERROR");

  printf("%s\r\n", gsm_rx_buffer);

  /*GSM_Result_t res1;

  res1 = GSM_SendCommandEx(
                "AT+CGATT?\r\n",
                5000,
                "OK",
                "ERROR");

        printf("AT+CGATT? Result = %d\r\n", res1);
        printf("%s\r\n", gsm_rx_buffer);

  res1 = GSM_SendCommandEx(
              "AT+CGREG?\r\n",
              5000,
              "OK",
              "ERROR");

      printf("AT+CGREG? Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);

      res1 = GSM_SendCommandEx(
                  "AT+SAPBR=2,1\r\n",
                  5000,
                  "OK",
                  "ERROR");

      //printf("\r\n========== SAPBR ==========\r\n");
      printf("Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);

      GSM_SendCommandEx("AT+HTTPTERM\r\n", 2000, "OK", "ERROR");

      res1 = GSM_SendCommandEx(
                        "AT+HTTPINIT\r\n",
                        5000,
                        "OK",
                        "ERROR");

      printf("Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);

      res1 = GSM_SendCommandEx("AT+HTTPPARA=\"CID\",1\r\n", 2000, "OK", "ERROR");

      printf("Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);

      GSM_SendCommandEx("AT+HTTPPARA=\"URL\",\"http://httpbin.org/get\"\r\n", 3000, "OK", "ERROR");

      res1 = GSM_SendCommandEx("AT+HTTPACTION=0\r\n", 15000, "+HTTPACTION", "ERROR");
      printf("Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);

      res1 = GSM_SendCommandEx("AT+HTTPREAD\r\n", 5000, "OK", "ERROR");
      printf("Result = %d\r\n", res1);
      printf("%s\r\n", gsm_rx_buffer);*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWR_K_GPIO_Port, PWR_K_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PWR_K_Pin */
  GPIO_InitStruct.Pin = PWR_K_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(PWR_K_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
