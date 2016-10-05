//////////////////////////////
// LZ77.CPP
//////////////////////////////

//#include <windows.h>
#include <stdio.h>
#include <memory.h>
//#include <crtdbg.h>
#include "Lz77.h"

///////////////////////////////////////////////////////// 
// Convert log2(n) of upper_bound
int CCompress::UpperLog2(int n) 
{ 
	int i = 0; 
	if (n > 0)
	{ 
		int m = 1; 
		while(1) 
		{ 
			if (m >= n) 
				return i; 
			m <<= 1; 
			i++; 
		} 
	} 
	else  
		return -1; 
} 
// UpperLog2 
/////////////////////////////////////////////////////////

///////////////////////////////////////////////////////// 
// Take log2(n) of lower_bound
int CCompress::LowerLog2(int n) 
{ 
	int i = 0; 
	if (n > 0) 
	{ 
		int m = 1; 
		while(1) 
		{ 
			if (m == n) 
				return i; 
			if (m > n) 
				return i - 1; 
			m <<= 1;
			i++; 
		} 
	} 
	else  
		return -1; 
} 
// LowerLog2 
///////////////////////////////////////////////////////// 

////////////////////////////////////////////////////////////
// Bit pointer * piByte (byte offset), * piBit (byte offset within the bit) bit after shift num
void CCompress::MovePos(int* piByte, int* piBit, int num) 
{ 
	num += (*piBit); 
	(*piByte) += num / 8; 
	(*piBit) = num % 8; 
}
// MovePos 
////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////// 
// Get the value of the first pos-bit byte byte
// Pos high order starting from 0 counting (from left)
BYTE CCompress::GetBit(BYTE byte, int pos) 
{ 
	int j = 1; 
	j <<= 7 - pos; 
	if (byte & j) 
		return 1; 
	else  
		return 0; 
} 
// GetBit 
///////////////////////////////////////////////////////////// 
/////////////////////////////////////////////////////////////
// Set byte first iBit bit aBit
// IBit order starting from the high-0 count (from left)
void CCompress::SetBit(BYTE* byte, int iBit, BYTE aBit) 
{ 
	if (aBit) 
		(*byte) |= (1 << (7 - iBit)); 
	else 
		(*byte) &= ~(1 << (7 - iBit)); 
} 
// SetBit 
//////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////// 
// DWORD value order from the upper byte to the lower byte
void CCompress::InvertDWord(DWORD* pDW) 
{ 
	union UDWORD{ DWORD dw; BYTE b[4]; }; 
	UDWORD* pUDW = (UDWORD*)pDW; 
	BYTE b; 
	b = pUDW->b[0];	pUDW->b[0] = pUDW->b[3]; pUDW->b[3] = b; 
	b = pUDW->b[1];	pUDW->b[1] = pUDW->b[2]; pUDW->b[2] = b; 
} 
// InvertDWord 
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////// 
// CopyBits: Copy the memory bit stream
// MemDest - destination data area
// NDestPos - the first byte of the target data area start bit
// MemSrc - source data area
// NSrcPos - starting the first byte of the source data area bits
// NBits - the number of bits to be copied
// Description:
// Start bit byte representation agreement from high to low (from left to right)
// Followed by 0,1, ..., 7
// Two data area to be copied can not overlap
void CCompress::CopyBits(BYTE* memDest, int nDestPos,  
			  BYTE* memSrc, int nSrcPos, int nBits) 
{ 
	int iByteDest = 0, iBitDest; 
	int iByteSrc = 0, iBitSrc = nSrcPos; 
	int nBitsToFill, nBitsCanFill; 
	while (nBits > 0) 
	{
        // Calculate the target area to the current byte padding bits
		nBitsToFill = min(nBits, iByteDest ? 8 : 8 - nDestPos);
        // Target area to fill the current byte start bit
		iBitDest = iByteDest ? 0 : nDestPos;
        // Calculate can copy the data from the source area median
		nBitsCanFill = min(nBitsToFill, 8 - iBitSrc);
        // Copy the bytes within
		CopyBitsInAByte(memDest + iByteDest, iBitDest,
                        memSrc + iByteSrc, iBitSrc, nBitsCanFill);
        // If you have not copied a nBitsToFill
		if (nBitsToFill > nBitsCanFill) 
		{ 
			iByteSrc++; iBitSrc = 0; iBitDest += nBitsCanFill; 
			CopyBitsInAByte(memDest + iByteDest, iBitDest,  
					memSrc + iByteSrc, iBitSrc,  
					nBitsToFill - nBitsCanFill); 
			iBitSrc += nBitsToFill - nBitsCanFill; 
		} 
		else  
		{ 
			iBitSrc += nBitsCanFill; 
			if (iBitSrc >= 8) 
			{ 
				iByteSrc++; iBitSrc = 0; 
			} 
		} 
		nBits -= nBitsToFill;   // position has been filled nBitsToFill
		iByteDest++; 
	}	 
} 
// CopyBits 
/////////////////////////////////////////////////////////

