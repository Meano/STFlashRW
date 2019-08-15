/**
  * @file  w25q.c
  * @brief w25q flash驱动
	*        接口： 1个SPI + 1个GPIO 
  * @par   date        version    author    remarks
	*        2018-12-01  v4.0       esone      
  *
  */
	
/** 头文件包含区 ------------------------------------------------ */
#include "W25Q.h" 
#include "stdio.h"

/** 接口 -------------------------------------------------------- */		

#define W25Q_CS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4);
#define W25Q_CS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4);
#define W25Q_WP_HIGH()    
#define W25Q_RST_HIGH()   

/** 私有变量 -------------------------------------------------------- */
static uint8_t Is4ByteAddrMode = 0;


/** 内部函数 --------------------------------------------------- */
static void w25q_cmd(uint8_t* p_data, uint16_t size);
static void w25q_wr_cmd(uint8_t* p_data, uint16_t size);
static void w25q_wr_enable(void);
static void w25q_wr_disable(void);
static void w25q_busy_wait(void);
static uint8_t w25q_rd_ads(void);
static void w25q_set_4b(void);
static uint16_t w25q_id(void);


/** 私有宏(类型定义) -------------------------------------------- */ 
#define W25Q256                 0xEF18

#define W25Q_ID                 0x90    /**< 4: 制造商ID(0xEF)；5：设备ID */
#define W25Q_STATUS_1           0x05    /**< 读取状态寄存器(S7 -S0 ) */
#define W25Q_STATUS_3           0x15    /**< 读取状态寄存器(S23-S16) */
#define W25Q_WR_ENABLE          0x06    
#define W25Q_WR_DISABLE         0x04
#define W25Q_4K_ERASE           0x20    /**< 擦除4K空间 */
#define W25Q_64K_ERASE          0xD8    /**< 擦除64K空间 */
#define W25Q_RD_DATA_1B         0x03    /**< 读取1B数据 */
#define W25Q_RD_DATA            0x0B    /**< 快速读取数据 */
#define W25Q_WR_PAGE            0x02    /**< 写不超过1页数据 */
#define W25Q_4BYTE_ERASE        0x21
#define W25Q_4BYTE_RD_DATA      0x0C    /**< 快速读取数据 */
#define W25Q_4BYTE_WR_PAGE      0x12    /**< 写不超过1页数据*/
#define W25Q_4BYTE_ADDR         0xB7    /**< 设置4字节地址 */
#define DUMMY                   0xFF    /**< 无效命令 */

#define W25Q_BUSY               0x01    /**< BUSY位 */
#define W25Q_ADS                0x01    /**< ADS位 */

#define W25Q_ERASE_CHIP         0xC7

/** 公有函数 --------------------------------------------------- */

/**
  * @brief  芯片初始化
  * @param	None
  * @retval None
  * @note   
  */
void w25q_init(void)
{
    /** 片选除能 */
    W25Q_CS_HIGH();

    W25Q_WP_HIGH();
    W25Q_RST_HIGH();

    /** 读取flash的id */
    uint16_t id = w25q_id();
    printf("id = %x \r\n", id);  

    /** 如果是W25Q256，设置为4字节地址*/
    if(id == W25Q256)                
    {
        uint8_t ads = w25q_rd_ads();        /**< 读取状态位ads，判断是否为4字节地址  */
        if(ads == 0)			            /**< 如果不是4字节地址  */
        {
            w25q_set_4b();                    /**< 设置为4字节地址  */  
        }
        Is4ByteAddrMode = 1;
        ads = w25q_rd_ads();
        printf("abs=> %d\n", ads);
    }
}

uint8_t SPI1_RW(u8 TxData)
{		 			 
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);     //等待发送区空
    SPI_I2S_SendData(SPI1, TxData);                                     //通过外设SPIx发送一个byte数据
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    //等待接收完一个byte
    return SPI_I2S_ReceiveData(SPI1);                                   //返回通过SPIx最近接收的数据
}

void HAL_SPI_Transmit(uint8_t *data, uint32_t len, uint32_t timeout){
    for(int i = 0; i < len; i++){
        SPI1_RW(*(data + i));
    }
}

void HAL_SPI_Receive(uint8_t *data, uint32_t len, uint32_t timeout){
    for(int i = 0; i < len; i++){
        *(data + i) = SPI1_RW(DUMMY);
    }
}

