#include <boost/foreach.hpp>
#include "Bitmaps.h"
#include "Util2.h"
#include "../MemPool.h"

using namespace std;

namespace Moses2
{

Bitmaps::Bitmaps()
{
}

Bitmaps::~Bitmaps()
{
	Clear();
}

void Bitmaps::Init(size_t inputSize, const std::vector<bool> &initSourceCompleted, MemPool &pool)
{
  m_pool = &pool;
  m_initBitmap = new (m_pool->Allocate<Bitmap>()) Bitmap(inputSize, pool);
  m_initBitmap->Initialize(initSourceCompleted);
  m_coll[m_initBitmap];
}

const Bitmap &Bitmaps::GetNextBitmap(const Bitmap &bm, const Range &range)
{
  Bitmap *newBM;
  if (m_recycler.empty()) {
	  newBM = new (m_pool->Allocate<Bitmap>()) Bitmap(bm.GetSize(), *m_pool);
  }
  else {
	  newBM = m_recycler.top();
	  m_recycler.pop();
  }
  newBM->Initialize(bm, range);

  Coll::const_iterator iter = m_coll.find(newBM);
  if (iter == m_coll.end()) {
    m_coll[newBM] = NextBitmaps();
    return *newBM;
  } else {
	m_recycler.push(newBM);
    return *iter->first;
  }
}

const Bitmap &Bitmaps::GetBitmap(const Bitmap &bm, const Range &range)
{
  Coll::iterator iter = m_coll.find(&bm);
  assert(iter != m_coll.end());

  const Bitmap *newBM;
  NextBitmaps &next = iter->second;
  NextBitmaps::const_iterator iterNext = next.find(&range);
  if (iterNext == next.end()) {
    // not seen the link yet.
    newBM = &GetNextBitmap(bm, range);
    next[&range] = newBM;
  } else {
    // link exist
    //std::cerr << "link exists" << endl;
    newBM = iterNext->second;
  }
  return *newBM;
}

void Bitmaps::Clear()
{
  m_coll.clear();
}

}

