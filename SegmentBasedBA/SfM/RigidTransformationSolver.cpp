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

#include "stdafx.h"
#include "RigidTransformationSolver.h"
#undef small
#ifdef __cplusplus
extern "C" {
#endif
#include <f2c.h>
#include <clapack.h>
#ifdef __cplusplus
}
#endif

bool RigidTransformationSolver::Run(const ThreeMatches3D &data, RigidTransformation3D &T, AlignedVector< ENFT_SSE::__m128> &work)
{
	ComputeMeanAndCovariance(data, work);
	return RecoverFromMeanAndCovariance(T, work);
}

bool RigidTransformationSolver::Run(const SixMatches3D &data, RigidTransformation3D &T, AlignedVector< ENFT_SSE::__m128> &work)
{
	ComputeMeanAndCovariance(data, work);
	return RecoverFromMeanAndCovariance(T, work);
}

bool RigidTransformationSolver::Run(const EightMatches3D &data, RigidTransformation3D &T, AlignedVector< ENFT_SSE::__m128> &work)
{
	ComputeMeanAndCovariance(data, work);
	return RecoverFromMeanAndCovariance(T, work);
}

bool RigidTransformationSolver::Run(const AlignedVector<Point3D> &X1s, const AlignedVector<Point3D> &X2s, RigidTransformation3D &T, AlignedVector< ENFT_SSE::__m128> &work)
{
	ComputeMeanAndCovariance(X1s, X2s, work);
	return RecoverFromMeanAndCovariance(T, work);
}

//bool RigidTransformationSolver::Run(const AlignedVector<Point3D> &X1s, const AlignedVector<Point3D> &X2s, RigidTransformation3D &T, RigidTransformation3D &TAprior, 
//									const LA::AlignedVector3f &center, AlignedVector< ENFT_SSE::__m128> &work)
//{
//	ComputeMeanAndCovariance(X1s, X2s, work);
//	return RecoverFromMeanAndCovariance(T, TAprior, center, work);
//}

void RigidTransformationSolver::ComputeMeanAndCovariance(const ThreeMatches3D &data, AlignedVector< ENFT_SSE::__m128> &work)
{
#if _DEBUG
	assert(data.X1(0).reserve() == 1 && data.X1(1).reserve() == 1 && data.X1(2).reserve() == 1);
#endif

	work.Resize(6);
	work[0] =  ENFT_SSE::_mm_set1_ps(1.0f / 3);
	m_mean1.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps(data.X1(0).XYZx(),  ENFT_SSE::_mm_add_ps(data.X1(1).XYZx(), data.X1(2).XYZx())), work[0]);
	m_mean2.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps(data.X2(0).XYZx(),  ENFT_SSE::_mm_add_ps(data.X2(1).XYZx(), data.X2(2).XYZx())), work[0]);

	 ENFT_SSE::__m128 *dX1s = work.Data(), *dX2s = work.Data() + 3;
	dX1s[0] =  ENFT_SSE::_mm_sub_ps(data.X1(0).XYZx(), m_mean1.XYZx());	dX1s[1] =  ENFT_SSE::_mm_sub_ps(data.X1(1).XYZx(), m_mean1.XYZx());	dX1s[2] =  ENFT_SSE::_mm_sub_ps(data.X1(2).XYZx(), m_mean1.XYZx());
	dX2s[0] =  ENFT_SSE::_mm_sub_ps(data.X2(0).XYZx(), m_mean2.XYZx());	dX2s[1] =  ENFT_SSE::_mm_sub_ps(data.X2(1).XYZx(), m_mean2.XYZx());	dX2s[2] =  ENFT_SSE::_mm_sub_ps(data.X2(2).XYZx(), m_mean2.XYZx());
	m_C.M_00_01_02_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[0],  ENFT_SSE::_mm_set1_ps(dX2s[0].m128_f32[0])), 
						  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[1],  ENFT_SSE::_mm_set1_ps(dX2s[1].m128_f32[0])),  ENFT_SSE::_mm_mul_ps(dX1s[2],  ENFT_SSE::_mm_set1_ps(dX2s[2].m128_f32[0]))));
	m_C.M_10_11_12_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[0],  ENFT_SSE::_mm_set1_ps(dX2s[0].m128_f32[1])), 
						  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[1],  ENFT_SSE::_mm_set1_ps(dX2s[1].m128_f32[1])),  ENFT_SSE::_mm_mul_ps(dX1s[2],  ENFT_SSE::_mm_set1_ps(dX2s[2].m128_f32[1]))));
	m_C.M_20_21_22_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[0],  ENFT_SSE::_mm_set1_ps(dX2s[0].m128_f32[2])), 
						  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1s[1],  ENFT_SSE::_mm_set1_ps(dX2s[1].m128_f32[2])),  ENFT_SSE::_mm_mul_ps(dX1s[2],  ENFT_SSE::_mm_set1_ps(dX2s[2].m128_f32[2]))));
}

