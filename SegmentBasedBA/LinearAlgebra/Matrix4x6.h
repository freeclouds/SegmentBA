/**
* This file is part of SegmentBA.
*
* Copyright (C) 2017 Zhejiang University
* For more information see <https://github.com/ZJUCVG/SegmentBA>
* If you use this code, please cite the corresponding publications as 
* listed on the above website.
*
* SegmentBA is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* SegmentBA is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with SegmentBA. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MATRIX_4x6_H_
#define _MATRIX_4x6_H_

#include "SSE.h"
#include "Matrix2x6.h"

namespace LA
{

	class AlignedCompactMatrix4x6f : public AlignedCompactMatrix2x6f
	{

	public:

		inline const  ENFT_SSE::__m128& M_20_21_22_23() const { return m_20_21_22_23; }	inline  ENFT_SSE::__m128& M_20_21_22_23() { return m_20_21_22_23; }
		inline const  ENFT_SSE::__m128& M_24_25_34_35() const { return m_24_25_34_35; }	inline  ENFT_SSE::__m128& M_24_25_34_35() { return m_24_25_34_35; }
		inline const  ENFT_SSE::__m128& M_30_31_32_33() const { return m_30_31_32_33; }	inline  ENFT_SSE::__m128& M_30_31_32_33() { return m_30_31_32_33; }
		inline const float& M20() const { return m_20_21_22_23.m128_f32[0]; }	inline float& M20() { return m_20_21_22_23.m128_f32[0]; }
		inline const float& M21() const { return m_20_21_22_23.m128_f32[1]; }	inline float& M21() { return m_20_21_22_23.m128_f32[1]; }
		inline const float& M22() const { return m_20_21_22_23.m128_f32[2]; }	inline float& M22() { return m_20_21_22_23.m128_f32[2]; }
		inline const float& M23() const { return m_20_21_22_23.m128_f32[3]; }	inline float& M23() { return m_20_21_22_23.m128_f32[3]; }
		inline const float& M24() const { return m_24_25_34_35.m128_f32[0]; }	inline float& M24() { return m_24_25_34_35.m128_f32[0]; }
		inline const float& M25() const { return m_24_25_34_35.m128_f32[1]; }	inline float& M25() { return m_24_25_34_35.m128_f32[1]; }
		inline const float& M30() const { return m_30_31_32_33.m128_f32[0]; }	inline float& M30() { return m_30_31_32_33.m128_f32[0]; }
		inline const float& M31() const { return m_30_31_32_33.m128_f32[1]; }	inline float& M31() { return m_30_31_32_33.m128_f32[1]; }
		inline const float& M32() const { return m_30_31_32_33.m128_f32[2]; }	inline float& M32() { return m_30_31_32_33.m128_f32[2]; }
		inline const float& M33() const { return m_30_31_32_33.m128_f32[3]; }	inline float& M33() { return m_30_31_32_33.m128_f32[3]; }
		inline const float& M34() const { return m_24_25_34_35.m128_f32[2]; }	inline float& M34() { return m_24_25_34_35.m128_f32[2]; }
		inline const float& M35() const { return m_24_25_34_35.m128_f32[3]; }	inline float& M35() { return m_24_25_34_35.m128_f32[3]; }
		inline void ConvertToConventionalStorage(float *work2)
		{
			AlignedCompactMatrix2x6f::ConvertToConventionalStorage(work2);
			memcpy(work2, &M34(), 8);
			memcpy(&M34(), &M30(), 16);
			memcpy(&M32(), work2, 8);
		}
		inline void ConvertToConventionalStorage(float *M) const
		{
			AlignedCompactMatrix2x6f::ConvertToConventionalStorage(M);
			memcpy(M + 12, &M20(), 24);
			memcpy(M + 18, &M30(), 16);
			memcpy(M + 22, &M34(), 8);
		}
		inline void ConvertToSpecialStorage(float *work2)
		{
			AlignedCompactMatrix2x6f::ConvertToSpecialStorage(work2);
			memcpy(work2, &M32(), 8);
			//memcpy(&M30(), &M34(), 16);
			memcpy(&M32(), &M30(), 8);
			memcpy(&M30(), &M34(), 8);
			memcpy(&M34(), work2, 8);
		}

	protected:

		 ENFT_SSE::__m128 m_20_21_22_23, m_24_25_34_35, m_30_31_32_33;

	};
}

#endif