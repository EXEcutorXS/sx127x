/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * work based on DORJI.COM sample code and
 * https://github.com/realspinner/SX127X_LoRa
 */

#ifndef __SX127X_H__
#define __SX127X_H__

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#define SX127X_MAX_PACKET	32


/********************LoRa mode***************************/
#define LR_RegFifo                                  0x00
// Common settings
#define LR_RegOpMode                                0x01
#define LR_RegFrMsb                                 0x06
#define LR_RegFrMid                                 0x07
#define LR_RegFrLsb                                 0x08
// Tx settings
#define LR_RegPaConfig                              0x09
#define LR_RegPaRamp                                0x0A
#define LR_RegOcp                                   0x0B
// Rx settings
#define LR_RegLna                                   0x0C
// LoRa registers
#define LR_RegFifoAddrPtr                           0x0D
#define LR_RegFifoTxBaseAddr                        0x0E
#define LR_RegFifoRxBaseAddr                        0x0F
#define LR_RegFifoRxCurrentaddr                     0x10
#define LR_RegIrqFlagsMask                          0x11
#define LR_RegIrqFlags                              0x12
#define LR_RegRxNbBytes                             0x13
#define LR_RegRxHeaderCntValueMsb                   0x14
#define LR_RegRxHeaderCntValueLsb                   0x15
#define LR_RegRxPacketCntValueMsb                   0x16
#define LR_RegRxPacketCntValueLsb                   0x17
#define LR_RegModemStat                             0x18
#define LR_RegPktSnrValue                           0x19
#define LR_RegPktRssiValue                          0x1A
#define LR_RegRssiValue                             0x1B
#define LR_RegHopChannel                            0x1C
#define LR_RegModemConfig1                          0x1D
#define LR_RegModemConfig2                          0x1E
#define LR_RegSymbTimeoutLsb                        0x1F
#define LR_RegPreambleMsb                           0x20
#define LR_RegPreambleLsb                           0x21
#define LR_RegPayloadLength                         0x22
#define LR_RegMaxPayloadLength                      0x23
#define LR_RegHopPeriod                             0x24
#define LR_RegFifoRxByteAddr                        0x25
#define LR_RegWideBandRSSI                          0x2C
#define LR_RegSyncWord								0x39
// I/O settings
#define REG_LR_DIOMAPPING1                          0x40
#define REG_LR_DIOMAPPING2                          0x41
// Version
#define REG_LR_VERSION                              0x42
// Additional settings
#define REG_LR_PLLHOP                               0x44
#define REG_LR_TCXO                                 0x4B
#define REG_LR_PADAC                                0x4D
#define REG_LR_FORMERTEMP                           0x5B
#define REG_LR_AGCREF                               0x61
#define REG_LR_AGCTHRESH1                           0x62
#define REG_LR_AGCTHRESH2                           0x63
#define REG_LR_AGCTHRESH3                           0x64

#define MODEM_STATUS_SIG_DET                        0x1
#define MODEM_STATUS_SIG_SYNC                       0x2
#define MODEM_STATUS_RXONGOING                      0x4
#define MODEM_STATUS_HEADER_VALID                   0x8
#define MODEM_STATUS_CLEAR                          0xA

#define IRQ_CAD 									0x1
#define	IRQ_FCC										0x2
#define	IRQ_CAD_DONE 								0x4
#define IRQ_TX_DONE									0x8
#define IRQ_VALID_HEADER							0x10
#define IRQ_CRC_ERROR								0x20
#define IRQ_RX_DONE									0x40
#define IRQ_RX_TIMEOUT								0x80

/**********************************************************
 **Parameter table define
 **********************************************************/
#define SX127X_FREQ_410MHZ 0x668011
#define SX127X_FREQ_433MHZ 0x6C4012
#define SX127X_FREQ_868MHZ 0xd90024
#define SX127X_FREQ_DEFAULT 0xD94024 //869 MHz
#define SX127X_FREQ_820MHZ 0xCD0022
#define SX127X_FREQ_1020MHZ 0xFF002A
#define SX127X_FREQ_525MHZ 0x834016
#define SX127X_FREQ_1MHZ   0x4000

#define SX127X_POWER_20DBM		20
#define SX127X_POWER_17DBM		17
#define SX127X_POWER_14DBM		14
#define SX127X_POWER_11DBM		11
#define SX127X_POWER_10DBM		10

#define SX127X_LORA_SF_6		6
#define SX127X_LORA_SF_7		7
#define SX127X_LORA_SF_8		8
#define SX127X_LORA_SF_9		9
#define SX127X_LORA_SF_10		10
#define SX127X_LORA_SF_11	    11
#define SX127X_LORA_SF_12		12