///////////////////////////////////////////////////////// 
// CopyBitsInAByte: a byte range within the bit stream copy
// Definition of parameters with CopyBits
// Description:
// This function is called by CopyBits, do error checking, that is,
// Assume that you want to copy the bits in a byte within the range
void CCompress::CopyBitsInAByte(BYTE* memDest, int nDestPos,
                                BYTE* memSrc, int nSrcPos, int nBits)
{
	BYTE b1, b2;
	b1 = *memSrc;
    b1 <<= nSrcPos; b1 >>= 8 - nBits;   // will not copied bit is cleared
    b1 <<= 8 - nBits - nDestPos;        // source and destination byte alignment
	*memDest |= b1;                     // Copy the value of bit 1
	b2 = 0xff; b2 <<= 8 - nDestPos;     // location will not replicate 1
	b1 |= b2;
	b2 = 0xff; b2 >>= nDestPos + nBits;
	b1 |= b2; 
	*memDest &= b1; // Copy the value of bit 0
}
// CopyBitsInAByte 
/////////////////////////////////////////////////////////

//------------------------------------------------------------------ 

CCompressKYModel::CCompressKYModel()
{	 
	SortHeap = new struct STIDXNODE[_MAX_WINDOW_SIZE]; 
}

CCompressKYModel::~CCompressKYModel()
{ 
	delete[] SortHeap; 
}

// Initialize the index table, with the release of the last compression space
void CCompressKYModel::_InitSortTable()
{ 
	memset(SortTable, 0, sizeof(WORD) * 65536); 
	nWndSize = 0; 
	HeapPos = 1; 
}

// Add a 2-byte string to the index
void CCompressKYModel::_InsertIndexItem(int off)
{ 
	WORD q; 
	BYTE ch1, ch2; 
	ch1 = pWnd[off]; ch2 = pWnd[off + 1];	 
	if (ch1 != ch2) 
	{
        // Create a new node
		q = HeapPos; 
		HeapPos++; 
		SortHeap[q].off = off; 
		SortHeap[q].next = SortTable[ch1 * 256 + ch2]; 
		SortTable[ch1 * 256 + ch2] = q; 
	} 
	else 
	{
        // Repeated 2-byte string
        // Offset because there is no virtual nor delete operation, if you compare the first node
        // If off and connected to
		q = SortTable[ch1 * 256 + ch2]; 
		if (q != 0 && off == SortHeap[q].off2 + 1) 
		{
            // Node merging
			SortHeap[q].off2 = off; 
		}		 
		else 
		{
            // Create a new node
			q = HeapPos; 
			HeapPos++; 
			SortHeap[q].off = off; 
			SortHeap[q].off2 = off; 
			SortHeap[q].next = SortTable[ch1 * 256 + ch2]; 
			SortTable[ch1 * 256 + ch2] = q; 
		} 
	} 
}

