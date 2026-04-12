/*
Copyright (c) 2007, Markus Trenkwalder

All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution.

* Neither the name of the library's copyright owner nor the names of its 
  contributors may be used to endorse or promote products derived from this 
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FIXEDP_CLASS_H_INCLUDED
#define FIXEDP_CLASS_H_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#include <concepts>
#include "fixed_func.h"

namespace fixedpoint {

// The template argument p in all of the following functions refers to the 
// fixed point precision (e.g. p = 8 gives 24.8 fixed point functions).

template <int p, typename BaseT = int32_t>
struct fixed_point {
  BaseT intValue;

  fixed_point() {}

  template <std::integral T>
  explicit fixed_point(T i) : intValue(static_cast<BaseT>(i) << p) {}

  template <std::integral T1, std::integral T2>
  explicit fixed_point(T1 n, T2 d) : intValue(fixdiv<p, BaseT>(static_cast<BaseT>(n), static_cast<BaseT>(d))) {}

  template <std::floating_point T>
  explicit fixed_point(T f) : intValue(double2fix<p, BaseT>(static_cast<double>(f))) {}

  double to_double() const { return fix2double<p, BaseT>(this->intValue); };
  explicit operator double() const { return to_double(); }
  explicit operator float() const { return static_cast<float>(to_double()); }

  fixed_point& operator+=(fixed_point r) {
    intValue += r.intValue;
    return *this;
  }
  fixed_point& operator-=(fixed_point r) {
    intValue -= r.intValue;
    return *this;
  }
  fixed_point& operator*=(fixed_point r) {
    intValue = fixmul<p, BaseT>(intValue, r.intValue);
    return *this;
  }
  fixed_point& operator/=(fixed_point r) {
    intValue = fixdiv<p, BaseT>(intValue, r.intValue);
    return *this;
  }

  fixed_point& operator+=(BaseT r) {
    intValue += r;
    return *this;
  }
  fixed_point& operator-=(BaseT r) {
    intValue -= r;
    return *this;
  }
  fixed_point& operator*=(BaseT r) {
    intValue *= r;
    return *this;
  }
  fixed_point& operator/=(BaseT r) {
    intValue /= r;
    return *this;
  }


  fixed_point operator-() const {
    fixed_point x;
    x.intValue = -intValue;
    return x;
  }
  fixed_point operator+(fixed_point r) const {
    fixed_point x = *this;
    x += r;
    return x;
  }
  fixed_point operator-(fixed_point r) const {
    fixed_point x = *this;
    x -= r;
    return x;
  }
  fixed_point operator*(fixed_point r) const {
    fixed_point x = *this;
    x *= r;
    return x;
  }
  fixed_point operator/(fixed_point r) const {
    fixed_point x = *this;
    x /= r;
    return x;
  }

  bool operator == (fixed_point r) const { return intValue == r.intValue; }
  bool operator != (fixed_point r) const { return !(*this == r); }
  bool operator <  (fixed_point r) const { return intValue < r.intValue; }
  bool operator >  (fixed_point r) const { return intValue > r.intValue; }
  bool operator <= (fixed_point r) const { return intValue <= r.intValue; }
  bool operator >= (fixed_point r) const { return intValue >= r.intValue; }

  fixed_point operator+(BaseT r) const {
    fixed_point x = *this;
    x += r;
    return x;
  }
  fixed_point operator-(BaseT r) const {
    fixed_point x = *this;
    x -= r;
    return x;
  }
  fixed_point operator*(BaseT r) const {
    fixed_point x = *this;
    x *= r;
    return x;
  }
  fixed_point operator/(BaseT r) const {
    fixed_point x = *this;
    x /= r;
    return x;
  }
};

template <int p, typename BaseT = int32_t>
static fixed_point<p, BaseT> create(double f) {
  return fixed_point<p, BaseT>(double2fix<p, BaseT>(f));
};

// Specializations for use with plain integers
template <int p, typename BaseT>
inline fixed_point<p, BaseT> operator + (BaseT a, fixed_point<p, BaseT> b)
{ return b + a; }

template <int p, typename BaseT>
inline fixed_point<p, BaseT> operator - (BaseT a, fixed_point<p, BaseT> b)
{ return -b + a; }

template <int p, typename BaseT>
inline fixed_point<p, BaseT> operator * (BaseT a, fixed_point<p, BaseT> b)
{ return b * a; }

template <int p, typename BaseT>
inline fixed_point<p, BaseT> operator / (BaseT a, fixed_point<p, BaseT> b)
{ fixed_point<p, BaseT> r(a); r /= b; return r; }

// math functions
// no default implementation

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> sin(fixed_point<p, BaseT> a);

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> cos(fixed_point<p, BaseT> a);

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> sqrt(fixed_point<p, BaseT> a);

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> rsqrt(fixed_point<p, BaseT> a);

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> inv(fixed_point<p, BaseT> a);

template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> abs(fixed_point<p, BaseT> a)
{ 
  fixed_point<p, BaseT> r;
  r.intValue = a.intValue > 0 ? a.intValue : -a.intValue; 
  return r; 
}

// specializations for 16.16 format

template <>
inline fixed_point<16, int32_t> sin(fixed_point<16, int32_t> a)
{
  fixed_point<16, int32_t> r;
  r.intValue = fixsin16(a.intValue);
  return r;
}

template <>
inline fixed_point<16, int32_t> cos(fixed_point<16, int32_t> a)
{
  fixed_point<16, int32_t> r;
  r.intValue = fixcos16(a.intValue);
  return r;
}


template <>
inline fixed_point<16, int32_t> sqrt(fixed_point<16, int32_t> a)
{
  fixed_point<16, int32_t> r;
  r.intValue = fixsqrt16(a.intValue);
  return r;
}

template <>
inline fixed_point<16, int32_t> rsqrt(fixed_point<16, int32_t> a)
{
  fixed_point<16, int32_t> r;
  r.intValue = fixrsqrt16(a.intValue);
  return r;
}

template <>
inline fixed_point<16, int32_t> inv(fixed_point<16, int32_t> a)
{
  fixed_point<16, int32_t> r;
  r.intValue = fixinv<16>(a.intValue);
  return r;
}

// The multiply accumulate case can be optimized.
template <int p, typename BaseT = int32_t>
inline fixed_point<p, BaseT> multiply_accumulate(
  int count, 
  const fixed_point<p, BaseT> *a,
  const fixed_point<p, BaseT> *b)
{
  typedef typename intermediate_type<BaseT>::type I;
  I result = 0;
  for (int i = 0; i < count; ++i)
    result += static_cast<I>(a[i].intValue) * b[i].intValue;
  fixed_point<p, BaseT> r;
  r.intValue = static_cast<BaseT>(result >> p);
  return r;
}

} // end namespace fixedpoint

#endif