void RigidTransformationSolver::ComputeMeanAndCovariance(const SixMatches3D &data, AlignedVector< ENFT_SSE::__m128> &work)
{
#if _DEBUG
	assert(data.X1(0).reserve() == 1 && data.X1(1).reserve() == 1 && data.X1(2).reserve() == 1);
	assert(data.X1(3).reserve() == 1 && data.X1(4).reserve() == 1 && data.X1(5).reserve() == 1);
#endif

	work.Resize(2);
	work[0] =  ENFT_SSE::_mm_set1_ps(1.0f / 6);
	m_mean1.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X1(0).XYZx(), data.X1(1).XYZx()), 
								 ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X1(2).XYZx(), data.X1(3).XYZx()),  ENFT_SSE::_mm_add_ps(data.X1(4).XYZx(), data.X1(5).XYZx()))), work[0]);
	m_mean2.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X2(0).XYZx(), data.X2(1).XYZx()), 
								 ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X2(2).XYZx(), data.X2(3).XYZx()),  ENFT_SSE::_mm_add_ps(data.X2(4).XYZx(), data.X2(5).XYZx()))), work[0]);

	 ENFT_SSE::__m128 &dX1 = work[0], &dX2 = work[1];
	m_C.SetZero();

#ifndef ACCUMULATE_COVARIANCE_6
#define ACCUMULATE_COVARIANCE_6(i)\
	dX1 =  ENFT_SSE::_mm_sub_ps(data.X1(i).XYZx(), m_mean1.XYZx());\
	dX2 =  ENFT_SSE::_mm_sub_ps(data.X2(i).XYZx(), m_mean2.XYZx());\
	m_C.M_00_01_02_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[0])), m_C.M_00_01_02_x());\
	m_C.M_10_11_12_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[1])), m_C.M_10_11_12_x());\
	m_C.M_20_21_22_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[2])), m_C.M_20_21_22_x())
#endif

	ACCUMULATE_COVARIANCE_6(0);	ACCUMULATE_COVARIANCE_6(1);	ACCUMULATE_COVARIANCE_6(2);
	ACCUMULATE_COVARIANCE_6(3);	ACCUMULATE_COVARIANCE_6(4);	ACCUMULATE_COVARIANCE_6(5);
}

void RigidTransformationSolver::ComputeMeanAndCovariance(const EightMatches3D &data, AlignedVector< ENFT_SSE::__m128> &work)
{
#if _DEBUG
	assert(data.X1(0).reserve() == 1 && data.X1(1).reserve() == 1 && data.X1(2).reserve() == 1 && data.X1(3).reserve() == 1);
	assert(data.X1(4).reserve() == 1 && data.X1(5).reserve() == 1 && data.X1(6).reserve() == 1 && data.X1(7).reserve() == 1);
#endif

	work.Resize(2);
	work[0] =  ENFT_SSE::_mm_set1_ps(0.125f);
	m_mean1.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X1(0).XYZx(), data.X1(1).XYZx()),  ENFT_SSE::_mm_add_ps(data.X1(2).XYZx(), data.X1(3).XYZx())), 
										    ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X1(4).XYZx(), data.X1(5).XYZx()),  ENFT_SSE::_mm_add_ps(data.X1(6).XYZx(), data.X1(7).XYZx()))), 
								work[0]);
	m_mean2.XYZx() =  ENFT_SSE::_mm_mul_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X2(0).XYZx(), data.X2(1).XYZx()),  ENFT_SSE::_mm_add_ps(data.X2(2).XYZx(), data.X2(3).XYZx())), 
										    ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_add_ps(data.X2(4).XYZx(), data.X2(5).XYZx()),  ENFT_SSE::_mm_add_ps(data.X2(6).XYZx(), data.X2(7).XYZx()))), 
								work[0]);
	 ENFT_SSE::__m128 &dX1 = work[0], &dX2 = work[1];
	m_C.SetZero();

#ifndef ACCUMULATE_COVARIANCE_8
#define ACCUMULATE_COVARIANCE_8(i)\
	dX1 =  ENFT_SSE::_mm_sub_ps(data.X1(i).XYZx(), m_mean1.XYZx());\
	dX2 =  ENFT_SSE::_mm_sub_ps(data.X2(i).XYZx(), m_mean2.XYZx());\
	m_C.M_00_01_02_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[0])), m_C.M_00_01_02_x());\
	m_C.M_10_11_12_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[1])), m_C.M_10_11_12_x());\
	m_C.M_20_21_22_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[2])), m_C.M_20_21_22_x())
