/**
 * @file flash.h
 * @brief Emulated EEPROM using STM32F3 flash operations.
 * @author Parker Lusk <parkerclusk@gmail.com>
 * @date 28 April 2019
 */

#ifndef FLASH_H
#define FLASH_H

#include "system.h"

namespace airdamon {

  class Flash
  {
  public:
    Flash() = default;
    ~Flash() = default;

    /**
     * @brief      Initialize the flash-based non-volatile memory
     *
     * @param[in]  len   The length (in bytes) of data to be managed.
     *                   Used to calculate number of pages to use.
     *                   ASSUMPTION: The size of the program is small
     *                   enough to accomodate this many pages at the end
     *                   of the flash memory map.
     */
    void init(size_t len);

    /**
     * @brief      Write data into non-volatile memory
     *
     * @param[in]  data  The data to write
     * @param[in]  len   The length of data (in bytes)
     *
     * @return     true if successful
     */
    bool write(const void * const data, size_t len);

    /**
     * @brief      Read data from non-volatile memory
     *
     * @param      data  The data
     * @param[in]  len   The length of data (in bytes)
     *
     * @return     true if successful
     */
    bool read(void * data, size_t len);

    /**
     * @brief      Erases the pages associated with this non-volatile 
     *             memory store (calculated from init).
     *
     * @return     true if successful
     */
    bool erase();

    // getters
    uint8_t getNumPages() const { return numPages_; } 
    uint8_t getStartPage() const { return startPage_; } 
    uint32_t getStartAddr() const { return startAddr_; } 

  private:
    // The following are specific to STM32F303xB/C devices. See Sec 4.2,
    // Table 7 (p. 65 of 1141) of reference manual RM0316, Rev 8.
    static constexpr uint32_t FLASH_MEM_BASE_ADDR = 0x08000000; ///< Page 0
    static constexpr uint16_t FLASH_MEM_PAGE_SIZE = 0x800; ///< 2 KB (2048 bytes)
    static constexpr uint8_t FLASH_MEM_TOTAL_PAGES = 128; /// < 128*2K = 256 KB flash

    uint8_t numPages_; ///< Number of pages required
    uint8_t startPage_; ///< Page number to start read/write at
    uint32_t startAddr_; ///< memory address of 'startPage_'
    
    /**
     * @brief      Simple checksum calculation to ensure write integrity
     *
     * @param[in]  data  The data
     * @param[in]  len   The length (in bytes)
     *
     * @return     The checksum.
     */
    static uint8_t computeChecksum(const void * const data, size_t len);

    /**
     * @brief      Given the number of bytes required to store in flash memory,
     *             calculate the number of memory pages / sectors needed.
     *
     * @param[in]  len   The length of the data to store (in bytes)
     *
     * @return     The number of pages required
     */
    static uint8_t calculateNumPages(size_t len);

  };

}

#endif // FLASH_H
