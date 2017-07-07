#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <misc.h>

// Botone interno
#define ROEN_BUTTON_GPIO        GPIOC
#define ROEN_BUTTON_PIN         GPIO_Pin_13
// Bottone esterno
#define EX_BUTTON_GPIO          GPIOB
#define EX_BUTTON_PIN           GPIO_Pin_10

int main(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);////Reset and clock control:abilita il clock di periferica GPIO; AFIO abilita i pin a funzionare in modalità altenative(USART ecc)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//Reset and clock control:abilita il clock della usart1

  GPIO_InitTypeDef GPIO_InitStructure; //struttura relativa alla routine di inizializzazione delle porte di i/o; contiene i campi relativi al funzionamento dei pin
  GPIO_StructInit(&GPIO_InitStructure);// inizializza la struttura

  // Led esterno
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  //GPIO_Pin è un vettore di bit che permette di configurare pin multipli associati ad un’unica porta in un solo step. 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;// Modalità output push pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;// velocità minima richiesta
  GPIO_Init(GPIOA, &GPIO_InitStructure);//inizializza la porta GPIOA con la struttura creata

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);

  // Initialize USART1_TX
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//modalità alternate function push-pull
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Initialize USART1_RX
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//modalità input floating
  GPIO_Init(GPIOA, &GPIO_InitStruct);
    // Inizializzazine USART
  USART_InitTypeDef USART_InitStructure;
  USART_StructInit(&USART_InitStructure);

  USART_InitStructure.USART_BaudRate = 19200;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);

  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  // Configure NVIC
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//gestore degli interrupt vettorizzati

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Configurazione bottone interno
  EXTI_InitTypeDef EXTI_InitStruct;
  GPIO_InitTypeDef GPIO_InitStructure2;
  GPIO_StructInit(&GPIO_InitStructure2);

  GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IPU;// input pull-up
  GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure2.GPIO_Pin = ROEN_BUTTON_PIN;
  GPIO_Init(ROEN_BUTTON_GPIO, &GPIO_InitStructure2);

  // Colleghiamo l'interrupt esterno al bottone USER
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);

  // Abilita l'intererupt
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  // Modalità interrupt
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  // Modalità trigger
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  // Collega l'interrupt al bottone
  EXTI_InitStruct.EXTI_Line = EXTI_Line13;
  EXTI_Init(&EXTI_InitStruct);

  // Configurazione bottone esterno
  EXTI_InitTypeDef EXTI_InitStruct2;
  GPIO_InitTypeDef GPIO_InitStructure3;
  GPIO_StructInit(&GPIO_InitStructure3);

  GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure3.GPIO_Pin = EX_BUTTON_PIN;
  GPIO_Init(EX_BUTTON_GPIO, &GPIO_InitStructure3);

  // Colleghiamo l'interrupt esterno al bottone esterno
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

  // Abilita l'intererupt
  EXTI_InitStruct2.EXTI_LineCmd = ENABLE;
  // Modalità interrupt
  EXTI_InitStruct2.EXTI_Mode = EXTI_Mode_Interrupt;
  // Modaitàà trigger
  EXTI_InitStruct2.EXTI_Trigger = EXTI_Trigger_Falling;
  // Collega l'interrupt al bottone
  EXTI_InitStruct2.EXTI_Line = EXTI_Line10;
  EXTI_Init(&EXTI_InitStruct2);

  // NIVC Interrupt controller per i pin da 10 a 15
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_Init(&NVIC_InitStruct);



  while (1) {

  }
}

// Interrupt collegato all'USART1
void USART1_IRQHandler(void) {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {//stato usart1: controlla se RX NOT EMPTY è diverso da 0(ho dati in ricezione)

    uint8_t data;

    data = USART_ReceiveData(USART1) & 0xff;
    if (data == '1') {
      GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET);
    }
    if (data == '2') {
      GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
    }
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }

  if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) { }
}

// Interrupt bottone interno
void EXTI15_10_IRQHandler(void) {

  // Bottone interno
  if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
    USART_SendData(USART1, '1');
    // Pulisce gli interrupt bit pendenti
    EXTI_ClearITPendingBit(EXTI_Line13);
  }

  // Bottone esterno
  if(EXTI_GetITStatus(EXTI_Line10) != RESET) {
    USART_SendData(USART1, '2');
    // Pulisce gli interrupt bit pendenti
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
}

/*
static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime){
    TimingDelay = nTime; while(TimingDelay != 0);
}

void SysTick_Handler(void){
    if (TimingDelay != 0x00)
    TimingDelay--;
}*/

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {//offre la possibilità di gestire le violazioni di assertion (una dichiarazione in cui ci si aspetta che il predicato sia sempre vero in un punto del codice)
  while (1);
}

#endif