//////////////////////////////////////////
// Sliding window right n bytes
void CCompressKYModel::_ScrollWindow(int n)
{	 
	for (int i = 0; i < n; i++) 
	{		 
		nWndSize++;		 
		if (nWndSize > 1)			 
			_InsertIndexItem(nWndSize - 2); 
	} 
}

////////////////////////////////////////////////// /////////
// Get has matched two-byte window position offset
// How many bytes total matches
int CCompressKYModel::_GetSameLen(BYTE* src, int srclen, int nSeekStart, int offset)
{ 
	int i = 2; // has matched two bytes
	int maxsame = min(srclen - nSeekStart, nWndSize - offset); 
	while (i < maxsame 
			&& src[nSeekStart + i] == pWnd[offset + i]) 
		i++; 
	//_ASSERT(nSeekStart + i <= srclen && offset + i <= nWndSize);
	return i; 
}

////////////////////////////////////////////////// /////////
// It finds the term in the sliding window
// NSeekStart - where to start matching
// Offset, len - for receiving the results expressed in the offset and length of the sliding window
// Return value - whether found a length of 2 or more matching string of bytes
BOOL CCompressKYModel::_SeekPhase(BYTE* src, int srclen, int nSeekStart, int* offset, int* len)
{
	int j, m, n; 
	if (nSeekStart < srclen - 1) 
	{ 
		BYTE ch1, ch2; 
		ch1 = src[nSeekStart]; ch2 = src[nSeekStart + 1]; 
		WORD p; 
		p = SortTable[ch1 * 256 + ch2]; 
		if (p != 0) 
		{ 
			m = 2; n = SortHeap[p].off; 
			while (p != 0) 
			{ 
				j = _GetSameLen(src, srclen,  
					nSeekStart, SortHeap[p].off); 
				if ( j > m ) 
				{  
					m = j;  
					n = SortHeap[p].off; 
				}			 
				p = SortHeap[p].next; 
			}	 
			(*offset) = n;  
			(*len) = m; 
			return TRUE;
		}	 
	} 
	return FALSE; 
}

////////////////////////////////////////
// Output compressed code
// Code - number to be output
// Bits - the number of bits to be output (for isGamma = TRUE is invalid)
// IsGamma - whether output γ coding
void CCompressKYModel::_OutCode(BYTE* dest, DWORD code, int bits, BOOL isGamma)
{	 
	if ( isGamma ) 
	{
		BYTE* pb;
        DWORD out;
        // Calculate the number of output bits
		int GammaCode = (int)code - 1; 
		int q = LowerLog2(GammaCode); 
		if (q > 0) 
		{ 
			out = 0xffff; 
			pb = (BYTE*)&out;
            // Output of q 1
			CopyBits(dest + CurByte, CurBit,  
				pb, 0, q); 
			MovePos(&CurByte, &CurBit, q); 
		}
        // Outputs 0
		out = 0; 
		pb = (BYTE*)&out;		 
		CopyBits(dest + CurByte, CurBit, pb + 3, 7, 1); 
		MovePos(&CurByte, &CurBit, 1); 
		if (q > 0) 
		{
            // Output remainder, q-bit
			int sh = 1; 
			sh <<= q; 
			out = GammaCode - sh; 
			pb = (BYTE*)&out; 
			InvertDWord(&out); 
			CopyBits(dest + CurByte, CurBit,  
				pb + (32 - q) / 8, (32 - q) % 8, q); 
			MovePos(&CurByte, &CurBit, q); 
		} 
	} 
	else  
	{ 
		DWORD dw = (DWORD)code; 
		BYTE* pb = (BYTE*)&dw; 
		InvertDWord(&dw); 
		CopyBits(dest + CurByte, CurBit,  
				pb + (32 - bits) / 8, (32 - bits) % 8, bits); 
		MovePos(&CurByte, &CurBit, bits); 
	} 
} 

