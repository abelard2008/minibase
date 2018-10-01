#include "lru.h"



LRU::LRU()
{
    frames = 0;
}



LRU::~LRU()
{
    delete [] frames;
}


void LRU::setBufferManager( const BufMgr* mgrArg )
{
    Replacer::setBufferManager(mgrArg);
    frames = new unsigned[ mgr->getNumBuffers() ];
    nframes = 0;
}


int LRU::pin ( int frameNo )
{
    if ( Replacer::pin(frameNo) != OK )
        return -1;

    update(frameNo);
    return OK;
}


int LRU::pick_victim()
{
    unsigned numBuffers = mgr->getNumBuffers();

    if ( nframes < numBuffers )
      {
	int frame = nframes++;
	frames[frame] = frame;
	state_bit[frame] = Pinned;
	++pin_count[frame];
	return frame;
      }

    for ( unsigned i = 0; i < numBuffers; ++i )
      {
        int frame = frames[i];
        if ( state_bit[frame] != Pinned )
          {
            state_bit[frame] = Pinned;
            ++pin_count[frame];
            update(frame);
            return frame;
          }
      }

    return -1;
}


 // This pushes the given frame to the end of the list.
void LRU::update( int frameNo )
{
    unsigned index;
    for ( index=0; index < nframes; ++index )
        if ( frames[index] == (unsigned)frameNo )
            break;

    while ( ++index < nframes )
        frames[index-1] = frames[index];
    frames[nframes-1] = frameNo;
}



void LRU::info()
{
    Replacer::info();

    std::cout << "LRU REPLACEMENT";
    for (unsigned i = 0; i < nframes; i++)
      {
        if (i % 5 == 0)
            std::cout << std::endl;
        std::cout << "\t" << frames[i];
      }
    std::cout << std::endl;
}



