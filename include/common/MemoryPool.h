#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__
#include <common/typedefs.h>
#include <assert.h>

#ifdef _WIN64
const static int kAddrSize = 8;
#else
const static int kAddrSize = 4;
#endif

class MemoryPool {
public:
	explicit MemoryPool(int count, int size);
	~MemoryPool();

	uint8_t * Get();
	bool Push(uint8_t * chunk);

private:
	MemoryPool() {};
	MemoryPool(const MemoryPool&) {}
	MemoryPool & operator=(const MemoryPool&) {}

private:
	uint8_t * m_pPool;
	uint8_t * m_pHead;
	uint8_t * m_pLast;

	int m_iCount;
	int m_iChunkSize;
	int m_iFreeCount;
};

#endif // !__MEMORY_POOL_H__
