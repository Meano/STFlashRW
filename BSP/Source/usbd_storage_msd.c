/**
  ******************************************************************************
  * @file    usbd_storage_msd.c
  * @author  MCD application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the disk operations functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "usbd_msc_mem.h"
#include "usb_conf.h"
#include "VirtualFatFS.h"
#include "DCMI.h"
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup STORAGE 
  * @brief media storage application module
  * @{
  */ 

/** @defgroup STORAGE_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STORAGE_Private_Defines
  * @{
  */ 

#define STORAGE_LUN_NBR                  1 
/**
  * @}
  */ 


/** @defgroup STORAGE_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STORAGE_Private_Variables
  * @{
  */ 
/* USB Mass storage Standard Inquiry Data */
const int8_t  STORAGE_Inquirydata[] = {//36
  
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (USBD_STD_INQUIRY_LENGTH - 5),
  0x00,
  0x00,	
  0x00,
  'M', 'e', 'g', 'a', 'h', 'u', 'n', 't', /* Manufacturer : 8 bytes */
  'T', 'e', 's', 't', 'Q', 'R', 'c', 'o', /* Product      : 16 Bytes */
  'd', 'e', ' ', ' ', ' ', ' ', ' ', ' ',
  '1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
}; 

/**
  * @}
  */ 


/** @defgroup STORAGE_Private_FunctionPrototypes
  * @{
  */ 
int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun, 
                           uint32_t *block_num, 
                           uint32_t *block_size);

int8_t  STORAGE_IsReady (uint8_t lun);

int8_t  STORAGE_IsWriteProtected (uint8_t lun);

int8_t STORAGE_Read (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_Write (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_GetMaxLun (void);


USBD_STORAGE_cb_TypeDef USBD_VirualFatFS_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef  *USBD_STORAGE_fops = &USBD_VirualFatFS_fops;

__IO uint32_t count = 0;
/**
  * @}
  */ 


/** @defgroup STORAGE_Private_Functions
  * @{
  */ 


/**
  * @brief  Initialize the storage medium
  * @param  lun : logical unit number
  * @retval Status
  */

int8_t STORAGE_Init (uint8_t lun)
{
	vfs_init("MeanoQRCode", 0x0100000);
	return 0;
}

/**
  * @brief  return medium capacity and block size
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
#define UDiskSize 0x01000000
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
	*block_size =  512;  
	*block_num =  UDiskSize/512;  
	return (0);
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{  
	return 0;
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
  return  0;
}

/**
  * @brief  Read data from the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to save data
  * @param  blk_addr :  address of 1st block to be read
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
uint32_t ReadBufferSequence = 0;
bool BufferCrashed = false;
int8_t STORAGE_Read (uint8_t lun, 
                 uint8_t *buffer, 
                 uint32_t block_number,                       
                 uint16_t blk_len)
{
	uint32_t addr = block_number * 512;
	uint32_t len = blk_len * 512;
	uint32_t index = 0;
	// 命令超界
	if((block_number + count) > (16 * 1024)){
		//*buffer = 0;
		return 0;
	}
	// 获取帧信息
	if(block_number == 4096) {
		// 帧读取完成,解锁Buffer
		if(DeviceInfo->CurrentStatus == USBBusyFlag) {
			addr = (uint32_t)GetDeviceInfoBuffer();
			ReadBufferSequence = CurrentFrame->Sequence;
			FrameBufferUnLock(USBBusyFlag, true, FrameForUSB);
		}
		// 读取帧信息,符合条件锁Buffer
		else {
			if(
				DeviceInfo->BufferOwner != FrameForDecode &&
				DeviceInfo->BufferStatus == 0 &&
				DeviceInfo->BufferSequence != ReadBufferSequence
			) {
				FrameBufferLock(USBBusyFlag);
			}
			addr = (uint32_t)GetDeviceInfoBuffer();
		}
	}
	else if(block_number >= 2048 && block_number < 4096){
		// 读取的过程中发生帧变化
		if(CurrentFrame->Status != 0) {
			addr = 0;
		}
		// 正常帧读取
		else {
			addr = (uint32_t)(CurrentFrame->Buffer) + (block_number - 2048) * 512;
		}
	}
	else {
		vfs_read(block_number, buffer, blk_len);
		return 0;
	}
	for(index = 0; index < len; index += 4){
		*((uint32_t *)(buffer + index)) = addr != 0 ? *((uint32_t *)(addr + index)) : 0;
	}
	return 0;
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */

int8_t STORAGE_Write (uint8_t lun, 
                  uint8_t *buf, 
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
	if(blk_addr < 1024){
		vfs_write(blk_addr, buf, blk_len);
		return 0;
	}else{
		return 0;
	}
}

/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */

int8_t STORAGE_GetMaxLun (void)
{
	return (STORAGE_LUN_NBR - 1);
}

