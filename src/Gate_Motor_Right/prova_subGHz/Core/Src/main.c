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
#include "radio_gpio.h"
#include "SPIRIT_General.h"
#include "SPIRIT_Radio.h"
#include "SPIRIT_Commands.h"
#include "SPIRIT_Qi.h"
#include "SPIRIT_Management.h"
#include "radio_spi.h"
#include "p2p_lib.h"

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
SPI_HandleTypeDef hspi1;

PCD_HandleTypeDef hpcd_USB_FS;


#define TX_BUFFER_SIZE   20
#define RX_BUFFER_SIZE   96

/* USER CODE BEGIN PV */
uint8_t TxLength = TX_BUFFER_SIZE;
uint8_t RxLength = RX_BUFFER_SIZE;
uint8_t aTransmitBuffer[TX_BUFFER_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
uint8_t aReceiveBuffer[RX_BUFFER_SIZE] = {0x00};

//variabili usate per la logica del cancello (servomotore)
typedef enum {
    CHIUSO,
    IN_APERTURA,
    APERTO,
    IN_CHIUSURA,
    FERMO,
    OSTACOLO
} StatoCancello;

StatoCancello stato_attuale = CHIUSO;
StatoCancello stato_precedente = CHIUSO;

uint8_t flag_cancello = 0;	//serve per segnalare che ho ricevuto effettivamente un nuovo pacchetto (altrimenti cancello fermo)
uint8_t comando_tel = 0;  	//serve per muovere il cancello
uint8_t fotoc = 0;        	//serve per la logica della fotocellula
uint8_t messaggio_scartato = 0;

uint16_t pwm_attuale = 1000;
uint16_t pwm_bersaglio = 1000;

//variabili usate per il controllo degli ultrasuoni
uint32_t ultimo_trigger = 0;
uint8_t turno_sensore = 1;


volatile uint16_t tempo_ritorno_eco = 0;

volatile uint8_t eco_ricevuto_sx = 0;
volatile uint8_t eco_ricevuto_dx = 0;

uint8_t contatore_ostacolo_sx = 0;
uint8_t contatore_ostacolo_dx = 0;

uint8_t fotoc_sx = 0;
uint8_t fotoc_dx = 0;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

volatile uint8_t flag_radio_irq = 0; // La bandierina per il modulo radio

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Muovi_Cancello();
static void Fotocellula();
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
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
	MX_SPI1_Init();
	//MX_USB_PCD_Init();

	/* USER CODE BEGIN 2 */

	// Settiamo TIM2 per far muovere il mio Servomotore
	MX_TIM2_Init();
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	// Settiamo TIM3 per usare gli ultrasuoni
	MX_TIM3_Init();
	HAL_TIM_Base_Start(&htim3);

	// Settiamo TIM4 per il timeout del cancello quanto è aperto
	MX_TIM4_Init();


    // Forza CSN alto (radio deselezionata) prima di iniziare
    HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);

    RadioExitShutdown();
    HAL_Delay(10);
    SpiritRadioSetXtalFrequency(50000000);


    // Usiamo il nome aggiornato della funzione
    uint8_t version = SpiritGeneralGetSpiritVersion();

    // 4. Verifichiamo. Se la versione è un numero valido (es. 0x30), il LED Verde si accende
    if (version != 0x00 && version != 0xFF)
    {
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
        Error_Handler();
    }

	SpiritCmdStrobeSres();
	HAL_Delay(10);
	SpiritCmdStrobeReady();
	HAL_Delay(5);


	P2P_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  	if (messaggio_scartato == 1){

	  		messaggio_scartato = 0;

	  		for(int i = 0; i < 6; i++) {
	  			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
	  			HAL_Delay(100);
	  		}

	  	}

		// 2. Facciamo girare la logica P2P
		P2P_Process(aTransmitBuffer, TxLength, aReceiveBuffer, RxLength);

		if (flag_cancello == 1){

			flag_cancello = 0;
			comando_tel = 1;

		}

		if ( (stato_attuale == IN_APERTURA) || (stato_attuale == IN_CHIUSURA) || (stato_attuale == OSTACOLO)){
			Fotocellula();
		}

		Muovi_Cancello();

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SDN_GPIO_Port, SDN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = DRDY_Pin|MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
                           LD7_Pin LD9_Pin LD10_Pin LD8_Pin
                           LD6_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SDN_Pin */
  GPIO_InitStruct.Pin = SDN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SDN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CSN_Pin */
  GPIO_InitStruct.Pin = CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CSN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C1_SDA_Pin */
  GPIO_InitStruct.Pin = I2C1_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
   HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /*Configure GPIO pin : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TRIG_Pin */
  GPIO_InitStruct.Pin = TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TRIG_Pin */
  GPIO_InitStruct.Pin = TRIG_Pin2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void EXTI9_5_IRQHandler(void)
{
    // Passa la palla alla libreria HAL per il Pin 7,
    // lei pulirà il registro hardware ed eviterà il blocco.
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

void EXTI15_10_IRQHandler(void)
{
    // Questo è il ponte per il Sensore ad Ultrasuoni (Pin 11)
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);

    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);

}

