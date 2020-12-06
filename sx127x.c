/**
*Author Milke Andrey a.k.a EXEcutorXS <streamline88@inbox.ru>
* based on code Wojciech Domski <Wojciech.Domski@gmail.com>
*/
#include "main.h"

extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi1;
extern nodeSettings_t settings;
uint32_t systick;

//////////////////////////////////
// logic
//////////////////////////////////


__weak void SX1276_SetNSS(SX1276_t * module, GPIO_PinState state) {
	HAL_GPIO_WritePin(module->nss.port, module->nss.pin, state);
}

__weak void SX1276_Reset(SX1276_t * module) {
	SX1276_SetNSS(module, 1);
	HAL_GPIO_WritePin(module->reset.port, module->reset.pin, GPIO_PIN_RESET);

	HAL_Delay(1);

	HAL_GPIO_WritePin(module->reset.port, module->reset.pin, GPIO_PIN_SET);

	HAL_Delay(6);
}

__weak void SX1276_SPICommand(SX1276_t * module, uint8_t cmd) {
	SX1276_SetNSS(module, 0);
	HAL_SPI_Transmit(module->spi, &cmd, 1, 1000);
	while (HAL_SPI_GetState(module->spi) != HAL_SPI_STATE_READY)
	;
}

__weak uint8_t SX1276_SPIReadByte(SX1276_t * module) {
	uint8_t txByte = 0x00;
	uint8_t rxByte = 0x00;

	SX1276_SetNSS(module, 0);
	HAL_SPI_TransmitReceive(module->spi, &txByte, &rxByte, 1, 1000);
	while (HAL_SPI_GetState(module->spi) != HAL_SPI_STATE_READY)

	;
	return rxByte;
}


//////////////////////////////////
// logic
//////////////////////////////////

uint8_t SX1276_SPIRead(SX1276_t * module, uint8_t addr) {
	uint8_t tmp;
	SX1276_SPICommand(module, addr);
	tmp = SX1276_SPIReadByte(module);
	SX1276_SetNSS(module, 1);
	return tmp;
}

void SX1276_SPIWrite(SX1276_t * module, uint8_t addr, uint8_t cmd) {
	SX1276_SetNSS(module, 0);
	SX1276_SPICommand(module, addr | 0x80);
	SX1276_SPICommand(module, cmd);
	SX1276_SetNSS(module, 1);
}

void SX1276_SPIBurstRead(SX1276_t * module, uint8_t addr, uint8_t* rxBuf,
uint8_t length) {
	uint8_t i;
	if (length <= 1) {
		return;
	} else {
		SX1276_SetNSS(module, 0);
		SX1276_SPICommand(module, addr);
		for (i = 0; i < length; i++) {
			*(rxBuf + i) = SX1276_SPIReadByte(module);
		}
		SX1276_SetNSS(module, 1);
	}
}

void SX1276_SPIBurstWrite(SX1276_t * module, uint8_t addr, uint8_t* txBuf,
uint8_t length) {
	unsigned char i;
	if (length <= 1) {
		return;
	} else {
		SX1276_SetNSS(module, 0);
		SX1276_SPICommand(module, addr | 0x80);
		for (i = 0; i < length; i++) {
			SX1276_SPICommand(module, *(txBuf + i));
		}
		SX1276_SetNSS(module, 1);
	}
}