void SPI_Transmit(uint8_t *sdata, uint8_t *rdata, uint32_t len, uint32_t timeout){
    for(int i = 0; i < len; i++){
        *(rdata + i) = SPI1_RW(*(sdata + i));
    }
}

/**
  * @brief  在指定地址，读指定长度的数据
  * @param	addr   32位地址
  * @param	size   字节数（< 256）
  * @param	p_dat  数据存储区指针
  * @retval NONE
  * @note  
  */
void w25q_rd_data(uint32_t addr, uint16_t size, uint8_t* p_dat)
{
    uint8_t addr_len = 3;
    uint8_t cmd_buf[6] = {W25Q_RD_DATA, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY};

    if(Is4ByteAddrMode)
    {
        addr_len = 4;
        cmd_buf[0] = W25Q_4BYTE_RD_DATA;
        cmd_buf[1] = (uint8_t)(addr >> 24); 
        cmd_buf[2] = (uint8_t)(addr >> 16); 
        cmd_buf[3] = (uint8_t)(addr >>  8);
        cmd_buf[4] = (uint8_t)addr;
        cmd_buf[5] = DUMMY;
    }	
    else
    {
        cmd_buf[0] = W25Q_RD_DATA;
        cmd_buf[1] = (uint8_t)(addr >> 16); 
        cmd_buf[2] = (uint8_t)(addr >>  8);
        cmd_buf[3] = (uint8_t)addr;  
    }

    W25Q_CS_LOW();
    HAL_SPI_Transmit(&cmd_buf[0], addr_len + 2, 1000);
    HAL_SPI_Receive(p_dat, size, 1000);
    W25Q_CS_HIGH();
}

/**
  * @brief  在指定地址，写指定长度的数据
  * @param	addr   32位地址
  * @param	size   字节数（< 256）
  * @param	p_dat  数据存储区指针
  * @retval NONE
  * @note   
  */
void w25q_wr_data(uint32_t addr, uint16_t size, uint8_t* p_dat)
{
    uint8_t addr_len = 3;
    uint32_t currentaddr = addr;
    uint8_t cmd_buf[5] = {W25Q_WR_PAGE, DUMMY, DUMMY, DUMMY, DUMMY}; 

    for(uint32_t offset = 0; offset < size; offset += 256){
        currentaddr = addr + offset;
        if((currentaddr % 4096) == 0)       
        {
            w25q_erase_4k(currentaddr);
            w25q_busy_wait();
        }

        if(Is4ByteAddrMode)
        {
            addr_len = 4;
            cmd_buf[0] = W25Q_4BYTE_WR_PAGE;
            cmd_buf[1] = (uint8_t)(currentaddr >> 24); 
            cmd_buf[2] = (uint8_t)(currentaddr >> 16); 
            cmd_buf[3] = (uint8_t)(currentaddr >>  8);
            cmd_buf[4] = (uint8_t)(currentaddr >>  0);
        }	
        else
        {
            cmd_buf[0] = W25Q_WR_PAGE;
            cmd_buf[1] = (uint8_t)(currentaddr >> 16); 
            cmd_buf[2] = (uint8_t)(currentaddr >>  8);
            cmd_buf[3] = (uint8_t)(currentaddr >>  0);  
        }
        w25q_wr_enable();
        W25Q_CS_LOW();
        HAL_SPI_Transmit(cmd_buf, addr_len + 1, 1000);		
        HAL_SPI_Transmit(p_dat + offset, 256, 1000);
        W25Q_CS_HIGH();
    }
}

void w25q_erase_chip(void)
{
    uint8_t cmd_buf[1] = {W25Q_ERASE_CHIP}; 
    w25q_wr_enable();
    w25q_busy_wait();

    W25Q_CS_LOW();
    HAL_SPI_Transmit(cmd_buf, 1, 1000);
    W25Q_CS_HIGH();
    w25q_busy_wait();
}


/** 私有函数 --------------------------------------------------- */



/**
  * @brief  w25q通讯命令（读写）
  * @param	p_data 命令缓冲区指针
  * @param	size   命令字节数
  * @retval None
  * @note   
  */
static void w25q_cmd(uint8_t* p_data, uint16_t size)
{
	w25q_busy_wait();
	
	W25Q_CS_LOW();
	SPI_Transmit(p_data, p_data, size, 1000);
	W25Q_CS_HIGH();	
}

