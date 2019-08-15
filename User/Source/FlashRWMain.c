#include "FlashRWMain.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "W25Q.h"
USB_OTG_CORE_HANDLE USB_OTG_dev;

void SPI1_Init(void)
{	 
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟
 
	//GPIOFA 5,6,7 初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PB3~5复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//PB3~5复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
    
    //GPIO_PinAFConfig(GPIOA,GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5, GPIO_AF_SPI1); //PA5复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6, GPIO_AF_SPI1); //PA6复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7, GPIO_AF_SPI1); //PA7复用为 SPI1
 
	//这里只针对SPI口初始化
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位SPI1
 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
	 
}   
//SPI1速度设置函数
//SPI速度=fAPB2/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256  
//fAPB2时钟一般为84Mhz：
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    SPI1->CR1&=0XFFC7;//位3-5清零，用来设置波特率
    SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI1速度 
    SPI_Cmd(SPI1, ENABLE); //使能SPI1
}

void RTCTest(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	PWR_BackupAccessCmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
	RCC_MCO1Config(RCC_MCO1Source_LSE, RCC_MCO1Div_1);
	while(1);
}



uint8_t flashrdata[4096];
uint8_t flashwdata[4096];

void flash_read(uint32_t offset, uint8_t * readdata, uint32_t blockcount)
{
    for(uint32_t address = 0; address < 4096 * blockcount; address += 4096)
    {
        w25q_rd_data(address + offset, 4096, readdata);
    }
}

void flash_erase(uint32_t offset, uint8_t * erasecount){
    uint32_t eraseblock = 
                    erasecount[0] << 24 |
                    erasecount[1] << 16 |
                    erasecount[2] <<  8 |
                    erasecount[3] <<  0;
    for(int i = 0; i < eraseblock; i++){
        w25q_erase_4k(offset + i * 4096);
    }
}

void flash_write(uint32_t offset, uint8_t * writedata, uint32_t blockcount)
{
    for(uint32_t address = 0; address < 4096 * blockcount; address += 4096)
    {
        w25q_wr_data(address + offset, 4096, writedata);
    }
}

uint8_t uart_send(uint8_t * buffer, uint32_t offset, uint32_t size){
    uint8_t sum = 0;
    uint8_t cbyte = 0;
    for(uint32_t i = 0; i < size; i++)
    {
        cbyte = *(buffer + offset + i);
        USART_SendData(USART2, cbyte);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        sum += cbyte;
        if(i == 2048){
            Delay(10);
        }
    }
    return sum;
}

uint8_t uart_recv(){
    while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(USART2);
}


uint8_t sendheader[9] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
void send_packet(uint8_t cmd, uint32_t para, uint8_t * payload, uint32_t offset, uint16_t size){
    sendheader[1] = cmd;
    sendheader[2] = (para & 0xFF000000) >> 24;
    sendheader[3] = (para & 0x00FF0000) >> 16;
    sendheader[4] = (para & 0x0000FF00) >> 8;
    sendheader[5] = (para & 0x000000FF) >> 0;
    sendheader[6] = (size & 0xff00) >> 8;
    sendheader[7] = (size & 0x00ff) >> 0;
    uart_send(sendheader, 0, 8);
    sendheader[8] = ~uart_send(payload, offset, size);
    uart_send(sendheader, 8, 1);
}

uint8_t recv_packet(uint8_t *cmd, uint32_t *para, uint16_t *size, uint8_t * payload) {
    *size = 0;
    uint32_t headsize = 8;
    uint32_t recvindex = 0;
    uint8_t recvbyte = 0;
    uint8_t sum = 0;
    while(true){
        recvbyte = uart_recv();
        if(recvindex == 0){
            if(recvbyte != 0xAA) continue;
        }
        else if(recvindex == 1){
            *cmd = recvbyte;
        }
        else if(recvindex < 6){
            *((uint8_t *)para + (6 - recvindex - 1)) = recvbyte;
        }
        else if(recvindex < 8){
            *((uint8_t *)size + (8 - recvindex - 1)) = recvbyte;
            if(*size > 4096) return false;
        }
        else if(recvindex < (headsize + *size)){
            *(payload + recvindex - 8) = recvbyte;
            sum += recvbyte;
        }
        else if(recvindex == (headsize + *size)){
            if(((~sum)&0xFF) == recvbyte){
                return true;
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
        recvindex++;
    }
    
}

int main(void)
{
	ClockInitialize();			// 时钟初始化
	DelayInitialize(1);			// 1ms Sys-Tick 初始化
	GPIOInitialize();			// GPIO 初始化
	USARTInitialize();			// 串口初始化
	printf("Start Flash Write Read!\n");
    SPI1_Init();
    SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
    printf("SPI Init!\n");
    w25q_init();
    
//   uint32_t readaddr = 0x01080000;
//    flash_read(readaddr, flashrdata, 1);
//    printf("AA");
//    printf("%08X", readaddr);
//    
//    uint8_t sum = 0;
//    for(int j = 0; j < 4096; j++){
//        printf("%02X", flashrdata[j]);
//        sum += flashrdata[j];
//    }
//    printf("%02X\n", sum);

	while(true)
    {
        uint8_t cmd = 0;
        uint32_t para = 0;
        uint16_t size = 0;
        if(!recv_packet(&cmd, &para, &size, flashwdata))continue;
        //printf("cmd: %02X, para: %08X, size: %d, data: \n", cmd, para, size);
        //for(int i = 0; i < size; i++){
        //    printf("%02X", flashwdata[i]);
        //    if((i + 1) % 16 == 0) printf("\n");
        //}
        switch(cmd){
            case 0x00:
                flash_read(para, flashrdata, 1);
                send_packet(cmd|0x80, para, flashrdata, 0, 4096);
                break;
            case 0x01:
                flash_write(para, flashwdata, 1);
                send_packet(cmd|0x80, para, NULL, 0, 0);
                break;
            case 0x02:
                flash_erase(para, flashwdata);
                send_packet(cmd|0x80, para, NULL, 0, 0);
                break;
        }
    }
}
