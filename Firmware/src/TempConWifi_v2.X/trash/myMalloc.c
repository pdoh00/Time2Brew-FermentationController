#include <string.h>

#include "myMalloc.h"
#include "main.h"

#define SETBIT(val,bitnum) {val |= BITMASK[bitnum];}
#define CLEARBIT(val,bitnum) {val &= (~BITMASK[bitnum]);}
#define GETBIT(val,bitnum) (val & BITMASK[bitnum])
#define BLOCK_IS_UNALLOCATED(blockIdx) (GETBIT(heap->BlockDataAllocated, blockIdx) == 0)
#define BLOCK_IS_ALLOCATED(blockIdx) (GETBIT(heap->BlockDataAllocated, blockIdx) >0)
#define BLOCK_IS_FREE(blockIDX) (GETBIT(heap->BlockInUse, blockIDX) == 0)
#define BLOCK_IS_IN_USE(blockIDX) (GETBIT(heap->BlockInUse, blockIDX) > 0)
const unsigned int BITMASK[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};

unsigned int getBitMask(char x) {
    unsigned int ret = 1;
    while (x--) ret << 1;
    return ret;
}

void myMalloc_Init(HEAP *obj, unsigned char *Heap, unsigned int HeapSize) {
    memset(obj, 0, sizeof (HEAP));
    SETBIT(obj->BlockInUse, 0);
    obj->blocks[0].DataAddress = Heap;
    obj->blocks[0].Length = HeapSize;
    obj->blocks[0].NextBlockIdx = -1;
    obj->RootIdx = 0;
}

void **myMalloc(HEAP *heap, unsigned int RequestedLength) {
    void **retVal = NULL;
    DISABLE_INTERRUPTS;
    int idx;
    //Allocate in 16 Byte chunks
    if (RequestedLength & 0xF) {
        RequestedLength >>= 4;
        RequestedLength++;
        RequestedLength <<= 4;
    }

    int BestBlockIdx = -1;
    int BestBlockLength = 32767;
    for (idx = 0; idx < 16; idx++) {
        if (BLOCK_IS_IN_USE(idx)) {
            if (BLOCK_IS_UNALLOCATED(idx)) {
                if (heap->blocks[idx].Length > RequestedLength) {
                    if (heap->blocks[idx].Length < BestBlockLength) {
                        BestBlockIdx = idx;
                        BestBlockLength = heap->blocks[idx].Length;
                    }
                }
            }
        }
    }

    if (BestBlockIdx < 0) goto AllocComplete;

    myMalloc_block *bestBlock = &heap->blocks[BestBlockIdx];

    if (BestBlockLength - RequestedLength >= 64) {
        //The new chunk is big enough to split off
        int newNextIdx = -1;
        for (idx = 0; idx < 16; idx++) {
            if (BLOCK_IS_FREE(idx)) {
                newNextIdx = idx;
                break;
            }
        }
        if (newNextIdx >= 0) {
            SETBIT(heap->BlockInUse, newNextIdx);
            CLEARBIT(heap->BlockDataAllocated, newNextIdx);
            myMalloc_block *Next = &heap->blocks[newNextIdx];
            Next->DataAddress = bestBlock->DataAddress + RequestedLength;
            Next->Length = (bestBlock->Length - RequestedLength);
            Next->NextBlockIdx = bestBlock->NextBlockIdx;

            SETBIT(heap->BlockDataAllocated, BestBlockIdx);
            bestBlock->Length = RequestedLength;
            bestBlock->NextBlockIdx = newNextIdx;

            retVal = &bestBlock->DataAddress;
            goto AllocComplete;
        }
    }
    SETBIT(heap->BlockDataAllocated, BestBlockIdx);
    retVal = &bestBlock->DataAddress;
AllocComplete:
    ENABLE_INTERRUPTS;
    return retVal;
}

