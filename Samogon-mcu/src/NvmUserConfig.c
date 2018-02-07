#include "asf.h"
#include <crc32.h>
#include "NvmUserConfig.h"

#define CONFIG_PHYSICAL_ADDRESS (FLASH_ADDR + FLASH_SIZE - NVMCTRL_ROW_SIZE)

 void getDefaultNvmUserConfig(NvmUserConfig *pNvmConfig)
 {
	memset(pNvmConfig, 0, sizeof(NvmUserConfig));
	pNvmConfig->magic = 'SMGN';
	pNvmConfig->celsius = 1;
	pNvmConfig->targetTemperature = 90.0;
 }
 
 bool writeNvmUserConfig(NvmUserConfig *pNvmConfig)
 {
	// obtain the physical address of the NVM row to write
	uint32_t physicalAddress = CONFIG_PHYSICAL_ADDRESS;
	uint32_t crc32 = 0;
	int i = 0;
	enum status_code status;

	status = crc32_calculate((uint32_t)pNvmConfig, sizeof(NvmUserConfig) - sizeof(uint32_t), &crc32);
	if (status != STATUS_OK) 
	{
		return false;
	}
	pNvmConfig->crc32 = crc32;
	pNvmConfig->magic = 'SMGN';

    struct nvm_config config_nvm;
    nvm_get_config_defaults(&config_nvm);
    config_nvm.manual_page_write = false;
    nvm_set_config(&config_nvm);

	// erase row
	for (i = 0; i < 10; i++)
	{
		status = nvm_erase_row(physicalAddress);
		if (status == STATUS_OK)
		{
			break;
		}
	}

	if (status != STATUS_OK)
	{
		return false;
	}

	// write page
	for (i = 0; i < 10; i++)
	{
		status = nvm_write_buffer(physicalAddress, pNvmConfig, sizeof(NvmUserConfig));
		if (status == STATUS_OK)
		{
			break;
		}
	}

	return (status == STATUS_OK);
}
 
 bool readNvmUserConfig(NvmUserConfig *pNvmConfig)
 {
	// obtain the physical address of the NVM row to write
	uint32_t physicalAddress = CONFIG_PHYSICAL_ADDRESS;
	uint32_t crc32 = 0;
	int i = 0;
	enum status_code status;

	status = crc32_calculate(physicalAddress, sizeof(NvmUserConfig) - sizeof(uint32_t), &crc32);
	if (status != STATUS_OK)
	{
		return false;
	}

	NvmUserConfig *pNvmConfigPhysical = (NvmUserConfig *)physicalAddress;
	if (crc32 != pNvmConfigPhysical->crc32 || 'SMGN' != pNvmConfigPhysical->magic)
	{
		// the CRC don't match, bad or missing config in flash
		return false;
	}

	memcpy(pNvmConfig, pNvmConfigPhysical, sizeof(NvmUserConfig));

	return true;
 }