#define	SX127X_LORA_BW_7_8KHZ		0
#define	SX127X_LORA_BW_10_4KHZ		1
#define	SX127X_LORA_BW_15_6KHZ		2
#define	SX127X_LORA_BW_20_8KHZ		3
#define	SX127X_LORA_BW_31_2KHZ		4
#define	SX127X_LORA_BW_41_7KHZ		5
#define	SX127X_LORA_BW_62_5KHZ		6
#define	SX127X_LORA_BW_125KHZ		7
#define	SX127X_LORA_BW_250KHZ		8
#define	SX127X_LORA_BW_500KHZ		9

#define SX127X_CR_4_5               1
#define SX127X_CR_4_6               2
#define SX127X_CR_4_7               3
#define SX127X_CR_4_8               4


typedef enum SX127X_STATUS {
	UNINITIALISED, SLEEP, STANDBY, TX, RX,
} SX127X_Status_t;

typedef struct {
	int pin;
	void * port;
} SX127X_dio_t;

typedef struct {
	//settings part
	uint32_t frequency;
	uint8_t power;
	uint8_t sf;
	uint8_t bw;
    uint8_t cr;
	uint8_t len;
    uint8_t crcEnable;
    uint8_t implicitHeader;
    uint16_t preamble;
    uint8_t syncWord;

    bool alwaysRX;    //По окончанию передачи модуль не засыпает и начинает приём

	SX127X_Status_t status;
	bool TXrequest;  //Запрос отправки сообщения
	uint32_t lastTransTick; //Время последней передачи
	uint32_t lastSignalTick;
    bool signalDetected;
	uint8_t rxBuf[32]; //Буфер в который будет перемещено входящее сообщение
	uint8_t txBuf[32]; //Буфер из которого будет отправлено сообщение
	uint8_t readBytes; // При приёме сообщения здесь будет записана его длина
	uint8_t irq;
	uint8_t modemStatus;
	bool badCrc;
	bool connected;

	//Hardware part

	SX127X_dio_t reset;
	SX127X_dio_t nss;
	SPI_HandleTypeDef* spi;

	uint8_t revision;
	uint32_t watchdogTick;
	uint16_t wdCounter;

} SX127X_t;

__weak void SX127X_SetNSS(SX127X_t * module, GPIO_PinState state);
__weak void SX127X_Reset(SX127X_t * module);
__weak void SX127X_SPICommand(SX127X_t * module, uint8_t cmd);
__weak uint8_t SX127X_SPIReadByte(SX127X_t * module);
__weak int SX127X_GetDIO0(SX127X_t * module);

uint8_t SX127X_SPIRead(SX127X_t * module, uint8_t addr);
void SX127X_SPIWrite(SX127X_t * module, uint8_t addr, uint8_t cmd);
void SX127X_SPIBurstRead(SX127X_t * module, uint8_t addr, uint8_t *rxBuf,
		uint8_t length);
void SX127X_SPIBurstWrite(SX127X_t * module, uint8_t addr, uint8_t *txBuf,
		uint8_t length);
void SX127X_standby(SX127X_t * module);
void SX127X_sleep(SX127X_t * module);


void SX127X_clearIrq(SX127X_t * module);
int SX127X_startRx(SX127X_t * module, uint32_t timeout);
uint8_t SX127X_receive(SX127X_t * module);
void SX127X_readStatus(SX127X_t * module);
void SX127X_readIrq(SX127X_t * module);
void SX127X_delayMicro(uint32_t micros);
void SX127X_defaultConfig(SX127X_t * module);

void SX127X_config(SX127X_t * module);
int16_t SX127X_RSSI(SX127X_t * module);
int16_t SX127X_RSSI_Pack(SX127X_t * module);
uint8_t SX127X_SNR(SX127X_t * module);
int8_t SX127X_readTemp(SX127X_t * module);
void SX127X_readAllRegisters(SX127X_t* module,uint8_t* buf);
void SX127X_Handler(SX127X_t* module);
void SX127X_transmit_it(SX127X_t* module);
void SX127X_PortConfig(SX127X_t * module, SX127X_dio_t reset, SX127X_dio_t nss, SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef SX127X_transmitAsync(SX127X_t* module, uint8_t lenght);
uint8_t SX127X_getRandom(SX127X_t* module);
void SX127X_init(SX127X_t* module);
//Функции для работы с модулем без задержек
//В основном цикле должна выхываться функция SX127X_Routine
//Для передачи массива необходимо записать его module->txBuf, длину в module->len
//И вызвать SX127X_requestTransmission
//Отслеживать приходящие сообщения можно по переменной module->readBytes, если она не равна нулю
//Значит в массив по адреу module->rxBuf был записан массив длиной readBytes.

#endif

