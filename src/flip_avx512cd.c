/**
 * @file flip_avx512cd.c
 *
 * This module deals with flipping discs.
 *
 * For LSB to MSB directions, isolate LS1B can be used to determine
 * contiguous opponent discs.
 * For MSB to LSB directions, LZCNT is used.
 *
 * @date 1998 - 2023
 * @author Toshihiko Okuhara
 * @version 4.5
 */

#include "bit.h"

const V4DI lmask_v4[66] = {
	{{ 0x00000000000000fe, 0x0101010101010100, 0x8040201008040200, 0x0000000000000000 }},
	{{ 0x00000000000000fc, 0x0202020202020200, 0x0080402010080400, 0x0000000000000100 }},
	{{ 0x00000000000000f8, 0x0404040404040400, 0x0000804020100800, 0x0000000000010200 }},
	{{ 0x00000000000000f0, 0x0808080808080800, 0x0000008040201000, 0x0000000001020400 }},
	{{ 0x00000000000000e0, 0x1010101010101000, 0x0000000080402000, 0x0000000102040800 }},
	{{ 0x00000000000000c0, 0x2020202020202000, 0x0000000000804000, 0x0000010204081000 }},
	{{ 0x0000000000000080, 0x4040404040404000, 0x0000000000008000, 0x0001020408102000 }},
	{{ 0x0000000000000000, 0x8080808080808000, 0x0000000000000000, 0x0102040810204000 }},
	{{ 0x000000000000fe00, 0x0101010101010000, 0x4020100804020000, 0x0000000000000000 }},
	{{ 0x000000000000fc00, 0x0202020202020000, 0x8040201008040000, 0x0000000000010000 }},
	{{ 0x000000000000f800, 0x0404040404040000, 0x0080402010080000, 0x0000000001020000 }},
	{{ 0x000000000000f000, 0x0808080808080000, 0x0000804020100000, 0x0000000102040000 }},
	{{ 0x000000000000e000, 0x1010101010100000, 0x0000008040200000, 0x0000010204080000 }},
	{{ 0x000000000000c000, 0x2020202020200000, 0x0000000080400000, 0x0001020408100000 }},
	{{ 0x0000000000008000, 0x4040404040400000, 0x0000000000800000, 0x0102040810200000 }},
	{{ 0x0000000000000000, 0x8080808080800000, 0x0000000000000000, 0x0204081020400000 }},
	{{ 0x0000000000fe0000, 0x0101010101000000, 0x2010080402000000, 0x0000000000000000 }},
	{{ 0x0000000000fc0000, 0x0202020202000000, 0x4020100804000000, 0x0000000001000000 }},
	{{ 0x0000000000f80000, 0x0404040404000000, 0x8040201008000000, 0x0000000102000000 }},
	{{ 0x0000000000f00000, 0x0808080808000000, 0x0080402010000000, 0x0000010204000000 }},
	{{ 0x0000000000e00000, 0x1010101010000000, 0x0000804020000000, 0x0001020408000000 }},
	{{ 0x0000000000c00000, 0x2020202020000000, 0x0000008040000000, 0x0102040810000000 }},
	{{ 0x0000000000800000, 0x4040404040000000, 0x0000000080000000, 0x0204081020000000 }},
	{{ 0x0000000000000000, 0x8080808080000000, 0x0000000000000000, 0x0408102040000000 }},
	{{ 0x00000000fe000000, 0x0101010100000000, 0x1008040200000000, 0x0000000000000000 }},
	{{ 0x00000000fc000000, 0x0202020200000000, 0x2010080400000000, 0x0000000100000000 }},
	{{ 0x00000000f8000000, 0x0404040400000000, 0x4020100800000000, 0x0000010200000000 }},
	{{ 0x00000000f0000000, 0x0808080800000000, 0x8040201000000000, 0x0001020400000000 }},
	{{ 0x00000000e0000000, 0x1010101000000000, 0x0080402000000000, 0x0102040800000000 }},
	{{ 0x00000000c0000000, 0x2020202000000000, 0x0000804000000000, 0x0204081000000000 }},
	{{ 0x0000000080000000, 0x4040404000000000, 0x0000008000000000, 0x0408102000000000 }},
	{{ 0x0000000000000000, 0x8080808000000000, 0x0000000000000000, 0x0810204000000000 }},
	{{ 0x000000fe00000000, 0x0101010000000000, 0x0804020000000000, 0x0000000000000000 }},
	{{ 0x000000fc00000000, 0x0202020000000000, 0x1008040000000000, 0x0000010000000000 }},
	{{ 0x000000f800000000, 0x0404040000000000, 0x2010080000000000, 0x0001020000000000 }},
	{{ 0x000000f000000000, 0x0808080000000000, 0x4020100000000000, 0x0102040000000000 }},
	{{ 0x000000e000000000, 0x1010100000000000, 0x8040200000000000, 0x0204080000000000 }},
	{{ 0x000000c000000000, 0x2020200000000000, 0x0080400000000000, 0x0408100000000000 }},
	{{ 0x0000008000000000, 0x4040400000000000, 0x0000800000000000, 0x0810200000000000 }},
	{{ 0x0000000000000000, 0x8080800000000000, 0x0000000000000000, 0x1020400000000000 }},
	{{ 0x0000fe0000000000, 0x0101000000000000, 0x0402000000000000, 0x0000000000000000 }},
	{{ 0x0000fc0000000000, 0x0202000000000000, 0x0804000000000000, 0x0001000000000000 }},
	{{ 0x0000f80000000000, 0x0404000000000000, 0x1008000000000000, 0x0102000000000000 }},
	{{ 0x0000f00000000000, 0x0808000000000000, 0x2010000000000000, 0x0204000000000000 }},
	{{ 0x0000e00000000000, 0x1010000000000000, 0x4020000000000000, 0x0408000000000000 }},
	{{ 0x0000c00000000000, 0x2020000000000000, 0x8040000000000000, 0x0810000000000000 }},
	{{ 0x0000800000000000, 0x4040000000000000, 0x0080000000000000, 0x1020000000000000 }},
	{{ 0x0000000000000000, 0x8080000000000000, 0x0000000000000000, 0x2040000000000000 }},
	{{ 0x00fe000000000000, 0x0100000000000000, 0x0200000000000000, 0x0000000000000000 }},
	{{ 0x00fc000000000000, 0x0200000000000000, 0x0400000000000000, 0x0100000000000000 }},
	{{ 0x00f8000000000000, 0x0400000000000000, 0x0800000000000000, 0x0200000000000000 }},
	{{ 0x00f0000000000000, 0x0800000000000000, 0x1000000000000000, 0x0400000000000000 }},
	{{ 0x00e0000000000000, 0x1000000000000000, 0x2000000000000000, 0x0800000000000000 }},
	{{ 0x00c0000000000000, 0x2000000000000000, 0x4000000000000000, 0x1000000000000000 }},
	{{ 0x0080000000000000, 0x4000000000000000, 0x8000000000000000, 0x2000000000000000 }},
	{{ 0x0000000000000000, 0x8000000000000000, 0x0000000000000000, 0x4000000000000000 }},
	{{ 0xfe00000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0xfc00000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0xf800000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0xf000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0xe000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0xc000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x8000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},	// pass
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }}
};

