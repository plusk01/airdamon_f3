#include "flash.h"

namespace airdamon {

void Flash::init(size_t len)
{
  // Calculate the required number of pages and the starting page (from end).
  numPages_ = Flash::calculateNumPages(len);
  startPage_ = FLASH_MEM_TOTAL_PAGES - numPages_;
  startAddr_ = FLASH_MEM_BASE_ADDR + FLASH_MEM_PAGE_SIZE*startPage_;
}

// ----------------------------------------------------------------------------

bool Flash::write(const void * const data, size_t len)
{
  // erase the pages we are about to write to
  erase();

  // how many words (4 bytes) are in the data we need to write?
  const uint32_t numWords = len / sizeof(uint32_t) + (len % sizeof(uint32_t) != 0);

  FLASH_Status status;


  for (uint8_t tries=3; tries>0; tries--) {

    FLASH_Unlock();

    const uint32_t * ptr = static_cast<const uint32_t *>(data);
    for (uint32_t i=0; i<numWords; ++i) {

      uint32_t addr = startAddr_ + i*sizeof(uint32_t);
      status = FLASH_ProgramWord(addr, *ptr++);
    }

    FLASH_Lock();

    if (status == FLASH_COMPLETE) break;
  }

  if (status != FLASH_COMPLETE) return false;

  // check data integrity after write
  return Flash::computeChecksum(data, len) == 
            Flash::computeChecksum(reinterpret_cast<uint32_t *>(startAddr_), len);
}

// ----------------------------------------------------------------------------

bool Flash::read(void * data, size_t len)
{
  memcpy(data, reinterpret_cast<uint8_t *>(startAddr_), len);
  return true;
}

// ----------------------------------------------------------------------------

bool Flash::erase()
{
  bool status = true;

  FLASH_Unlock();

  // Erase all the pages we have purview of
  for (uint8_t i=0; i<numPages_; ++i) {
    if (FLASH_ErasePage(startAddr_ + i*FLASH_MEM_PAGE_SIZE) != FLASH_COMPLETE) {
      status = false;
      break;
    }
  }

  FLASH_Lock();

  return status;
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

uint8_t Flash::computeChecksum(const void * const data, size_t len)
{
  uint8_t chksum = 0;

  const uint8_t * p = static_cast<const uint8_t *>(data);
  for (uint32_t i=0; i<len; ++i) {
    chksum ^= p[i];
  }

  return chksum;
}

// ----------------------------------------------------------------------------

uint8_t Flash::calculateNumPages(size_t len)
{
  return len / FLASH_MEM_PAGE_SIZE + (len % FLASH_MEM_PAGE_SIZE != 0);
}

} // ns airdamon
