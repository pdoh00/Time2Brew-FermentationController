/* 
 * File:   myMalloc.h
 * Author: THORAXIUM
 *
 * Created on March 2, 2015, 12:15 AM
 */

#ifndef MYMALLOC_H
#define	MYMALLOC_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        void *DataAddress;
        unsigned int Length;
        int NextBlockIdx;
    } myMalloc_block;

    typedef struct {
        unsigned int BlockInUse;
        unsigned int BlockDataAllocated;
        myMalloc_block blocks[16];
        //unsigned int myHeapSize;
        unsigned char *heap;
        unsigned int RootIdx;
    } HEAP;

    void myMalloc_Init(HEAP *obj, unsigned char *Heap, unsigned int HeapSize);
    void **myMalloc(HEAP *heap, unsigned int RequestedLength);
    void myFree(HEAP *heap, void **blockPointer);

#ifdef	__cplusplus
}
#endif

#endif	/* MYMALLOC_H */

