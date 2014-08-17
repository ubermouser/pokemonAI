#ifndef FP_COMPARE_H
#define FP_COMPARE_H

#include <stdint.h>

#include "../src/fixedpoint/fixed_class.h"

#define SIZETYPE_D int64_t
#define USIZETYPE_D uint64_t
#define ABSMASK_D 0x7FFFFFFFFFFFFFFF
#define SIZETYPE_F int32_t
#define USIZETYPE_F uint32_t
#define ABSMASK_F 0x7FFFFFFF
#define EPSILON_D 0.0000001
#define EPSILON_F 0.00001
#define EPSILON_S 108 // this value is "equivalent" to the FP_EPSILON for doubles above

inline static double fastabs(double x)
{	
	typedef union 
	{
		double fval;
		uint64_t ival;
	} intflt;
	
	intflt y = { x };
	y.ival &= ABSMASK_D;
	
	return y.fval;
	
	/*SIZETYPE_D y = (SIZETYPE_D)((USIZETYPE_D&)x & ABSMASK_D);
	return (double&)y;*/
};

inline static float fastabs(float x)
{
	typedef union 
	{
		float fval;
		uint32_t ival;
	} intflt;
	
	intflt y = { x };
	y.ival &= ABSMASK_F;
	
	return y.fval;
	
	/*SIZETYPE_F y = (SIZETYPE_F)((USIZETYPE_F&)x & ABSMASK_F);
	return (float&)y;*/
};

inline static fixType fastabs(fixType x)
{
	return fixedpoint::abs(x);
};

inline static bool mostlyEQ(double lhs, double rhs)
{
	return (fastabs(lhs - rhs) < EPSILON_D);
};

inline static bool mostlyEQ(float lhs, float rhs)
{
	return (fastabs(lhs - rhs) < EPSILON_F);
};

inline static bool mostlyEQ(fixType lhs, fixType rhs)
{
	return (fastabs(lhs - rhs).intValue < EPSILON_S);
};

#undef ABSMASK_D
#undef USIZETYPE_D
#undef SIZETYPE_D
#undef EPSILON_D
#undef ABSMASK_F
#undef USIZETYPE_F
#undef SIZETYPE_F
#undef EPSILON_F

template<class cType>
inline static bool mostlyNEQ(cType lhs, cType rhs)
{
	return !(mostlyEQ(lhs, rhs));
};

template<class cType>
inline static bool mostlyLTE(cType lhs, cType rhs)
{
	return (lhs < rhs) || (mostlyEQ(lhs, rhs));
};

template<class cType>
inline static bool mostlyGTE(cType lhs, cType rhs)
{
	return (lhs > rhs) || (mostlyEQ(lhs, rhs));
};

template<class cType>
inline static bool mostlyLT(cType lhs, cType rhs)
{
	return !(mostlyGTE(lhs, rhs));
};

template<class cType>
inline static bool mostlyGT(cType lhs, cType rhs)
{
	return !(mostlyLTE(lhs, rhs));
};

/* Scales a floating point number value, bounded from min..max,
 * to 0..1. */
template <class cType>
inline static cType scale(cType value, cType max, cType min)
{
	// scale from Max .. Min to -1 .. 1
	if (mostlyEQ(min, max)) return (cType)1.0;
	cType ratio = (value - min) / (max - min);

	return ratio;
};

/* Scales a floating point number value, bounded from 0..1,
 * to min..max. */
template <class cType>
inline static cType deScale(cType value, cType max, cType min)
{
	cType result = value * (max - min) + min;

	return result;
};

#endif /* FP_COMPARE_H */

