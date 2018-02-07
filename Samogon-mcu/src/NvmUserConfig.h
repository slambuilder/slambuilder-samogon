#ifndef NVMUSERCONFIG_H_
#define NVMUSERCONFIG_H_

// This structure is persisted in the NMV memory, at the last page of the flash
// Ensure the size of the structure is exactly one page (FLASH_USER_PAGE_SIZE = 64 bytes = 16 DWORDS)
typedef struct TNvmUserConfig
{
	uint32_t magic;				// Should be a magic symbol 'SMGN'
	float targetTemperature;	// the target temperature last set by the user
	uint32_t celsius;			// User display: 1 for Celsius, 0 for Fahrenheit
	uint32_t reserved[12];		// filler to ensure the sizeof(NvmUserConfig) == FLASH_USER_PAGE_SIZE
	uint32_t crc32;				// Always keep crc32 field at the very end of the structure
} NvmUserConfig;

void getDefaultNvmUserConfig(NvmUserConfig *pNvmConfig);
bool writeNvmUserConfig(NvmUserConfig *pNvmConfig);
bool readNvmUserConfig(NvmUserConfig *pNvmConfig);

#endif /* NVMUSERCONFIG_H_ */