// Questa funzione scatta da sola quando un pin EXTI viene premuto
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if(GPIO_Pin == GPIO_PIN_7)
	{
		flag_radio_irq = 1; // Alza SOLO la bandierina software, NON fare altro qui!
	}

	// 2. SE PREMI IL PULSANTE BLU DELLA SCHEDA (Pin PA0 - Fronte di Salita)
	if(GPIO_Pin == GPIO_PIN_0)
	{
		// Chiama la funzione della libreria che forza il cambio di stato!
		Set_KeyStatus(SET);
	}

	if (GPIO_Pin == GPIO_PIN_11) {

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) {
			__HAL_TIM_SET_COUNTER(&htim3, 0);
		}
		else {
			// FRONTE DI DISCESA
			//calcoliamo la "distanza" direttamente vedendo a quanto è arrivato il timer
			tempo_ritorno_eco = __HAL_TIM_GET_COUNTER(&htim3);
			eco_ricevuto_sx = 1;
		}
	}

	if (GPIO_Pin == GPIO_PIN_12) {
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
			__HAL_TIM_SET_COUNTER(&htim3, 0);
		} else {

			tempo_ritorno_eco = __HAL_TIM_GET_COUNTER(&htim3);

			if (tempo_ritorno_eco > 100) {
				eco_ricevuto_dx = 1;
			}
		}
	}
}

void TIM4_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim4);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {

	//scaduto il Time-out, prima stoppo il conteggio, dopo resetto il contatore
    HAL_TIM_Base_Stop_IT(&htim4);
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    if (stato_attuale == APERTO){
    	stato_attuale = IN_CHIUSURA;
    }
  }

}

static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */
	__HAL_RCC_TIM2_CLK_ENABLE();
	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 71;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 19999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
	Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
	Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
	{
	Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
	Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 1000;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
	Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);

}

static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */
  __HAL_RCC_TIM3_CLK_ENABLE();
  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

static void MX_TIM4_Init(void)
{
  /* Abilita il clock per il TIM4 */
  __HAL_RCC_TIM4_CLK_ENABLE();

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;

  /* Imposta Prescaler e Period per ottenere 30 secondi a 72MHz */
  htim4.Init.Prescaler = 23999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 59999;

  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
    /* USER CODE BEGIN TIM3_Init 2 */

    /* USER CODE END TIM3_Init 2 */
  }
}

