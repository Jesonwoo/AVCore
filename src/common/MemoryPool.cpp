#include <common/MemoryPool.h>
#include <common/corelog.hpp>
#include <new>

MemoryPool::MemoryPool(int count, int size)
	: m_pPool(NULL)
	, m_pHead(NULL)
	, m_pLast(NULL)
	, m_iCount(count)
	, m_iChunkSize(size)
	, m_iFreeCount(0)
{
	assert(count > 0 || size > 0);
	
	try {
		m_pPool = new uint8_t[(size + kAddrSize) * count];
	} catch(const std::bad_alloc & e) {
        LOG_SCOPE_F(ERROR, "bad_allc: %s", e.what());
        LOG_F(ERROR, "couldn't new a memory pool, chunk size: %d, count: %d\n",
              m_iChunkSize, m_iCount);
		return;
	}
}

MemoryPool::~MemoryPool()
{
	delete[] m_pPool;
	m_pPool = NULL;
	m_pHead = NULL;
	m_pLast = NULL;
	m_iCount = 0;
	m_iChunkSize = 0;
	m_iFreeCount = 0;
}

uint8_t * MemoryPool::Get()
{

	return NULL;
}

bool MemoryPool::Push(uint8_t * chunk)
{

	return false;
}