#endif

	ACCUMULATE_COVARIANCE_8(0);	ACCUMULATE_COVARIANCE_8(1);	ACCUMULATE_COVARIANCE_8(2);	ACCUMULATE_COVARIANCE_8(3);
	ACCUMULATE_COVARIANCE_8(4);	ACCUMULATE_COVARIANCE_8(5);	ACCUMULATE_COVARIANCE_8(6);	ACCUMULATE_COVARIANCE_8(7);
}

void RigidTransformationSolver::ComputeMeanAndCovariance(const AlignedVector<Point3D> &X1s, const AlignedVector<Point3D> &X2s, AlignedVector< ENFT_SSE::__m128> &work)
{
	work.Resize(2);

	m_mean1 =  ENFT_SSE::_mm_setzero_ps();
	const ushort N = ushort(X1s.Size());
	for(ushort i = 0; i < N; ++i)
		m_mean1.XYZx() =  ENFT_SSE::_mm_add_ps(X1s[i].XYZx(), m_mean1.XYZx());
	 ENFT_SSE::__m128 &one_over_N = work[0];
	one_over_N =  ENFT_SSE::_mm_set1_ps(1.0f / N);
	m_mean1.XYZx() =  ENFT_SSE::_mm_mul_ps(m_mean1.XYZx(), one_over_N);
	m_mean2.XYZx() =  ENFT_SSE::_mm_setzero_ps();
	for(ushort i = 0; i < N; ++i)
		m_mean2.XYZx() =  ENFT_SSE::_mm_add_ps(X2s[i].XYZx(), m_mean2.XYZx());
	m_mean2 =  ENFT_SSE::_mm_mul_ps(m_mean2.XYZx(), one_over_N);

	 ENFT_SSE::__m128 &dX1 = work[0], &dX2 = work[1];
	m_C.SetZero();
	for(ushort i = 0; i < N; ++i)
	{
		dX1 =  ENFT_SSE::_mm_sub_ps(X1s[i].XYZx(), m_mean1.XYZx());
		dX2 =  ENFT_SSE::_mm_sub_ps(X2s[i].XYZx(), m_mean2.XYZx());
		m_C.M_00_01_02_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[0])), m_C.M_00_01_02_x());
		m_C.M_10_11_12_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[1])), m_C.M_10_11_12_x());
		m_C.M_20_21_22_x() =  ENFT_SSE::_mm_add_ps( ENFT_SSE::_mm_mul_ps(dX1,  ENFT_SSE::_mm_set1_ps(dX2.m128_f32[2])), m_C.M_20_21_22_x());
	}
}

bool RigidTransformationSolver::RecoverFromMeanAndCovariance(RigidTransformation3D &T, AlignedVector< ENFT_SSE::__m128> &work)
{
	char jobu = 'A', jobvt = 'A';
	integer m = 3, n = 3, lda = 4, ldu = 4, ldvt = 4, lwork = 15, info;
	float s[3];

	// C = U * S * V^T
	// LAPACK treats matrix as column-major, i.e., C' = C^T.
	// C' = U' * S' * V'^T
	// Since C^T = V * S * U^T, LAPACK returned U' and V'^T correspond to V and U^T respectively.
	// So here just pass U and V^T to LAPACK reversely.
	work.Resize(4);
	sgesvd_(&jobu, &jobvt, &m, &n, m_C, &lda, s, m_VT, &ldvt, m_U, &ldu, (float *) work.Data(), &lwork, &info);

	if(info != 0)
		return false;

	LA::AlignedMatrix3f &V = m_VT;
	V.Transpose();
	T.r00() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_00_01_02_x(), V.M_00_01_02_x()));
	T.r01() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_00_01_02_x(), V.M_10_11_12_x()));
	T.r02() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_00_01_02_x(), V.M_20_21_22_x()));
	T.r10() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_10_11_12_x(), V.M_00_01_02_x()));
	T.r11() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_10_11_12_x(), V.M_10_11_12_x()));
	T.r12() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_10_11_12_x(), V.M_20_21_22_x()));
	T.r20() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_20_21_22_x(), V.M_00_01_02_x()));
	T.r21() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_20_21_22_x(), V.M_10_11_12_x()));
	T.r22() = ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(m_U.M_20_21_22_x(), V.M_20_21_22_x()));
	const float det = T.Determinant();
	if(det < 0)
	{
		T.r20() = -T.r20();
		T.r21() = -T.r21();
		T.r22() = -T.r22();
	}

	T.tX() = m_mean2.X() - ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(T.M_00_01_02_x(), m_mean1.XYZx()));
	T.tY() = m_mean2.Y() - ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(T.M_10_11_12_x(), m_mean1.XYZx()));
	T.tZ() = m_mean2.Z() - ENFT_SSE::SSE::Sum012( ENFT_SSE::_mm_mul_ps(T.M_20_21_22_x(), m_mean1.XYZx()));

	return true;
}