/**
  * @brief  w25q通讯命令(单写)
  * @param	p_data 命令缓冲区指针
  * @param	size   命令字节数
  * @retval None
  * @note   
  */
static void w25q_wr_cmd(uint8_t* p_data, uint16_t size)
{
	w25q_busy_wait();
	
	W25Q_CS_LOW();
	HAL_SPI_Transmit(p_data, size, 1000);
	W25Q_CS_HIGH();	
	Delay(1);
}

/**
  * @brief  写使能
  * @param	None
  * @retval None
  * @note   
  */
static void w25q_wr_enable(void)
{
	uint8_t cmd_buf = W25Q_WR_ENABLE;	
	w25q_wr_cmd(&cmd_buf, 1);
}

/**
  * @brief  写除能
  * @param	None
  * @retval None
  * @note   
  */
static void w25q_wr_disable(void)
{
	uint8_t cmd_buf = W25Q_WR_DISABLE;	
	w25q_wr_cmd(&cmd_buf, 1);
}

/**
  * @brief  w25q忙等待
  * @param	None
  * @retval None
  * @note   
  */
static void w25q_busy_wait(void)
{
	uint8_t cmd_buf[2] = {W25Q_STATUS_1, DUMMY};	
	
	W25Q_CS_LOW();
	do
	{
        cmd_buf[0] = W25Q_STATUS_1;
        cmd_buf[1] = DUMMY;
		SPI_Transmit(cmd_buf, cmd_buf, 2, 1000);	
		cmd_buf[1] &= W25Q_BUSY;
	}while(cmd_buf[1]);
	
	W25Q_CS_HIGH();
}

/**
  * @brief  4k块擦除
  * @param	addr   32位地址
  * @retval None
  * @note   
  */
void w25q_erase_4k(uint32_t addr)
{
    uint8_t addr_len = 3;
    uint8_t cmd_buf[5] = {W25Q_4K_ERASE, DUMMY, DUMMY, DUMMY, DUMMY}; 

    if(Is4ByteAddrMode)
    {
        addr_len = 4;
        cmd_buf[0] = W25Q_4BYTE_ERASE;
        cmd_buf[1] = (uint8_t)(addr >> 24); 
        cmd_buf[2] = (uint8_t)(addr >> 16); 
        cmd_buf[3] = (uint8_t)(addr >>  8);
        cmd_buf[4] = (uint8_t)addr;
    }	
    else
    {
        cmd_buf[0] = W25Q_4K_ERASE;
        cmd_buf[1] = (uint8_t)(addr >> 16);
        cmd_buf[2] = (uint8_t)(addr >>  8);
        cmd_buf[3] = (uint8_t)addr;
    } 

    w25q_wr_enable();		
    w25q_cmd(&cmd_buf[0], addr_len + 1);	
    w25q_wr_disable();
}

/**
  * @brief  读取状态位ADS
  * @param	None
  * @retval ADS
  * @note   ADS = 0 为3字节地址；  ADS = 1 为4字节地址
  */
static uint8_t w25q_rd_ads(void)
{
    uint8_t cmd_buf[2] = {W25Q_STATUS_3, DUMMY};	

    W25Q_CS_LOW();

    SPI_Transmit(cmd_buf, cmd_buf, 2, 1000);
    cmd_buf[1] &= W25Q_ADS;
    W25Q_CS_HIGH();

    return(cmd_buf[1]);
}

/**
  * @brief  设置为4字节地址
  * @param	None
  * @retval None
  * @note   
  */
static void w25q_set_4b(void)
{
	uint8_t cmd_buf[2] = {W25Q_4BYTE_ADDR, DUMMY};
	w25q_cmd(&cmd_buf[0], 1);
}

/**
  * @brief  制造商ID(0xEF) + 设备ID
  * @param	None
  * @retval 0xEF00 + 0xID
  * @note   
  */
static uint16_t w25q_id(void)
{
    uint16_t id;
    uint8_t cmd_buf[6] = {W25Q_ID, DUMMY, DUMMY, 0, DUMMY, DUMMY};	
    w25q_cmd(&cmd_buf[0], 6);
    id = (uint16_t)(cmd_buf[4] << 8) + cmd_buf[5];
    return(id);
}