void SX1276_config(SX1276_t * module){
	SX1276_sleep(module); //Change modem mode Must in Sleep mode

	SX1276_SPIWrite(module, LR_RegFrMsb, (module->frequency>>16)&0xFF);
	SX1276_SPIWrite(module, LR_RegFrMid, (module->frequency>> 8)&0xFF);
	SX1276_SPIWrite(module, LR_RegFrLsb, (module->frequency    )&0xFF);

	//setting base parameter
	SX1276_SPIWrite(module, LR_RegPaConfig, 0xF0 | (module->power-5)); //Setting output power parameter

	SX1276_SPIWrite(module, LR_RegOcp, 0x32);	//OCP to 150 mA
	SX1276_SPIWrite(module, LR_RegLna, 0x23);	//LNA to full gain
	SX1276_SPIWrite(module,	LR_RegModemConfig1,
	((module->bw << 4) +         //Bandwidth
	( module->cr << 1))+         //Coding rate
	(module->implicitHeader&1)); //Implicit header flag
	SX1276_SPIWrite(module,	LR_RegModemConfig2,
	((module->sf << 4) +         //Spreading factor
	(module->crcEnable<<2) +     //CRC enable flag
	0x3));                       //RX Time-Out MSB (always max)

	SX1276_SPIWrite(module, LR_RegSymbTimeoutLsb, 0xFF); //RX symb Timeout = 0x3FF(Max)
	SX1276_SPIWrite(module, LR_RegPreambleMsb, (module->preamble>>8)& 0xFF); //RegPreambleMsb
	SX1276_SPIWrite(module, LR_RegPreambleLsb, module->preamble & 0xFF); //RegPreambleLsb
	SX1276_SPIWrite(module, LR_RegPayloadLength, module->len); //Payload lenght
	SX1276_SPIWrite(module, LR_RegSyncWord, module->syncWord);  //Sync word
	module->readBytes = 0;

	/*** Sensitivity correction for 500kHz BW (see Errata) ***/
	module->revision=SX1276_SPIRead(module, REG_LR_VERSION);
	if (module->bw==SX1276_LORA_BW_500KHZ && module->revision==0x12)
	{
		if (module->frequency > SX1276_FREQ_820MHZ &&
			module->frequency < SX1276_FREQ_1020MHZ)
		{
			SX1276_SPIWrite(module, 0x36, 0x2);
			SX1276_SPIWrite(module, 0x3A, 0x64);

		}
		if (module->frequency > SX1276_FREQ_410MHZ &&
			module->frequency < SX1276_FREQ_525MHZ)
		{
			SX1276_SPIWrite(module, 0x36, 0x3);
			SX1276_SPIWrite(module, 0x3A, 0x7F);

		}

	}
	SX1276_standby(module); //Entry standby mode
}

void SX1276_defaultConfig(SX1276_t * module)
{
module->bw=SX1276_LORA_BW_125KHZ;
module->cr=SX1276_CR_4_8;
module->crcEnable=1;
module->frequency=SX1276_FREQ_DEFAULT;
module->implicitHeader=1;
module->len=3;
module->power=SX1276_POWER_20DBM;
module->preamble=5;
module->sf=SX1276_LORA_SF_12;
module->spi=&hspi1;
module->syncWord=0x1;
module->alwaysRX=1;

}

void SX1276_standby(SX1276_t * module) {
	if (module->frequency<SX1276_FREQ_525MHZ)
	SX1276_SPIWrite(module, LR_RegOpMode, 0x89);
	else
	SX1276_SPIWrite(module, LR_RegOpMode, 0x81);
	module->status = STANDBY;
}

void SX1276_sleep(SX1276_t * module) {
	if (module->frequency<SX1276_FREQ_525MHZ)
	SX1276_SPIWrite(module, LR_RegOpMode, 0x88);
	else
	SX1276_SPIWrite(module, LR_RegOpMode, 0x80);
	module->status = SLEEP;
}

void SX1276_clearIrq(SX1276_t * module) {
	SX1276_SPIWrite(module, LR_RegIrqFlags, 0xFF);
}