void Fotocellula(){

	if (HAL_GetTick() - ultimo_trigger >= 50) {
		ultimo_trigger = HAL_GetTick();

		if (turno_sensore == 0) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
			for(volatile int i = 0; i < 100; i++);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
			turno_sensore = 1;
		} else {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_SET);
			for(volatile int i = 0; i < 100; i++);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_RESET);
			turno_sensore = 0;
		}
	}

	// Controllo Sinistro
	if (eco_ricevuto_sx == 1) {
		eco_ricevuto_sx = 0;
		if (tempo_ritorno_eco <= 2000) {
			contatore_ostacolo_sx++;
		} else {
			contatore_ostacolo_sx = 0;
		}

		if (contatore_ostacolo_sx >= 3) fotoc_sx = 1;
		else fotoc_sx = 0;
	}

	// Controllo Destro
	if (eco_ricevuto_dx == 1) {
		eco_ricevuto_dx = 0;
		if (tempo_ritorno_eco <= 2000) {
			contatore_ostacolo_dx++;
		} else {
			contatore_ostacolo_dx = 0;
		}

		if (contatore_ostacolo_dx >= 3) fotoc_dx = 1;
		else fotoc_dx = 0;
	}


	if ((fotoc_dx == 1) || (fotoc_sx == 1)) {
		fotoc = 1;
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
	} else {
		fotoc = 0;
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
	}

}



void Muovi_Cancello(){

	switch (stato_attuale) {
			case CHIUSO:
				pwm_bersaglio = 1000;
				if (comando_tel) {
					stato_attuale = IN_APERTURA;
					comando_tel = 0;
				}
				break;

			case IN_APERTURA:
				pwm_bersaglio = 1800;
				if (fotoc) {
					stato_attuale = OSTACOLO;
				}
				else if (pwm_attuale == 1800) {
					stato_attuale = APERTO;

					//inizia il conteggio per il time-out in chiusura (contatore resettato per sicurezza)
					__HAL_TIM_SET_COUNTER(&htim4, 0);
					__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE); //abbassa la bandierina che si alza quando faccio la Init
					HAL_TIM_Base_Start_IT(&htim4);
				}

				else if (comando_tel) {
					stato_precedente = IN_APERTURA;
					stato_attuale = FERMO;
					comando_tel = 0;
				}
				break;

			case APERTO:
				pwm_bersaglio = 1800;
				if (comando_tel) {
					stato_attuale = IN_CHIUSURA;
					comando_tel = 0;

					//se sto in apertura e mi arriva un comando, interrompi il timer
					__HAL_TIM_SET_COUNTER(&htim4, 0);
					HAL_TIM_Base_Stop_IT(&htim4);
				}
				break;

			case IN_CHIUSURA:
				pwm_bersaglio = 1000;
				if (fotoc) {
					stato_attuale = OSTACOLO;
				}
				else if (pwm_attuale == 1000) {
					stato_attuale = CHIUSO;
				}
				//comando_tel indica un messaggio inviato durante l'apertura che porta il cancello a fermarsi
				//utilizzato principalmente per prove, si può eliminare
				else if (comando_tel) {
					stato_precedente = IN_CHIUSURA;
					stato_attuale = FERMO;
					comando_tel = 0;
				}
				break;

			case FERMO:
				// Il motore si ferma dove si trova
				pwm_bersaglio = pwm_attuale;
				if (comando_tel) {
					// Se ero in chiusura, chiude. Se ero in apertura, apre.
					stato_attuale = (stato_precedente == IN_CHIUSURA) ? IN_APERTURA : IN_CHIUSURA;
					comando_tel = 0;
				}
				break;

			case OSTACOLO:
				pwm_bersaglio = pwm_attuale; // Stop immediato
				// In un cancello vero, qui si aspetta che fotoc torni a 0
				if (!fotoc && comando_tel) {
					HAL_Delay(100);
					stato_attuale = IN_APERTURA; // Per sicurezza riapre sempre
					comando_tel = 0;
				}
				break;
		}

		// --- MOVIMENTO FLUIDO ---
		static uint32_t ultimo_passo = 0;
		if (pwm_attuale != pwm_bersaglio && (HAL_GetTick() - ultimo_passo > 20)) {
			if (pwm_attuale < pwm_bersaglio) pwm_attuale += 5;
			else if (pwm_attuale > pwm_bersaglio) pwm_attuale -= 5;

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_attuale);
			ultimo_passo = HAL_GetTick();
		}

}

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