const V4DI rmask_v4[66] = {
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000001, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000003, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000007, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x000000000000000f, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x000000000000001f, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x000000000000003f, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x000000000000007f, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000000001, 0x0000000000000000, 0x0000000000000002 }},
	{{ 0x0000000000000100, 0x0000000000000002, 0x0000000000000001, 0x0000000000000004 }},
	{{ 0x0000000000000300, 0x0000000000000004, 0x0000000000000002, 0x0000000000000008 }},
	{{ 0x0000000000000700, 0x0000000000000008, 0x0000000000000004, 0x0000000000000010 }},
	{{ 0x0000000000000f00, 0x0000000000000010, 0x0000000000000008, 0x0000000000000020 }},
	{{ 0x0000000000001f00, 0x0000000000000020, 0x0000000000000010, 0x0000000000000040 }},
	{{ 0x0000000000003f00, 0x0000000000000040, 0x0000000000000020, 0x0000000000000080 }},
	{{ 0x0000000000007f00, 0x0000000000000080, 0x0000000000000040, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000000101, 0x0000000000000000, 0x0000000000000204 }},
	{{ 0x0000000000010000, 0x0000000000000202, 0x0000000000000100, 0x0000000000000408 }},
	{{ 0x0000000000030000, 0x0000000000000404, 0x0000000000000201, 0x0000000000000810 }},
	{{ 0x0000000000070000, 0x0000000000000808, 0x0000000000000402, 0x0000000000001020 }},
	{{ 0x00000000000f0000, 0x0000000000001010, 0x0000000000000804, 0x0000000000002040 }},
	{{ 0x00000000001f0000, 0x0000000000002020, 0x0000000000001008, 0x0000000000004080 }},
	{{ 0x00000000003f0000, 0x0000000000004040, 0x0000000000002010, 0x0000000000008000 }},
	{{ 0x00000000007f0000, 0x0000000000008080, 0x0000000000004020, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000010101, 0x0000000000000000, 0x0000000000020408 }},
	{{ 0x0000000001000000, 0x0000000000020202, 0x0000000000010000, 0x0000000000040810 }},
	{{ 0x0000000003000000, 0x0000000000040404, 0x0000000000020100, 0x0000000000081020 }},
	{{ 0x0000000007000000, 0x0000000000080808, 0x0000000000040201, 0x0000000000102040 }},
	{{ 0x000000000f000000, 0x0000000000101010, 0x0000000000080402, 0x0000000000204080 }},
	{{ 0x000000001f000000, 0x0000000000202020, 0x0000000000100804, 0x0000000000408000 }},
	{{ 0x000000003f000000, 0x0000000000404040, 0x0000000000201008, 0x0000000000800000 }},
	{{ 0x000000007f000000, 0x0000000000808080, 0x0000000000402010, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000001010101, 0x0000000000000000, 0x0000000002040810 }},
	{{ 0x0000000100000000, 0x0000000002020202, 0x0000000001000000, 0x0000000004081020 }},
	{{ 0x0000000300000000, 0x0000000004040404, 0x0000000002010000, 0x0000000008102040 }},
	{{ 0x0000000700000000, 0x0000000008080808, 0x0000000004020100, 0x0000000010204080 }},
	{{ 0x0000000f00000000, 0x0000000010101010, 0x0000000008040201, 0x0000000020408000 }},
	{{ 0x0000001f00000000, 0x0000000020202020, 0x0000000010080402, 0x0000000040800000 }},
	{{ 0x0000003f00000000, 0x0000000040404040, 0x0000000020100804, 0x0000000080000000 }},
	{{ 0x0000007f00000000, 0x0000000080808080, 0x0000000040201008, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000101010101, 0x0000000000000000, 0x0000000204081020 }},
	{{ 0x0000010000000000, 0x0000000202020202, 0x0000000100000000, 0x0000000408102040 }},
	{{ 0x0000030000000000, 0x0000000404040404, 0x0000000201000000, 0x0000000810204080 }},
	{{ 0x0000070000000000, 0x0000000808080808, 0x0000000402010000, 0x0000001020408000 }},
	{{ 0x00000f0000000000, 0x0000001010101010, 0x0000000804020100, 0x0000002040800000 }},
	{{ 0x00001f0000000000, 0x0000002020202020, 0x0000001008040201, 0x0000004080000000 }},
	{{ 0x00003f0000000000, 0x0000004040404040, 0x0000002010080402, 0x0000008000000000 }},
	{{ 0x00007f0000000000, 0x0000008080808080, 0x0000004020100804, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000010101010101, 0x0000000000000000, 0x0000020408102040 }},
	{{ 0x0001000000000000, 0x0000020202020202, 0x0000010000000000, 0x0000040810204080 }},
	{{ 0x0003000000000000, 0x0000040404040404, 0x0000020100000000, 0x0000081020408000 }},
	{{ 0x0007000000000000, 0x0000080808080808, 0x0000040201000000, 0x0000102040800000 }},
	{{ 0x000f000000000000, 0x0000101010101010, 0x0000080402010000, 0x0000204080000000 }},
	{{ 0x001f000000000000, 0x0000202020202020, 0x0000100804020100, 0x0000408000000000 }},
	{{ 0x003f000000000000, 0x0000404040404040, 0x0000201008040201, 0x0000800000000000 }},
	{{ 0x007f000000000000, 0x0000808080808080, 0x0000402010080402, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0001010101010101, 0x0000000000000000, 0x0002040810204080 }},
	{{ 0x0100000000000000, 0x0002020202020202, 0x0001000000000000, 0x0004081020408000 }},
	{{ 0x0300000000000000, 0x0004040404040404, 0x0002010000000000, 0x0008102040800000 }},
	{{ 0x0700000000000000, 0x0008080808080808, 0x0004020100000000, 0x0010204080000000 }},
	{{ 0x0f00000000000000, 0x0010101010101010, 0x0008040201000000, 0x0020408000000000 }},
	{{ 0x1f00000000000000, 0x0020202020202020, 0x0010080402010000, 0x0040800000000000 }},
	{{ 0x3f00000000000000, 0x0040404040404040, 0x0020100804020100, 0x0080000000000000 }},
	{{ 0x7f00000000000000, 0x0080808080808080, 0x0040201008040201, 0x0000000000000000 }},
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }},	// pass
	{{ 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 }}
};

