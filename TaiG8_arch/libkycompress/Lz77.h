//////////////////////////////
// LZ77.h
/////////////// ///////////////

// use in your own heap allocation index node, not sliding window
// maximum of 65536 bytes of compressed data
// optimized version

#ifndef _WIX_LZ77_COMPRESS_HEADER_001_
#define _WIX_LZ77_COMPRESS_HEADER_001_


#ifndef _WIN32

#include <iostream>
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        DWORD;
typedef bool                BOOL;

#define TRUE                true
#define FALSE               false

#define min(a, b) (a > b) ? b : a

#endif

// sliding window size in bytes
#define _MAX_WINDOW_SIZE 65536

class CCompress
{
public:
    CCompress () {};
    virtual ~ CCompress () {};
    
public:
    virtual int Compress(BYTE * src, int srclen, BYTE * dest) = 0;
    virtual BOOL Decompress(BYTE * src, int srclen, BYTE * dest, int destlen) = 0;
    
protected:
    // Tools
    
    ///////////////////////// ////////////////////////////////
    // CopyBitsInAByte: within a byte range to copy bit stream
    // Definition with CopyBits parameters
    // Description:
    // this function is called by CopyBits, do error checking, that is
    // supposed to copy the bits in a byte range
    void CopyBitsInAByte(BYTE * memDest, int nDestPos,
                         BYTE * memSrc, int nSrcPos, int nBits);
    
    //////////////////////////////////////////// ////////////
    // CopyBits: copy the memory bit stream
    // memDest - target data area
    // nDestPos - the first byte of the target data area start bit
    // memSrc - source data area
    // nSrcPos - the starting area of the first byte of the source data bits
    // nBits - the number of bits to be copied
    // Description:
    // start bit byte representation agreement from high to low ( From left to right)
    // followed by 0,1, ..., 7
    // to copy the data area can not have two coincident
    void CopyBits(BYTE * memDest, int nDestPos,
                  BYTE * memSrc, int nSrcPos, int nBits);
    
    ///////////////////////////////////////////////// /////////////
    // DWORD value from the upper byte to the lower byte aligned
    void InvertDWord(DWORD * pDW);
    
    /////////////// //////////////////////////////////////////////
    // set byte the first iBit bit ABIT
    // iBit order starting from the high-0 count (from left)
    void SetBit(BYTE * byte, int iBit, BYTE ABIT);
    
    /////////////// /////////////////////////////////////////////
    // get bytes the value of the first pos-bit byte
    // pos high order starting from 0 counting (from left)
    BYTE GetBit(BYTE byte, int pos);
    
    /////////////////// /////////////////////////////////////////
    // bit pointer * piByte (word section offset), * piBit (within bit byte offset) after the shift num bits
    void MovePos(int * piByte, int * piBit, int num);
    
    //////////////// /////////////////////////////////////////
    // take log2 (n) of upper_bound
    int UpperLog2(int n);
    
    /////////////////////////////////////////// //////////////
    // take log2 (n) of lower_bound
    int LowerLog2 (int n);
};

class CCompressKYModel : public CCompress
{
public:
    CCompressKYModel();
    virtual ~CCompressKYModel();
public:
    /////////////////////////////////////////////
    // compression some byte stream
    // src - the source data area
    // srclen - byte length of the source data area, srclen <= 65536
    // dest - compressed data area allocated srclen bytes of memory before calling
    // return value> 0 compressed data length
    // return value = 0 data can not be compressed
    // returns <0 compression exception error
    int Compress(BYTE *src, int srclen, BYTE * dest);
    
    //////////////// /////////////////////////////
    // decompress some byte stream
    // src - receiving raw data memory area, srclen < 65536 =
    // srclen - byte length of the source data area
    // dest - compressed data area
    // return value - success
    BOOL Decompress(BYTE * src, int srclen, BYTE * dest, int destlen);
    
protected:
    
    BYTE * pWnd;
    // the maximum window size is 64k, and not slide
    // maximum of 64k only compressed data, so you can easily start from the middle of the file decompression
    // length of the current window
    int nWndSize;
    
    // sort the sliding window in each 2-byte string
    // Sort order fast terms matching
    // sorting method is to use a 64k size pointer array
    // array subscript followed by a 2-byte string corresponding to each: (0000) (0001) ... (0100 ) (0101) ...
    // every pointer to a list, the list of nodes for the 2-byte string at each position appears
    struct STIDXNODE
    {
        WORD off; // offset in src
        WORD off2; // 2 for the corresponding byte sequence is repeated byte node
        // refers to off2 corresponds off from the 2-byte string
        WORD next; // the pointer in SortHeap
    };
    
    WORD SortTable [65536]; // 256 * 256 the next target point SortHeap pointer
    
    // sliding window is not because there is no operation to remove a node, so
    // node can continuously distributed SortHeap the
    struct STIDXNODE * SortHeap;
    int HeapPos; // assign the current location
    
    // current output position (byte offset and bit offset)
    int CurByte, CurBit;
    
protected:
    ///////////////////////////////// ///////
    // output compressed code
    // code - to the number of output
    // bits - the number of bits to be output (for isGamma = TRUE invalid)
    // isGamma - whether output γ coding
    void _OutCode(BYTE * dest, DWORD code, int bits, BOOL isGamma);
    
    ///////////////////////////////////// //////////////////////
    // Find the term in the sliding window
    // nSeekStart - where to begin matching
    // offset, len - for receiving the results, represented within the sliding window offset and length of
    // return value - identified any length of 3 or more matching byte string
    BOOL _SeekPhase(BYTE* src, int srclen, int nSeekStart, int* offset, int* len);
    
    ///////////////////////////////////////////////// //////////
    // get has matched 3-byte window position offset
    // how many bytes total matches
    inline int _GetSameLen(BYTE* src, int srclen, int nSeekStart, int offset);
    
    ////////////////////////////////////////// 
    // sliding window to the right n bytes of
    inline void _ScrollWindow(int n);
    
    // add a 2-byte string to the index in
    inline void _InsertIndexItem(int off);
    
    // initialize the index table, with the release of the last compression space 
    void _InitSortTable();
}; 


#endif // _WIX_LZW_COMPRESS_HEADER_001_