/////////////////////////////////////////////
// Byte stream compression section
// Src - the source data area
// Srclen - source data region length in bytes
// Dest - compressed data area allocated srclen + 5 bytes of memory before the call
// Return value> 0 compressed data length
// Return value = 0 data can not be compressed
// Returns <0 Compression exception error
int CCompressKYModel::Compress(BYTE* src, int srclen, BYTE* dest)
{ 
	int i; 
	CurByte = 0; CurBit = 0;	 
	int off, len; 
	if (srclen > 65536)  
		return -1; 
	pWnd = src; 
	_InitSortTable(); 
	for (i = 0; i < srclen; i++) 
	{		 
		if (CurByte >= srclen) 
			return 0; 
		if (_SeekPhase(src, srclen, i, &off, &len)) 
		{
            // Output matching term flag (1bit) + len (γ coding) + offset (maximum 16bit)
			_OutCode(dest, 1, 1, FALSE); 
			_OutCode(dest, len, 0, TRUE);
            
            // When the window is less than 64k in size, does not require 16-bit storage shift
			_OutCode(dest, off, UpperLog2(nWndSize), FALSE); 
			_ScrollWindow(len); 
			i += len - 1; 
		} 
		else 
		{
            // Output single non-matching characters 0 (1bit) + char (8bit)
			_OutCode(dest, 0, 1, FALSE); 
			_OutCode(dest, (DWORD)(src[i]), 8, FALSE); 
			_ScrollWindow(1); 
		} 
	} 
	int destlen = CurByte + ((CurBit) ? 1 : 0); 
	if (destlen >= srclen) 
		return 0; 
	return destlen; 
}

/////////////////////////////////////////////
// Decompress some byte stream
// Src - receiving raw data memory area
// Srclen - source data region length in bytes
// Dest - compressed data area
// Return value - success
BOOL CCompressKYModel::Decompress(BYTE* src, int srclen, BYTE* dest, int destlen)
{ 
	int i; 
	CurByte = 0; CurBit = 0; 
	pWnd = src; // initialize window
	nWndSize = 0; 
	if (srclen > 65536)  
		return FALSE; 
	for (i = 0; i < srclen; i++) 
	{		 
		BYTE b = GetBit(dest[CurByte], CurBit); 
		MovePos(&CurByte, &CurBit, 1); 
		if (b == 0) // single character
		{ 
			CopyBits(src + i, 0, dest + CurByte, CurBit, 8); 
			MovePos(&CurByte, &CurBit, 8); 
			nWndSize++; 
		} 
		else // inside the window
		{ 
			int q = -1; 
			while (b != 0)
			{
				q++;
				b = GetBit(dest[CurByte], CurBit);
				MovePos(&CurByte, &CurBit, 1);
			}
			int len, off;
			DWORD dw = 0;
			BYTE* pb; 
			if (q > 0)
			{
				pb = (BYTE*)&dw;
				CopyBits(pb + (32 - q) / 8, (32 - q) % 8, dest + CurByte, CurBit, q);
				MovePos(&CurByte, &CurBit, q);
				InvertDWord(&dw);
				len = 1;
				len <<= q;
				len += dw;
				len += 1;
			}
			else
				len = 2;

			// When the window is less than 64k in size, does not require 16-bit storage shift
			dw = 0;
			pb = (BYTE*)&dw;
			int bits = UpperLog2(nWndSize);
			CopyBits(pb + (32 - bits) / 8, (32 - bits) % 8, dest + CurByte, CurBit, bits);
			MovePos(&CurByte, &CurBit, bits);
			InvertDWord(&dw); 
			off = (int)dw;
			// Output term
			for (int j = 0; j < len; j++)
			{
				//_ASSERT(i + j <  srclen);
				//_ASSERT(off + j <  _MAX_WINDOW_SIZE);
				src[i + j] = pWnd[off + j];
			}
			nWndSize += len;
			i += len - 1;
		}
		// Sliding window
		if (nWndSize > _MAX_WINDOW_SIZE)
		{
			pWnd += nWndSize - _MAX_WINDOW_SIZE;
			nWndSize = _MAX_WINDOW_SIZE;
		}
	}
	return TRUE;
}
