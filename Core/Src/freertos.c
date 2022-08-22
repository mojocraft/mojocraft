/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "usart.h"
#include "queue.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* USER CODE BEGIN Variables */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = 
{
  .name = "ledTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myMutex01 */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void ledTaskFunction(void * arg);
void myTask03(void *arg);
TaskHandle_t task03Handle;
QueueHandle_t myQueueHandle;
typedef struct myMessage
{
  uint8_t messageID;
  char data[40];
}myMessage_or;
#define QUEUELENTH 5
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of myMutex01 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  myQueueHandle = xQueueCreate(5, sizeof(myMessage_or));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  //xTaskCreate(myTask03, "task 03", 128 * 4, (void *)myQueueHandle, osPriorityNormal, &task03Handle);
  
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ledTaskHandle = osThreadNew(ledTaskFunction, NULL, &ledTask_attributes);
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  
  if(myQueueHandle == NULL)
    printf("myQueue create faild.\r\n");
  else
    printf("myQueue create seccess.\r\n");
  printf("end of the initation.\r\n");
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  struct myMessage queueMessage1 = {.messageID = 1, .data = "Hello world!\r\n"};
  struct myMessage queueMessage2 = {.messageID = 2, .data = "Nothing in the world can take12345678\r\n"};
  /* Infinite loop */
  for(;;)
  {
    if(xQueueSend(myQueueHandle, &queueMessage1, 100) == pdTRUE)
      ;
    else
      printf("Queue is full.\r\n");
    if(xQueueSend(myQueueHandle, &queueMessage2, 100) == pdTRUE)
      ;
    else
      printf("Queue is full.\r\n");
    if(xQueueSend(myQueueHandle, " can you do this\r\n", 100) == pdTRUE)
      ;
    else
      printf("Queue is full.\r\n");
    if(xQueueSend(myQueueHandle, " message 3\r\n", 100) == pdTRUE)
      ;
    else
      printf("Queue is full.\r\n");
    if(xQueueSend(myQueueHandle, " Message 4\r\n", 100) == pdTRUE)
      ;
    else
      printf("Queue is full.\r\n");
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void ledTaskFunction(void * arg)
{
  struct myMessage messageVector;
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    if(xQueueReceive(myQueueHandle, &messageVector, portMAX_DELAY) == pdTRUE)
      {
        printf("%s",messageVector.data);
      }
    else
      printf("receive faild.\r\n"); 
  }
}

/* USER CODE END Application */