int SX1276_startRx(SX1276_t * module, uint32_t timeout) {
	uint8_t addr;
	SX1276_config(module);		//Setting base parameter
	SX1276_SPIWrite(module, REG_LR_PADAC, 0x84);	//Normal and RX
	SX1276_SPIWrite(module, LR_RegHopPeriod, 0x00);	//No FHSS
	SX1276_clearIrq(module);
	SX1276_SPIWrite(module, LR_RegPayloadLength, module->len);
	addr = SX1276_SPIRead(module, LR_RegFifoRxBaseAddr); //Read RxBaseAddr
	SX1276_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr->FiFoAddrPtr
	if (module->frequency<SX1276_FREQ_525MHZ)
	SX1276_SPIWrite(module, LR_RegOpMode, 0x8d);	//Cont RX Mode + LF
	else
	SX1276_SPIWrite(module, LR_RegOpMode,0x85);	    //Cont RX Mode + HF
	module->readBytes = 0;

	while (1) {
		if ((SX1276_SPIRead(module, LR_RegModemStat) & 0x04) == 0x04) {	//Rx-on going RegModemStat
			module->status = RX;
			return 1;}

		if (--timeout == 0) {
			SX1276_Reset(module);
			SX1276_config(module);
			return 0;}

		HAL_Delay(1);}
}

int SX1276_startTx(SX1276_t * module, uint8_t len, uint32_t timeout) {
	uint8_t addr;
	uint8_t temp;

	module->len = len;

	SX1276_config(module); //setting base parameter
	SX1276_SPIWrite(module, REG_LR_PADAC, 0x87);	//20dBm
	SX1276_SPIWrite(module, LR_RegHopPeriod, 0x00);
	SX1276_clearIrq(module);
	SX1276_SPIWrite(module, LR_RegPayloadLength, len);
	addr = SX1276_SPIRead(module, LR_RegFifoTxBaseAddr);
	SX1276_SPIWrite(module, LR_RegFifoAddrPtr, addr);
	HAL_Delay(5);
	while (1) {
		temp = SX1276_SPIRead(module, LR_RegPayloadLength);
		if (temp == len) {
			module->status = TX;
			return 1;
		}

		if (--timeout == 0) {
			SX1276_Reset(module);
			SX1276_config(module);
			return 0;
		}
		HAL_Delay(1);
	}
}


int SX1276_transmit(SX1276_t * module, uint8_t* txBuffer, uint8_t length,
uint32_t timeout) {
	module->status=TX;
	SX1276_SPIBurstWrite(module, 0x00, txBuffer, length);

	if (module->frequency<SX1276_FREQ_525MHZ)
	SX1276_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode LF
	else
	SX1276_SPIWrite(module, LR_RegOpMode, 0x83);	//Tx Mode HF

	while (1) {
		SX1276_readIrq(module);
		if (module->irq&IRQ_TX_DONE) {
			SX1276_clearIrq(module);
			SX1276_standby(module);
			return true;
		}

		if (--timeout == 0) {
			SX1276_Reset(module);
			SX1276_config(module);
			return false;
		}
		HAL_Delay(1);
	}
}


uint8_t SX1276_receive(SX1276_t * module) {
	unsigned char addr;
	unsigned char packet_size;
	memset(module->rxBuf, 0x00, SX1276_MAX_PACKET);

	addr = SX1276_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
	SX1276_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr
	if (module->sf == SX1276_LORA_SF_6)
	packet_size = module->len;
	else
	packet_size = SX1276_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
	SX1276_SPIBurstRead(module, 0x00, module->rxBuf, packet_size);
	module->readBytes = packet_size;
	return module->readBytes;
}



void SX1276_transmit_it(SX1276_t* module) {
	uint8_t addr;
	SX1276_config(module); //setting base parameter
	module->status=TX;
	SX1276_SPIWrite(module, REG_LR_PADAC, 0x87);	//Tx for 20dBm
	SX1276_SPIWrite(module, LR_RegHopPeriod, 0x00); //RegHopPeriod NO FHSS
	SX1276_clearIrq(module);
	SX1276_SPIWrite(module, LR_RegPayloadLength, module->len); //RegPayloadLength 21byte
	addr = SX1276_SPIRead(module, LR_RegFifoTxBaseAddr); //RegFiFoTxBaseAddr
	SX1276_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RegFifoAddrPtr
	SX1276_SPIBurstWrite(module, 0x00, module->txBuf, module->len);
	module->lastTransTick=HAL_GetTick();
	if (module->frequency<SX1276_FREQ_525MHZ)
	SX1276_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode LF
	else
	SX1276_SPIWrite(module, LR_RegOpMode, 0x83);	//Tx Mode HF
	module->TXrequest=0;


}

