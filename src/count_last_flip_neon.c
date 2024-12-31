/**
 * @file count_last_flip_neon.c
 *
 *
 * A function is provided to count the number of fipped disc of the last move.
 *
 * The basic principle is to read into an array a precomputed result. Doing
 * this is easy for a single line ; as we can use arrays of the form:
 *  - COUNT_FLIP[square where we play][8-bits disc pattern].
 * The problem is thus to convert any line of a 64-bits disc pattern into an
 * 8-bits disc pattern. A fast way to do this is to select the right line,
 * with a bit-mask, to gather the masked-bits into a continuous set by the 
 * neon vaddvq_u16 instruction.
 * Once we get our 8-bits disc patterns, we directly get the number of
 * flipped discs from the precomputed array, and add them from each flipping
 * lines.
 * For optimization purpose, the value returned is twice the number of flipped
 * disc, to facilitate the computation of disc difference.
 *
 * @date 1998 - 2023
 * @author Richard Delorme
 * @author Toshihiko Okuhara
 * @version 4.5
 * 
 */

#include <arm_neon.h>

/** precomputed count flip array */
const unsigned char COUNT_FLIP[8][256] = {
	{
		 0,  0,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		 8,  8,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		10, 10,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		 8,  8,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		12, 12,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		 8,  8,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		10, 10,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
		 8,  8,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,  6,  6,  0,  0,  2,  2,  0,  0,  4,  4,  0,  0,  2,  2,  0,  0,
	},
	{
		 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 6,  6,  6,  6,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 8,  8,  8,  8,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 6,  6,  6,  6,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		10, 10, 10, 10,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 6,  6,  6,  6,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 8,  8,  8,  8,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
		 6,  6,  6,  6,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,
	},
	{
		 0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 4,  6,  4,  4,  4,  6,  4,  4,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 6,  8,  6,  6,  6,  8,  6,  6,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 4,  6,  4,  4,  4,  6,  4,  4,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 8, 10,  8,  8,  8, 10,  8,  8,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 4,  6,  4,  4,  4,  6,  4,  4,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 6,  8,  6,  6,  6,  8,  6,  6,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
		 4,  6,  4,  4,  4,  6,  4,  4,  0,  2,  0,  0,  0,  2,  0,  0,  2,  4,  2,  2,  2,  4,  2,  2,  0,  2,  0,  0,  0,  2,  0,  0,
	},
	{
		 0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 2,  6,  4,  4,  2,  2,  2,  2,  2,  6,  4,  4,  2,  2,  2,  2,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 4,  8,  6,  6,  4,  4,  4,  4,  4,  8,  6,  6,  4,  4,  4,  4,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 2,  6,  4,  4,  2,  2,  2,  2,  2,  6,  4,  4,  2,  2,  2,  2,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 6, 10,  8,  8,  6,  6,  6,  6,  6, 10,  8,  8,  6,  6,  6,  6,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 2,  6,  4,  4,  2,  2,  2,  2,  2,  6,  4,  4,  2,  2,  2,  2,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 4,  8,  6,  6,  4,  4,  4,  4,  4,  8,  6,  6,  4,  4,  4,  4,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
		 2,  6,  4,  4,  2,  2,  2,  2,  2,  6,  4,  4,  2,  2,  2,  2,  0,  4,  2,  2,  0,  0,  0,  0,  0,  4,  2,  2,  0,  0,  0,  0,
	},
	{
		 0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,
		 2,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,
		 4, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  4, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,
		 0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,
		 2,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  4,  4,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,
	},
	{
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 2, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 2, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  8,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
	{
		 0, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 10,  8,  8,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
	{
		 0, 12, 10, 10,  8,  8,  8,  8,  6,  6,  6,  6,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 12, 10, 10,  8,  8,  8,  8,  6,  6,  6,  6,  6,  6,  6,  6,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
};

#ifdef HAS_CPU_64
/* bit masks for diagonal lines (interleaved) */
const uint64x2_t mask_dvhd[64][2] = {
	{{ 0x000000000000ff01, 0x0000000000000000 }, { 0x0801040102010101, 0x8001400120011001 }},
	{{ 0x000000000001ff02, 0x0000000000000000 }, { 0x1002080204020202, 0x0002800240022002 }},
	{{ 0x000000010002ff04, 0x0000000000000000 }, { 0x2004100408040404, 0x0004000480044004 }},
	{{ 0x000100020004ff08, 0x0000000000000000 }, { 0x4008200810080808, 0x0008000800088008 }},
	{{ 0x000200040008ff10, 0x0000000000000001 }, { 0x8010401020101010, 0x0010001000100010 }},
	{{ 0x000400080010ff20, 0x0000000000010002 }, { 0x0020802040202020, 0x0020002000200020 }},
	{{ 0x000800100020ff40, 0x0000000100020004 }, { 0x0040004080404040, 0x0040004000400040 }},
	{{ 0x001000200040ff80, 0x0001000200040008 }, { 0x0080008000808080, 0x0080008000800080 }},
	{{ 0x00000000ff010002, 0x0000000000000000 }, { 0x0401020101010001, 0x4001200110010801 }},
	{{ 0x00000001ff020004, 0x0000000000000000 }, { 0x0802040202020102, 0x8002400220021002 }},
	{{ 0x00010002ff040008, 0x0000000000000000 }, { 0x1004080404040204, 0x0004800440042004 }},
	{{ 0x00020004ff080010, 0x0000000000000001 }, { 0x2008100808080408, 0x0008000880084008 }},
	{{ 0x00040008ff100020, 0x0000000000010002 }, { 0x4010201010100810, 0x0010001000108010 }},
	{{ 0x00080010ff200040, 0x0000000100020004 }, { 0x8020402020201020, 0x0020002000200020 }},
	{{ 0x00100020ff400080, 0x0001000200040008 }, { 0x0040804040402040, 0x0040004000400040 }},
	{{ 0x00200040ff800000, 0x0002000400080010 }, { 0x0080008080804080, 0x0080008000800080 }},
	{{ 0x0000ff0100020004, 0x0000000000000000 }, { 0x0201010100010001, 0x2001100108010401 }},
	{{ 0x0001ff0200040008, 0x0000000000000000 }, { 0x0402020201020002, 0x4002200210020802 }},
	{{ 0x0002ff0400080010, 0x0000000000000001 }, { 0x0804040402040104, 0x8004400420041004 }},
	{{ 0x0004ff0800100020, 0x0000000000010002 }, { 0x1008080804080208, 0x0008800840082008 }},
	{{ 0x0008ff1000200040, 0x0000000100020004 }, { 0x2010101008100410, 0x0010001080104010 }},
	{{ 0x0010ff2000400080, 0x0001000200040008 }, { 0x4020202010200820, 0x0020002000208020 }},
	{{ 0x0020ff4000800000, 0x0002000400080010 }, { 0x8040404020401040, 0x0040004000400040 }},
	{{ 0x0040ff8000000000, 0x0004000800100020 }, { 0x0080808040802080, 0x0080008000800080 }},
	{{ 0xff01000200040008, 0x0000000000000000 }, { 0x0101000100010001, 0x1001080104010201 }},
	{{ 0xff02000400080010, 0x0000000000000001 }, { 0x0202010200020002, 0x2002100208020402 }},
	{{ 0xff04000800100020, 0x0000000000010002 }, { 0x0404020401040004, 0x4004200410040804 }},
	{{ 0xff08001000200040, 0x0000000100020004 }, { 0x0808040802080108, 0x8008400820081008 }},
	{{ 0xff10002000400080, 0x0001000200040008 }, { 0x1010081004100210, 0x0010801040102010 }},
	{{ 0xff20004000800000, 0x0002000400080010 }, { 0x2020102008200420, 0x0020002080204020 }},
	{{ 0xff40008000000000, 0x0004000800100020 }, { 0x4040204010400840, 0x0040004000408040 }},
	{{ 0xff80000000000000, 0x0008001000200040 }, { 0x8080408020801080, 0x0080008000800080 }},
	{{ 0x0002000400080010, 0x000000000000ff01 }, { 0x0001000100010001, 0x0801040102010101 }},
	{{ 0x0004000800100020, 0x000000000001ff02 }, { 0x0102000200020002, 0x1002080204020202 }},
	{{ 0x0008001000200040, 0x000000010002ff04 }, { 0x0204010400040004, 0x2004100408040404 }},
	{{ 0x0010002000400080, 0x000100020004ff08 }, { 0x0408020801080008, 0x4008200810080808 }},
	{{ 0x0020004000800000, 0x000200040008ff10 }, { 0x0810041002100110, 0x8010401020101010 }},
	{{ 0x0040008000000000, 0x000400080010ff20 }, { 0x1020082004200220, 0x0020802040202020 }},
	{{ 0x0080000000000000, 0x000800100020ff40 }, { 0x2040104008400440, 0x0040004080404040 }},
	{{ 0x0000000000000000, 0x001000200040ff80 }, { 0x4080208010800880, 0x0080008000808080 }},
	{{ 0x0004000800100020, 0x00000000ff010002 }, { 0x0001000100010001, 0x0401020101010001 }},
	{{ 0x0008001000200040, 0x00000001ff020004 }, { 0x0002000200020002, 0x0802040202020102 }},
	{{ 0x0010002000400080, 0x00010002ff040008 }, { 0x0104000400040004, 0x1004080404040204 }},
	{{ 0x0020004000800000, 0x00020004ff080010 }, { 0x0208010800080008, 0x2008100808080408 }},
	{{ 0x0040008000000000, 0x00040008ff100020 }, { 0x0410021001100010, 0x4010201010100810 }},
	{{ 0x0080000000000000, 0x00080010ff200040 }, { 0x0820042002200120, 0x8020402020201020 }},
	{{ 0x0000000000000000, 0x00100020ff400080 }, { 0x1040084004400240, 0x0040804040402040 }},
	{{ 0x0000000000000000, 0x00200040ff800000 }, { 0x2080108008800480, 0x0080008080804080 }},
	{{ 0x0008001000200040, 0x0000ff0100020004 }, { 0x0001000100010001, 0x0201010100010001 }},
	{{ 0x0010002000400080, 0x0001ff0200040008 }, { 0x0002000200020002, 0x0402020201020002 }},
	{{ 0x0020004000800000, 0x0002ff0400080010 }, { 0x0004000400040004, 0x0804040402040104 }},
	{{ 0x0040008000000000, 0x0004ff0800100020 }, { 0x0108000800080008, 0x1008080804080208 }},
	{{ 0x0080000000000000, 0x0008ff1000200040 }, { 0x0210011000100010, 0x2010101008100410 }},
	{{ 0x0000000000000000, 0x0010ff2000400080 }, { 0x0420022001200020, 0x4020202010200820 }},
	{{ 0x0000000000000000, 0x0020ff4000800000 }, { 0x0840044002400140, 0x8040404020401040 }},
	{{ 0x0000000000000000, 0x0040ff8000000000 }, { 0x1080088004800280, 0x0080808040802080 }},
	{{ 0x0010002000400080, 0xff01000200040008 }, { 0x0001000100010001, 0x0101000100010001 }},
	{{ 0x0020004000800000, 0xff02000400080010 }, { 0x0002000200020002, 0x0202010200020002 }},
	{{ 0x0040008000000000, 0xff04000800100020 }, { 0x0004000400040004, 0x0404020401040004 }},
	{{ 0x0080000000000000, 0xff08001000200040 }, { 0x0008000800080008, 0x0808040802080108 }},
	{{ 0x0000000000000000, 0xff10002000400080 }, { 0x0110001000100010, 0x1010081004100210 }},
	{{ 0x0000000000000000, 0xff20004000800000 }, { 0x0220012000200020, 0x2020102008200420 }},
	{{ 0x0000000000000000, 0xff40008000000000 }, { 0x0440024001400040, 0x4040204010400840 }},
	{{ 0x0000000000000000, 0xff80000000000000 }, { 0x0880048002800180, 0x8080408020801080 }}
};
#else
/* bit masks for diagonal lines */
const uint64x2_t mask_dvhd[64][2] = {
	{{ 0x0000000000000001, 0x00000000000000ff }, { 0x0101010101010101, 0x8040201008040201 }},
	{{ 0x0000000000000102, 0x00000000000000ff }, { 0x0202020202020202, 0x0080402010080402 }},
	{{ 0x0000000000010204, 0x00000000000000ff }, { 0x0404040404040404, 0x0000804020100804 }},
	{{ 0x0000000001020408, 0x00000000000000ff }, { 0x0808080808080808, 0x0000008040201008 }},
	{{ 0x0000000102040810, 0x00000000000000ff }, { 0x1010101010101010, 0x0000000080402010 }},
	{{ 0x0000010204081020, 0x00000000000000ff }, { 0x2020202020202020, 0x0000000000804020 }},
	{{ 0x0001020408102040, 0x00000000000000ff }, { 0x4040404040404040, 0x0000000000008040 }},
	{{ 0x0102040810204080, 0x00000000000000ff }, { 0x8080808080808080, 0x0000000000000080 }},
	{{ 0x0000000000000102, 0x000000000000ff00 }, { 0x0101010101010101, 0x4020100804020100 }},
	{{ 0x0000000000010204, 0x000000000000ff00 }, { 0x0202020202020202, 0x8040201008040201 }},
	{{ 0x0000000001020408, 0x000000000000ff00 }, { 0x0404040404040404, 0x0080402010080402 }},
	{{ 0x0000000102040810, 0x000000000000ff00 }, { 0x0808080808080808, 0x0000804020100804 }},
	{{ 0x0000010204081020, 0x000000000000ff00 }, { 0x1010101010101010, 0x0000008040201008 }},
	{{ 0x0001020408102040, 0x000000000000ff00 }, { 0x2020202020202020, 0x0000000080402010 }},
	{{ 0x0102040810204080, 0x000000000000ff00 }, { 0x4040404040404040, 0x0000000000804020 }},
	{{ 0x0204081020408000, 0x000000000000ff00 }, { 0x8080808080808080, 0x0000000000008040 }},
	{{ 0x0000000000010204, 0x0000000000ff0000 }, { 0x0101010101010101, 0x2010080402010000 }},
	{{ 0x0000000001020408, 0x0000000000ff0000 }, { 0x0202020202020202, 0x4020100804020100 }},
	{{ 0x0000000102040810, 0x0000000000ff0000 }, { 0x0404040404040404, 0x8040201008040201 }},
	{{ 0x0000010204081020, 0x0000000000ff0000 }, { 0x0808080808080808, 0x0080402010080402 }},
	{{ 0x0001020408102040, 0x0000000000ff0000 }, { 0x1010101010101010, 0x0000804020100804 }},
	{{ 0x0102040810204080, 0x0000000000ff0000 }, { 0x2020202020202020, 0x0000008040201008 }},
	{{ 0x0204081020408000, 0x0000000000ff0000 }, { 0x4040404040404040, 0x0000000080402010 }},
	{{ 0x0408102040800000, 0x0000000000ff0000 }, { 0x8080808080808080, 0x0000000000804020 }},
	{{ 0x0000000001020408, 0x00000000ff000000 }, { 0x0101010101010101, 0x1008040201000000 }},
	{{ 0x0000000102040810, 0x00000000ff000000 }, { 0x0202020202020202, 0x2010080402010000 }},
	{{ 0x0000010204081020, 0x00000000ff000000 }, { 0x0404040404040404, 0x4020100804020100 }},
	{{ 0x0001020408102040, 0x00000000ff000000 }, { 0x0808080808080808, 0x8040201008040201 }},
	{{ 0x0102040810204080, 0x00000000ff000000 }, { 0x1010101010101010, 0x0080402010080402 }},
	{{ 0x0204081020408000, 0x00000000ff000000 }, { 0x2020202020202020, 0x0000804020100804 }},
	{{ 0x0408102040800000, 0x00000000ff000000 }, { 0x4040404040404040, 0x0000008040201008 }},
	{{ 0x0810204080000000, 0x00000000ff000000 }, { 0x8080808080808080, 0x0000000080402010 }},
	{{ 0x0000000102040810, 0x000000ff00000000 }, { 0x0101010101010101, 0x0804020100000000 }},
	{{ 0x0000010204081020, 0x000000ff00000000 }, { 0x0202020202020202, 0x1008040201000000 }},
	{{ 0x0001020408102040, 0x000000ff00000000 }, { 0x0404040404040404, 0x2010080402010000 }},
	{{ 0x0102040810204080, 0x000000ff00000000 }, { 0x0808080808080808, 0x4020100804020100 }},
	{{ 0x0204081020408000, 0x000000ff00000000 }, { 0x1010101010101010, 0x8040201008040201 }},
	{{ 0x0408102040800000, 0x000000ff00000000 }, { 0x2020202020202020, 0x0080402010080402 }},
	{{ 0x0810204080000000, 0x000000ff00000000 }, { 0x4040404040404040, 0x0000804020100804 }},
	{{ 0x1020408000000000, 0x000000ff00000000 }, { 0x8080808080808080, 0x0000008040201008 }},
	{{ 0x0000010204081020, 0x0000ff0000000000 }, { 0x0101010101010101, 0x0402010000000000 }},
	{{ 0x0001020408102040, 0x0000ff0000000000 }, { 0x0202020202020202, 0x0804020100000000 }},
	{{ 0x0102040810204080, 0x0000ff0000000000 }, { 0x0404040404040404, 0x1008040201000000 }},
	{{ 0x0204081020408000, 0x0000ff0000000000 }, { 0x0808080808080808, 0x2010080402010000 }},
	{{ 0x0408102040800000, 0x0000ff0000000000 }, { 0x1010101010101010, 0x4020100804020100 }},
	{{ 0x0810204080000000, 0x0000ff0000000000 }, { 0x2020202020202020, 0x8040201008040201 }},
	{{ 0x1020408000000000, 0x0000ff0000000000 }, { 0x4040404040404040, 0x0080402010080402 }},
	{{ 0x2040800000000000, 0x0000ff0000000000 }, { 0x8080808080808080, 0x0000804020100804 }},
	{{ 0x0001020408102040, 0x00ff000000000000 }, { 0x0101010101010101, 0x0201000000000000 }},
	{{ 0x0102040810204080, 0x00ff000000000000 }, { 0x0202020202020202, 0x0402010000000000 }},
	{{ 0x0204081020408000, 0x00ff000000000000 }, { 0x0404040404040404, 0x0804020100000000 }},
	{{ 0x0408102040800000, 0x00ff000000000000 }, { 0x0808080808080808, 0x1008040201000000 }},
	{{ 0x0810204080000000, 0x00ff000000000000 }, { 0x1010101010101010, 0x2010080402010000 }},
	{{ 0x1020408000000000, 0x00ff000000000000 }, { 0x2020202020202020, 0x4020100804020100 }},
	{{ 0x2040800000000000, 0x00ff000000000000 }, { 0x4040404040404040, 0x8040201008040201 }},
	{{ 0x4080000000000000, 0x00ff000000000000 }, { 0x8080808080808080, 0x0080402010080402 }},
	{{ 0x0102040810204080, 0xff00000000000000 }, { 0x0101010101010101, 0x0100000000000000 }},
	{{ 0x0204081020408000, 0xff00000000000000 }, { 0x0202020202020202, 0x0201000000000000 }},
	{{ 0x0408102040800000, 0xff00000000000000 }, { 0x0404040404040404, 0x0402010000000000 }},
	{{ 0x0810204080000000, 0xff00000000000000 }, { 0x0808080808080808, 0x0804020100000000 }},
	{{ 0x1020408000000000, 0xff00000000000000 }, { 0x1010101010101010, 0x1008040201000000 }},
	{{ 0x2040800000000000, 0xff00000000000000 }, { 0x2020202020202020, 0x2010080402010000 }},
	{{ 0x4080000000000000, 0xff00000000000000 }, { 0x4040404040404040, 0x4020100804020100 }},
	{{ 0x8000000000000000, 0xff00000000000000 }, { 0x8080808080808080, 0x8040201008040201 }}
};
#endif

/**
 * Count last flipped discs when playing on the last empty.
 *
 * @param pos the last empty square.
 * @param P player's disc pattern.
 * @return flipped disc count.
 */

int last_flip(int pos, unsigned long long P)
{
	unsigned int	n_flips;
	const unsigned char *COUNT_FLIP_X = COUNT_FLIP[pos & 7];
	const unsigned char *COUNT_FLIP_Y = COUNT_FLIP[pos >> 3];
	uint64x2_t	PP = vdupq_n_u64(P);
	uint64x2_t	II;
#ifdef HAS_CPU_64	// vaddvq
	unsigned int t;
	const uint64x2_t dmask = { 0x0808040402020101, 0x8080404020201010 };

	PP = vreinterpretq_u64_u8(vzip1q_u8(vreinterpretq_u8_u64(PP), vreinterpretq_u8_u64(PP)));
	II = vandq_u64(PP, mask_dvhd[pos][0]);	// 2 dirs interleaved
	t = vaddvq_u16(vreinterpretq_u16_u64(II));
	n_flips  = COUNT_FLIP_X[t >> 8];
	n_flips += COUNT_FLIP_X[t & 0xFF];
	II = vandq_u64(vreinterpretq_u64_u8(vtstq_u8(vreinterpretq_u8_u64(PP), vreinterpretq_u8_u64(mask_dvhd[pos][1]))), dmask);
	t = vaddvq_u16(vreinterpretq_u16_u64(II));
	n_flips += COUNT_FLIP_Y[t >> 8];
	n_flips += COUNT_FLIP_Y[t & 0xFF];

#else // Neon kindergarten
	const uint64x2_t dmask = { 0x1020408001020408, 0x1020408001020408 };

	II = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(vreinterpretq_u8_u64(vandq_u64(PP, mask_dvhd[pos][0])))));
	n_flips  = COUNT_FLIP_X[vgetq_lane_u32(vreinterpretq_u32_u64(II), 2)];
	n_flips += COUNT_FLIP_X[vgetq_lane_u32(vreinterpretq_u32_u64(II), 0)];
	II = vreinterpretq_u64_s8(vnegq_s8(vreinterpretq_s8_u8(vtstq_u8(vreinterpretq_u8_u64(PP), vreinterpretq_u8_u64(mask_dvhd[pos][1])))));
	II = vpaddlq_u32(vmulq_u32(vreinterpretq_u32_u64(dmask), vreinterpretq_u32_u64(II)));
	n_flips += COUNT_FLIP_Y[vgetq_lane_u8(vreinterpretq_u8_u64(II), 11)];
	n_flips += COUNT_FLIP_Y[vgetq_lane_u8(vreinterpretq_u8_u64(II), 3)];
#endif
	return n_flips;
}

/**
 * @brief Get the final score.
 *
 * Get the final score, when 1 empty square remain.
 * The following code has been adapted from Zebra by Gunnar Anderson.
 *
 * @param player Board.player to evaluate.
 * @param alpha  Alpha bound. (beta - 1)
 * @param x      Last empty square to play.
 * @return       The final score, as a disc difference.
 */
int board_score_1(const unsigned long long player, const int alpha, const int x)
{
	int score, score2, n_flips;

	score = 2 * bit_count(player) - SCORE_MAX + 2;	// = (bit_count(P) + 1) - (SCORE_MAX - 1 - bit_count(P))

	n_flips = last_flip(x, player);
	score += n_flips;

	if (n_flips == 0) {	// (23%)
		score2 = score - 2;	// empty for opponent
		if (score <= 0)
			score = score2;
		if (score > alpha) {	// lazy cut-off (40%)
			if ((n_flips = last_flip(x, ~player)) != 0)	// (98%)
				score = score2 - n_flips;
		}
	}

	return score;
}
