#include "mru.h"




MRU::MRU()
{
    frames = 0;
}



MRU::~MRU()
{
    delete [] frames;
}


void MRU::setBufferManager( const BufMgr* mgrArg )
{
    Replacer::setBufferManager(mgrArg);

    int numBuffers = mgr->getNumBuffers();
    frames = new int[numBuffers];

    for ( int index = 0; index < numBuffers; ++index )
        frames[index] = -index;

    frames[0] = -numBuffers;
}


int MRU::pin ( int frameNo )
{
    if ( Replacer::pin(frameNo) != OK )
        return -1;

    update(frameNo);
    return OK;
}


int MRU::pick_victim()
{
    unsigned numBuffers = mgr->getNumBuffers();

    for ( unsigned i = 0; i < numBuffers; ++i )
        if (frames[i] < 0)
          {
            if ( i == 0 )
                frames[i] = 0;
            else
                frames[i] *= -1;
            int frame = frames[i];
            state_bit[frame] = Pinned;
            ++pin_count[frame];
            update(frame);
            return frame;
          }
  
    for ( unsigned i = 0; i < numBuffers; ++i )
      {
        int frame = frames[i];
        if ( state_bit[frame] != Pinned )
          {
            state_bit[frame] = Pinned;
            ++pin_count[frame];
#ifdef DEBUG
            std::cout << "Frame " << frame << " is a victim\n";
#endif
            update(frame);
            return frame;
          }
      }
#ifdef DEBUG
    std::cout << "No victims found!!!!!!\n";
#endif
    return -1;
}



void MRU::update( int frameNo )
{
    unsigned index, numBuffers=mgr->getNumBuffers();
    for ( index=0; index < numBuffers; ++index )
        if ( frames[index] < 0  ||  frames[index] == frameNo )
            break;


      // If buffer pool is not yet full, add this frame to it...
    if ( frames[index] < 0 )
        frames[index] = frameNo;

    int frame = frames[index];
    while ( index-- )
        frames[index+1] = frames[index];
    frames[0] = frame;
}



void MRU::info()
{
    Replacer::info();

    std::cout << "MRU REPLACEMENT";
    for (unsigned i = 0; i < mgr->getNumBuffers(); i++)
      {
        if (i % 5 == 0)
            std::cout << std::endl;
        std::cout << "\t" << frames[i];
      }
    std::cout << std::endl;
}



