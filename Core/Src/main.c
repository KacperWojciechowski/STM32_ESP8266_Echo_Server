/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Total number of configuration commands
#define COMMANDS 12
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// ESP8266 module receive buffer
char data_r[1000];

// message received buffer
char message[100];

// Echo message ready flag
int ready_to_send = 0;

// Config commands table for ESP8266
const char* config[] = {
		"AT+RST\r\n",								// Restart the module
		"ATE0\r\n",									// Disable echo of commands
		"AT\r\n",									// Test communication
		"AT+GMR\r\n",								// Check software version
		"AT+CWMODE=1\r\n",							// Operate as client
		"AT+CIPMODE=0\r\n",							// Choose receive data format
		"AT+CIPMUX=1\r\n",							// Configure multiple connections
		"AT+CIPSERVER=1,7\r\n",						// Enable server and set port to 7
		"AT+CWMODE=?\r\n",							// Check operation mode
		"AT+CWJAP=\"KAJA_K14583\",\"****************\"\r\n",	// Connect to WiFi with given SSID and password
		"AT+CIPSTA?\r\n",							// Check connection status
		"AT+CIFSR\r\n"								// Display server IP address
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// Clear buffer function
void clr_buffer(char* buff, int size);

// Function checking whether the module is busy
int is_busy(void);

// Function preparing the echo message
void prepare_echo(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Function clearing the given buffer
void clr_buffer(char* buff, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		buff[i] = 0;
	}
}

// Function checking whether the module answered that it's busy
int is_busy(void)
{
	// Return value variable
	int ret = 0;

	// Check if the receive buffer contains substring "busy"
	if (strstr(data_r, "busy") != NULL)
	{
		ret = 1;
	}

	return ret;
}

void prepare_echo(void)
{
	// Message length buffer (works with messages up to 99 characters)
	char byte_count[3];

	// Sending command buffer
	char command[20];
	int i;

	// Check whether the reply from the module informs that a message was received
	if (strstr(data_r, "+IPD") != NULL)
	{
		// move to the beginning of message length info
		i = 9;

		// extract message length info
		while(data_r[i] != ':')
		{
			byte_count[i - 9] = data_r[i];
			i++;
		}
		byte_count[i - 9] = '\0';

		// move to the first character of the message
		i++;

		// extract the message
		while(data_r[i] != 0)
		{
			message[i - 11] = data_r[i];
			i++;
		}

		// close the message
		message[i++] = '\r';
		message[i++] = '\n';
		message[i] = '\0';

		// Format the sending data command
		sprintf(command, "AT+CIPSEND=0,%d\r\n", atoi(byte_count));

		// Send the command
		HAL_UART_Transmit(&huart6, (uint8_t*)(command), strlen(command), 10);

		// Set the ready flag
		ready_to_send = 1;
	}
}

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
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  // Enable the ESP8266
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int i;

  // Configure the ESP8266 module
  for (i = 1; i < COMMANDS - 2; i++)
  {
  	  // Send the configuration command
	  HAL_UART_Transmit(&huart6, (uint8_t*)(config[i]), strlen(config[i]), 10);

	  // Clear the receive buffer
	  clr_buffer(data_r, 1000);

	  // Receive the response
	  HAL_UART_Receive(&huart6, (uint8_t*)data_r, sizeof(data_r), 100);

	  // During restart sequence, wait 1 second for module to restart
	  if (i == 0)
	  {
		  HAL_Delay(1000);
	  }

	  // Pass the result to the overseeing PC
	  HAL_UART_Transmit(&huart3, (uint8_t*)data_r, sizeof(data_r), 10);
	  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n\n", sizeof("\r\n\n"), 10);
  }

  // Configure the connection (separated due to the processing time of the module)
  for (i = COMMANDS - 3; i < COMMANDS; i++)
  {
	  // Send the command
	  HAL_UART_Transmit(&huart6, (uint8_t*)(config[i]), strlen(config[i]), 10);

	  // Clear the receive buffer
	  clr_buffer(data_r, 1000);

	  // Receive the response
	  HAL_UART_Receive(&huart6, (uint8_t*)data_r, sizeof(data_r), 100);

	  // Wait for the command to be processed (mostly applicable to DHCP address receive)
	  while(is_busy())
	  {
		  HAL_Delay(1000);

		  // Re-transmit the configuration command
		  HAL_UART_Transmit(&huart6, (uint8_t*)(config[i]), strlen(config[i]), 10);

		  // Clear the receive buffer
		  clr_buffer(data_r, 1000);

		  // Receive the response
		  HAL_UART_Receive(&huart6, (uint8_t*)data_r, sizeof(data_r), 100);
	  }

	  // Pass the result to the overseeing PC
	  HAL_UART_Transmit(&huart3, (uint8_t*)data_r, sizeof(data_r), 10);
	  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n\n", sizeof("\r\n\n"), 10);
  }

  // clear the buffer
  clr_buffer(data_r, 1000);

  while (1)
  {
	  // If echo message is ready to be sent back
	  if (ready_to_send)
	  {
		  // Transmit the message
		  HAL_UART_Transmit(&huart6, (uint8_t*)message, sizeof(message), 10);

		  // Clear the flag
		  ready_to_send = 0;
	  }
	  // If there is a message to be processed
	  else if (data_r[0] != 0)
	  {
		  // Prepare data for echo message
		  prepare_echo();

		  // Send the received data to overseeing PC
		  HAL_UART_Transmit(&huart3, (uint8_t*)data_r, sizeof(data_r), 10);
		  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n\n", sizeof("\r\n\n"), 10);
	  }

	  // clear the buffer
	  clr_buffer(data_r, 1000);

	  // receive a message from the ESP8266 module
	  HAL_UART_Receive(&huart6, (uint8_t*)data_r, sizeof(data_r), 100);


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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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

#ifdef  USE_FULL_ASSERT
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