void myFree(HEAP *heap, void **blockPointer) {
    DISABLE_INTERRUPTS;
    int idx;
    int thisIdx = -1;
    for (idx = 0; idx < 16; idx++) {
        if (BLOCK_IS_IN_USE(idx)) {
            if (&(heap->blocks[idx].DataAddress) == blockPointer) {
                thisIdx = idx;
                break;
            }
        }
    }
    if (thisIdx == -1) goto FreeComplete;

    CLEARBIT(heap->BlockDataAllocated, thisIdx); //Free the RAM

    //Consolidate the Free Space.
    myMalloc_block *thisBlock = &heap->blocks[thisIdx];
    myMalloc_block *nextBlock;
    int nextIdx;
    while (1) {
        if (thisBlock->NextBlockIdx < 0) goto FreeComplete;
        nextIdx = thisBlock->NextBlockIdx;
        nextBlock = &heap->blocks[nextIdx];

        if (BLOCK_IS_ALLOCATED(thisIdx)) {
            //I am in use so we need to move on the the next block...
            thisIdx = nextIdx;
            thisBlock = &heap->blocks[thisIdx];
            continue;
        }

        if (BLOCK_IS_ALLOCATED(nextIdx)) {
            //I am free but the next block is not free 
            //Is the next block then end of the heap? If so we're done.
            if (nextBlock->NextBlockIdx < 0) goto FreeComplete;

            //Nope the heap goes on so move to the next next block and keep looking
            //for contingous free blocks...
            thisIdx = nextBlock->NextBlockIdx;
            thisBlock = &heap->blocks[thisIdx];
            continue;
        }

        //I am free and the next block is free.  Let's absorb the next block into ourselves!
        CLEARBIT(heap->BlockInUse, nextIdx); //Mark the block as free...
        thisBlock->NextBlockIdx = nextBlock->NextBlockIdx; //Skip over the next block and link to its follower...
        thisBlock->Length += nextBlock->Length; //Expand our size...
    }

FreeComplete:
    ENABLE_INTERRUPTS;
    return;
}

static void myCompactHeap(HEAP *heap) {
    DISABLE_INTERRUPTS;
    int thisIdx = heap->RootIdx;
    int nextIdx = -1;
    int idx;
    myMalloc_block *thisBlock = NULL;
    myMalloc_block *nextBlock = NULL;
    while (1) {
        if (thisIdx < 0) goto CompactFinished;
        thisBlock = &heap->blocks[thisIdx];
        nextIdx = thisBlock->NextBlockIdx;

        //Is this the last block..If so exit; the heap is optimal.
        if (nextIdx < 0) goto CompactFinished;

        nextBlock = &heap->blocks[nextIdx];

        if (BLOCK_IS_ALLOCATED(thisIdx)) {
            //I am in use so walk to the next block and continue...
            thisIdx = nextIdx;
            continue;
        }

        //I am UNALLOCATED and I am ALSO NOT THE LAST BLOCK...
        //Is the next block free as well?
        if (BLOCK_IS_UNALLOCATED(nextIdx)) {
            //The Next block is free... we need to absorb it into one MEGA FREE BLOCK
            CLEARBIT(heap->BlockInUse, nextIdx); //Mark the block as free...
            thisBlock->NextBlockIdx = nextBlock->NextBlockIdx; //Skip over the next block and link to its follower...
            thisBlock->Length += nextBlock->Length; //Expand our size...
            //Now start over again...
            continue;
        }

        //The next block is allocated... but we're unallocated! BAD!
        //Slide the next block's data back to our start data address...
        unsigned char *dst = thisBlock->DataAddress;
        unsigned char *src = nextBlock->DataAddress;
        int len = nextBlock->Length;
        //Slide the whole dataset backwards... At the end "thisBlock" will be what was once the "nextBlock"
        while (len--) {
            *(dst++) = *(src++);
        }

        //Update the NextBlock so It know where it's data is NOW
        nextBlock->DataAddress = thisBlock->DataAddress;

        //Move our Data address to start at the end of the "nextBlock"
        thisBlock->DataAddress = nextBlock->DataAddress + nextBlock->Length;

        //None of the sizes changed... it was a slide only...

        //Now fix the linked list...
        thisBlock->NextBlockIdx = nextBlock->NextBlockIdx;
        nextBlock->NextBlockIdx = thisIdx;

        //Now we need to update our Parent so it no longer points at us...but the "nextBlock" instead.
        //Not to worry.. the "nextBlock" points to us so we don't get forgotten.
        if (thisIdx == heap->RootIdx) {
            //The root doesn't have any parent. We just need to set the "nextblock" as the new root.           
            heap->RootIdx = nextIdx;
        } else {
            //Who is our Parent?
            for (idx = 0; idx < 16; idx++) {
                if (BLOCK_IS_IN_USE(idx)) {
                    if (heap->blocks[idx].NextBlockIdx == thisIdx) {
                        //Found you! Update our old parent with thier new child's index...
                        heap->blocks[idx].NextBlockIdx = nextIdx;
                        break;
                    }
                }
            }
        }
        //Repeat
    }

CompactFinished:
    ENABLE_INTERRUPTS;
    return;
}