/**
 * Compute flipped discs when playing on square pos.
 *
 * @param pos player's move.
 * @param P player's disc pattern.
 * @param O opponent's disc pattern.
 * @return flipped disc pattern.
 */

__m128i vectorcall mm_Flip(const __m128i OP, int pos)
{
	__m256i	PP, OO, flip, outflank, mask;
	__m128i	flip2;

	PP = _mm256_broadcastq_epi64(OP);
	OO = _mm256_permute4x64_epi64(_mm256_castsi128_si256(OP), 0x55);

	mask = rmask_v4[pos].v4;
		// right: look for non-opponent (or edge) bit with lzcnt
	outflank = _mm256_andnot_si256(OO, mask);
	outflank = _mm256_srlv_epi64(_mm256_set1_epi64x(0x8000000000000000), _mm256_lzcnt_epi64(outflank));
	outflank = _mm256_and_si256(outflank, PP);
		// set all bits higher than outflank
#if 0 // use 0
	// flip = _mm256_and_si256(_mm256_xor_si256(_mm256_sub_epi64(_mm256_setzero_si256(), outflank), outflank), mask);
	flip = _mm256_ternarylogic_epi64(_mm256_sub_epi64(_mm256_setzero_si256(), outflank), outflank, mask, 0x28);
#else // use -1
	// flip = _mm256_andnot_si256(_mm256_or_si256(_mm256_add_epi64(outflank, _mm256_set1_epi64x(-1)), outflank), mask);
	flip = _mm256_ternarylogic_epi64(_mm256_add_epi64(outflank, _mm256_set1_epi64x(-1)), outflank, mask, 0x02);
#endif

	mask = lmask_v4[pos].v4;
		// left: look for non-opponent LS1B
	outflank = _mm256_andnot_si256(OO, mask);
#if 0 // cmpeq
	// outflank = _mm256_and_si256(outflank, _mm256_sub_epi64(_mm256_setzero_si256(), outflank));	// LS1B
	// outflank = _mm256_and_si256(outflank, PP);
	outflank = _mm256_ternarylogic_epi64(_mm256_sub_epi64(_mm256_setzero_si256(), outflank), outflank, PP, 0x80);
		// set all bits if outflank = 0, otherwise higher bits than outflank
	outflank = _mm256_sub_epi64(_mm256_cmpeq_epi64(outflank, _mm256_setzero_si256()), outflank);
	// flip = _mm256_or_si256(flip, _mm256_andnot_si256(outflank, mask));
	flip = _mm256_ternarylogic_epi64(flip, outflank, mask, 0xf2);
#else // test_mask
	// outflank = _mm256_xor_si256(outflank, _mm256_add_epi64(outflank, _mm256_set1_epi64x(-1)));	// BLSMSK
	// outflank = _mm256_and_si256(outflank, mask);	// non-opponent LS1B and opponent inbetween
	outflank = _mm256_ternarylogic_epi64(outflank, _mm256_add_epi64(outflank, _mm256_set1_epi64x(-1)), mask, 0x28);
		// apply flip if P is in BLSMSK, i.e. LS1B is P
	// flip = _mm256_mask_or_epi64(flip, _mm256_test_epi64_mask(outflank, PP), flip, _mm256_and_si256(outflank, OO));
	flip = _mm256_mask_ternarylogic_epi64(flip, _mm256_test_epi64_mask(outflank, PP), outflank, OO, 0xf8);
#endif

	flip2 = _mm_or_si128(_mm256_castsi256_si128(flip), _mm256_extracti128_si256(flip, 1));
	flip2 = _mm_or_si128(flip2, _mm_shuffle_epi32(flip2, 0x4e));	// SWAP64

	return flip2;
}