HAL_StatusTypeDef SX1276_requestTransmission(SX1276_t* module,uint8_t lenght)
{
if (module->TXrequest==0 &&
	module->status!=TX)
{
	module->len=lenght;
	module->TXrequest=1;

	return HAL_OK;
}
else
	return HAL_ERROR;
}


void SX1276_activity(SX1276_t* module)
{
    SX1276_readStatus(module);
    SX1276_readIrq(module);

	if (module->status==UNINITIALISED)
		SX1276_config(module);

	if ((module->status==SLEEP || module->status==STANDBY) && module->alwaysRX)
		SX1276_startRx(module, 100);

	if (module->TXrequest && (module->modemStatus&MODEM_STATUS_SIG_DET)==0)
		SX1276_transmit_it(module);

	SX1276_readIrq(module);
		if (module->irq & IRQ_TX_DONE)
		{
			SX1276_clearIrq(module);
			SX1276_startRx(module, 100);
		}

		if (module->irq & IRQ_RX_DONE)
		{
			module->badCrc=(module->irq&IRQ_CRC_ERROR)>>5;
			SX1276_receive(module);
			SX1276_clearIrq(module);
		}

}

int16_t SX1276_RSSI(SX1276_t * module) {
	if (module->frequency<SX1276_FREQ_525MHZ)
	return (int16_t)SX1276_SPIRead(module, LR_RegRssiValue)-164;
	else
	return (int16_t)SX1276_SPIRead(module, LR_RegRssiValue)-157;
}

int16_t SX1276_RSSI_Pack(SX1276_t * module) {
	if (module->frequency<SX1276_FREQ_525MHZ)
	return (int16_t)SX1276_SPIRead(module, LR_RegPktRssiValue)-164;
	else
	return (int16_t)SX1276_SPIRead(module, LR_RegPktRssiValue)-157;
}

uint8_t SX1276_SNR(SX1276_t * module){
	return SX1276_SPIRead(module, LR_RegPktSnrValue)/4;
}

int8_t SX1276_readTemp(SX1276_t * module)
{
	int8_t temp;
	uint8_t ret;
	SX1276_sleep(module);
	SX1276_SPIWrite(module, LR_RegOpMode, 0x0); //sleep FSK mode
	SX1276_SPIWrite(module, LR_RegOpMode, 0x5); //FS RX mode
	temp = SX1276_SPIRead(module, 0x3B);
	SX1276_SPIWrite(module, 0x3B, temp&0xFE);
	SX1276_delayMicro(500);
	ret = SX1276_SPIRead(module, 0x3C);
	SX1276_SPIWrite(module, 0x3B, temp|0x1);
	SX1276_SPIWrite(module, LR_RegOpMode, 0x0); //sleep FSK mode
	SX1276_sleep(module);
	if (ret&0x80)
	return 255-ret;
	else
	return ret*-1;
}

void SX1276_readStatus(SX1276_t * module)
{
module->modemStatus = SX1276_SPIRead(module, LR_RegModemStat);
if (module->modemStatus & MODEM_STATUS_SIG_DET)
{
	module->signalDetected=true;
	module->lastRX=HAL_GetTick();
}
else
	module->signalDetected=false;
}

void SX1276_readIrq(SX1276_t * module)
{
module->irq = SX1276_SPIRead(module, LR_RegIrqFlags);
}

void SX1276_delayMicro(uint32_t micros)
{
	micros *=SystemCoreClock / 10000000 ;
	while (micros--);
}

uint8_t SX1276_getRandom(SX1276_t* module)
{
return SX1276_SPIRead(module, LR_RegWideBandRSSI);
}

void SX1276_init(SX1276_t* module)
{
HAL_GPIO_WritePin(module->reset.port, module->reset.pin, GPIO_PIN_SET);
HAL_Delay(10);
module->revision=SX1276_SPIRead(module, REG_LR_VERSION);
module->revision=SX1276_SPIRead(module, REG_LR_VERSION);
}
