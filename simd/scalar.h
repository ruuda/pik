// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Single-element vectors and operations.
// (No include guard nor namespace: this is included from the middle of simd.h.)

// Returned by set_shift_*_count; do not use directly.
struct scalar_shift_left_count {
  int count;
};
struct scalar_shift_right_count {
  int count;
};

// (Wrapper class required for overloading comparison operators.)
template <typename T>
struct scalar {
  SIMD_INLINE scalar() {}
  scalar(const scalar&) = default;
  scalar& operator=(const scalar&) = default;
  SIMD_INLINE explicit scalar(const T t) : raw(t) {}

  SIMD_INLINE scalar& operator*=(const scalar other) {
    return *this = (*this * other);
  }
  SIMD_INLINE scalar& operator/=(const scalar other) {
    return *this = (*this / other);
  }
  SIMD_INLINE scalar& operator+=(const scalar other) {
    return *this = (*this + other);
  }
  SIMD_INLINE scalar& operator-=(const scalar other) {
    return *this = (*this - other);
  }
  SIMD_INLINE scalar& operator&=(const scalar other) {
    return *this = (*this & other);
  }
  SIMD_INLINE scalar& operator|=(const scalar other) {
    return *this = (*this | other);
  }
  SIMD_INLINE scalar& operator^=(const scalar other) {
    return *this = (*this ^ other);
  }

  T raw;
};

template <typename T>
struct VecT<T, 1, NONE> {
  using type = scalar<T>;
};

template <typename T>
struct Dup128T<T, NONE> {
  using type = void;  // unsupported but required for simd.h to compile.
};

// ------------------------------ Cast

template <typename T, typename FromT>
SIMD_INLINE scalar<T> cast_to(Scalar<T>, scalar<FromT> v) {
  static_assert(sizeof(T) <= sizeof(FromT), "Promoting is undefined");
  T to;
  CopyBytes<sizeof(FromT)>(&v.raw, &to);
  return scalar<T>(to);
}

// ------------------------------ Set

template <typename T>
SIMD_INLINE scalar<T> setzero(Scalar<T>) {
  return scalar<T>(T(0));
}

template <typename T, typename T2>
SIMD_INLINE scalar<T> set1(Scalar<T>, const T2 t) {
  return scalar<T>(t);
}

template <typename T, typename T2>
SIMD_INLINE scalar<T> iota(Scalar<T>, const T2 first) {
  return scalar<T>(first);
}

// ================================================== ARITHMETIC

template <typename T>
SIMD_INLINE scalar<T> operator+(const scalar<T> a, const scalar<T> b) {
  const uint64_t a64 = static_cast<int64_t>(a.raw);
  const uint64_t b64 = static_cast<int64_t>(b.raw);
  return scalar<T>((a64 + b64) & ~T(0));
}
SIMD_INLINE scalar<float> operator+(const scalar<float> a,
                                    const scalar<float> b) {
  return scalar<float>(a.raw + b.raw);
}
SIMD_INLINE scalar<double> operator+(const scalar<double> a,
                                     const scalar<double> b) {
  return scalar<double>(a.raw + b.raw);
}

template <typename T>
SIMD_INLINE scalar<T> operator-(const scalar<T> a, const scalar<T> b) {
  const uint64_t a64 = static_cast<int64_t>(a.raw);
  const uint64_t b64 = static_cast<int64_t>(b.raw);
  return scalar<T>((a64 - b64) & ~T(0));
}
SIMD_INLINE scalar<float> operator-(const scalar<float> a,
                                    const scalar<float> b) {
  return scalar<float>(a.raw - b.raw);
}
SIMD_INLINE scalar<double> operator-(const scalar<double> a,
                                     const scalar<double> b) {
  return scalar<double>(a.raw - b.raw);
}

// ------------------------------ Saturating addition

// Returns a + b clamped to the destination range.

// Unsigned
SIMD_INLINE scalar<uint8_t> add_sat(const scalar<uint8_t> a,
                                  const scalar<uint8_t> b) {
  return scalar<uint8_t>(SIMD_MIN(SIMD_MAX(0, a.raw + b.raw), 255));
}
SIMD_INLINE scalar<uint16_t> add_sat(const scalar<uint16_t> a,
                                   const scalar<uint16_t> b) {
  return scalar<uint16_t>(SIMD_MIN(SIMD_MAX(0, a.raw + b.raw), 65535));
}

// Signed
SIMD_INLINE scalar<int8_t> add_sat(const scalar<int8_t> a, const scalar<int8_t> b) {
  return scalar<int8_t>(SIMD_MIN(SIMD_MAX(-128, a.raw + b.raw), 127));
}
SIMD_INLINE scalar<int16_t> add_sat(const scalar<int16_t> a,
                                  const scalar<int16_t> b) {
  return scalar<int16_t>(SIMD_MIN(SIMD_MAX(-32768, a.raw + b.raw), 32767));
}

// ------------------------------ Saturating subtraction

// Returns a - b clamped to the destination range.

// Unsigned
SIMD_INLINE scalar<uint8_t> sub_sat(const scalar<uint8_t> a,
                                  const scalar<uint8_t> b) {
  return scalar<uint8_t>(SIMD_MIN(SIMD_MAX(0, a.raw - b.raw), 255));
}
SIMD_INLINE scalar<uint16_t> sub_sat(const scalar<uint16_t> a,
                                   const scalar<uint16_t> b) {
  return scalar<uint16_t>(SIMD_MIN(SIMD_MAX(0, a.raw - b.raw), 65535));
}

// Signed
SIMD_INLINE scalar<int8_t> sub_sat(const scalar<int8_t> a, const scalar<int8_t> b) {
  return scalar<int8_t>(SIMD_MIN(SIMD_MAX(-128, a.raw - b.raw), 127));
}
SIMD_INLINE scalar<int16_t> sub_sat(const scalar<int16_t> a,
                                  const scalar<int16_t> b) {
  return scalar<int16_t>(SIMD_MIN(SIMD_MAX(-32768, a.raw - b.raw), 32767));
}

// ------------------------------ Average

// Returns (a + b + 1) / 2

SIMD_INLINE scalar<uint8_t> avg(const scalar<uint8_t> a, const scalar<uint8_t> b) {
  return scalar<uint8_t>((a.raw + b.raw + 1) / 2);
}
SIMD_INLINE scalar<uint16_t> avg(const scalar<uint16_t> a, const scalar<uint16_t> b) {
  return scalar<uint16_t>((a.raw + b.raw + 1) / 2);
}

// ------------------------------ Absolute value

template <typename T>
SIMD_INLINE scalar<T> abs(const scalar<T> a) {
  const T i = a.raw;
  return (i >= 0 || i == LimitsMin<T>()) ? a : scalar<T>(-i);
}

// ------------------------------ Shift lanes by constant #bits

template <int kBits, typename T>
SIMD_INLINE scalar<T> shift_left(const scalar<T> v) {
  static_assert(0 <= kBits && kBits < sizeof(T) * 8, "Invalid shift");
  return scalar<T>(v.raw << kBits);
}

template <int kBits, typename T>
SIMD_INLINE scalar<T> shift_right(const scalar<T> v) {
  static_assert(0 <= kBits && kBits < sizeof(T) * 8, "Invalid shift");
  return scalar<T>(v.raw >> kBits);
}

// ------------------------------ Shift lanes by same variable #bits

template <typename T>
SIMD_INLINE scalar_shift_left_count set_shift_left_count(Scalar<T>,
                                                         const int bits) {
  return scalar_shift_left_count{bits};
}

template <typename T>
SIMD_INLINE scalar_shift_right_count set_shift_right_count(Scalar<T>,
                                                           const int bits) {
  return scalar_shift_right_count{bits};
}

template <typename T>
SIMD_INLINE scalar<T> shift_left_same(const scalar<T> v,
                                      const scalar_shift_left_count bits) {
  return scalar<T>(v.raw << bits.count);
}
template <typename T>
SIMD_INLINE scalar<T> shift_right_same(const scalar<T> v,
                                       const scalar_shift_right_count bits) {
  return scalar<T>(v.raw >> bits.count);
}

// ------------------------------ Shift lanes by independent variable #bits

// Single-lane => same as above except for the argument type.
template <typename T>
SIMD_INLINE scalar<T> shift_left_var(const scalar<T> v, const scalar<T> bits) {
  return scalar<T>(v.raw << bits.raw);
}
template <typename T>
SIMD_INLINE scalar<T> shift_right_var(const scalar<T> v, const scalar<T> bits) {
  return scalar<T>(v.raw >> bits.raw);
}

// ------------------------------ min/max

template <typename T>
SIMD_INLINE scalar<T> min(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(SIMD_MIN(a.raw, b.raw));
}

template <typename T>
SIMD_INLINE scalar<T> max(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(SIMD_MAX(a.raw, b.raw));
}

// Returns the closest value to v within [lo, hi].
template <typename T>
SIMD_INLINE scalar<T> clamp(const scalar<T> v, const scalar<T> lo,
                            const scalar<T> hi) {
  return min(max(lo, v), hi);
}

// ------------------------------ mul/div

template <typename T>
SIMD_INLINE scalar<T> operator*(const scalar<T> a, const scalar<T> b) {
  if (IsFloat<T>()) {
    return scalar<T>(static_cast<T>(double(a.raw) * b.raw));
  } else if (IsSigned<T>()) {
    return scalar<T>(static_cast<T>(int64_t(a.raw) * b.raw));
  } else {
    return scalar<T>(static_cast<T>(uint64_t(a.raw) * b.raw));
  }
}

template <typename T>
SIMD_INLINE scalar<T> operator/(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(a.raw / b.raw);
}

// "Extensions": useful but not quite performance-portable operations. We add
// functions to this namespace in multiple places.
namespace ext {

// Returns the upper 16 bits of a * b in each lane.
SIMD_INLINE scalar<int16_t> mulhi(const scalar<int16_t> a, const scalar<int16_t> b) {
  return scalar<int16_t>((a.raw * b.raw) >> 16);
}

// Returns (((a * b) >> 14) + 1) >> 1.
SIMD_INLINE scalar<int16_t> mulhrs(const scalar<int16_t> a, const scalar<int16_t> b) {
  const int rounded = ((a.raw * b.raw) + (1 << 14)) >> 15;
  const int clamped = SIMD_MIN(SIMD_MAX(-32768, rounded), 32767);
  return scalar<int16_t>(clamped);
}

}  // namespace ext

// Multiplies even lanes (0, 2 ..) and returns the double-wide result.
SIMD_INLINE scalar<int64_t> mul_even(const scalar<int32_t> a,
                                   const scalar<int32_t> b) {
  const int64_t a64 = a.raw;
  return scalar<int64_t>(a64 * b.raw);
}
SIMD_INLINE scalar<uint64_t> mul_even(const scalar<uint32_t> a,
                                    const scalar<uint32_t> b) {
  const uint64_t a64 = a.raw;
  return scalar<uint64_t>(a64 * b.raw);
}

// Approximate reciprocal
SIMD_INLINE scalar<float> rcp_approx(const scalar<float> v) {
  return scalar<float>(1.0f / v.raw);
}

// ------------------------------ Floating-point multiply-add variants

template <typename T>
SIMD_INLINE scalar<T> mul_add(const scalar<T> mul, const scalar<T> x,
                            const scalar<T> add) {
  return mul * x + add;
}

template <typename T>
SIMD_INLINE scalar<T> mul_sub(const scalar<T> mul, const scalar<T> x,
                            const scalar<T> sub) {
  return mul * x - sub;
}

template <typename T>
SIMD_INLINE scalar<T> nmul_add(const scalar<T> mul, const scalar<T> x,
                             const scalar<T> add) {
  return add - mul * x;
}

// nmul_sub would require an additional negate of mul or x.

// ------------------------------ Floating-point square root

// Approximate reciprocal square root
SIMD_INLINE scalar<float> rsqrt_approx(const scalar<float> v) {
  float f = v.raw;
  const float half = f * 0.5f;
  uint32_t bits;
  CopyBytes<4>(&f, &bits);
  // Initial guess based on log2(f)
  bits = 0x5F3759DF - (bits >> 1);
  CopyBytes<4>(&bits, &f);
  // One Newton-Raphson iteration
  return scalar<float>(f * (1.5f - (half * f * f)));
}

// Square root
SIMD_INLINE scalar<float> sqrt(const scalar<float> v) {
  return rsqrt_approx(v) * v;
}
SIMD_INLINE scalar<double> sqrt(const scalar<double> v) {
  return scalar<double>(sqrt(scalar<float>(v.raw)).raw);
}

// ------------------------------ Floating-point rounding

// Approximation of round-to-nearest for numbers representable as integers.
SIMD_INLINE scalar<float> round_nearest(const scalar<float> v) {
  const float bias = v.raw < 0.0f ? -0.5f : 0.5f;
  return scalar<float>(static_cast<int32_t>(v.raw + bias));
}
SIMD_INLINE scalar<double> round_nearest(const scalar<double> v) {
  const double bias = v.raw < 0.0 ? -0.5 : 0.5;
  return scalar<double>(static_cast<int64_t>(v.raw + bias));
}

template <typename Float, typename Bits, int kMantissaBits, int kExponentBits,
          class V>
V Ceiling(const V v) {
  const Bits kExponentMask = (1ull << kExponentBits) - 1;
  const Bits kMantissaMask = (1ull << kMantissaBits) - 1;
  const Bits kBias = kExponentMask / 2;

  Float f = v.raw;
  const bool positive = f > 0.0f;

  Bits bits;
  CopyBytes<sizeof(Bits)>(&v, &bits);

  const int exponent = ((bits >> kMantissaBits) & kExponentMask) - kBias;
  // Already an integer.
  if (exponent >= kMantissaBits) return v;
  // |v| <= 1 => 0 or 1.
  if (exponent < 0) return V(positive);

  const Bits mantissa_mask = kMantissaMask >> exponent;
  // Already an integer
  if ((bits & mantissa_mask) == 0) return v;

  // Clear fractional bits and round up
  if (positive) bits += (kMantissaMask + 1) >> exponent;
  bits &= ~mantissa_mask;

  CopyBytes<sizeof(Bits)>(&bits, &f);
  return V(f);
}

template <typename Float, typename Bits, int kMantissaBits, int kExponentBits,
          class V>
V Floor(const V v) {
  const Bits kExponentMask = (1ull << kExponentBits) - 1;
  const Bits kMantissaMask = (1ull << kMantissaBits) - 1;
  const Bits kBias = kExponentMask / 2;

  Float f = v.raw;
  const bool negative = f < 0.0f;

  Bits bits;
  CopyBytes<sizeof(Bits)>(&v, &bits);

  const int exponent = ((bits >> kMantissaBits) & kExponentMask) - kBias;
  // Already an integer.
  if (exponent >= kMantissaBits) return v;
  // |v| <= 1 => -1 or 0.
  if (exponent < 0) return V(negative ? -1.0 : 0.0f);

  const Bits mantissa_mask = kMantissaMask >> exponent;
  // Already an integer
  if ((bits & mantissa_mask) == 0) return v;

  // Clear fractional bits and round down
  if (negative) bits += (kMantissaMask + 1) >> exponent;
  bits &= ~mantissa_mask;

  CopyBytes<sizeof(Bits)>(&bits, &f);
  return V(f);
}

// Toward +infinity, aka ceiling
SIMD_INLINE scalar<float> round_pos_inf(const scalar<float> v) {
  return Ceiling<float, uint32_t, 23, 8>(v);
}
SIMD_INLINE scalar<double> round_pos_inf(const scalar<double> v) {
  return Ceiling<double, uint64_t, 52, 11>(v);
}

// Toward -infinity, aka floor
SIMD_INLINE scalar<float> round_neg_inf(const scalar<float> v) {
  return Floor<float, uint32_t, 23, 8>(v);
}
SIMD_INLINE scalar<double> round_neg_inf(const scalar<double> v) {
  return Floor<double, uint64_t, 52, 11>(v);
}

// ================================================== COMPARE

// Comparisons fill a lane with 1-bits if the condition is true, else 0.
template <typename T>
scalar<T> ComparisonResult(const bool result) {
  T ret;
  SetBytes(result ? 0xFF : 0, &ret);
  return scalar<T>(ret);
}

template <typename T>
SIMD_INLINE scalar<T> operator==(const scalar<T> a, const scalar<T> b) {
  return ComparisonResult<T>(a.raw == b.raw);
}

template <typename T>
SIMD_INLINE scalar<T> operator<(const scalar<T> a, const scalar<T> b) {
  return ComparisonResult<T>(a.raw < b.raw);
}
template <typename T>
SIMD_INLINE scalar<T> operator>(const scalar<T> a, const scalar<T> b) {
  return ComparisonResult<T>(a.raw > b.raw);
}

template <typename T>
SIMD_INLINE scalar<T> operator<=(const scalar<T> a, const scalar<T> b) {
  return ComparisonResult<T>(a.raw <= b.raw);
}
template <typename T>
SIMD_INLINE scalar<T> operator>=(const scalar<T> a, const scalar<T> b) {
  return ComparisonResult<T>(a.raw >= b.raw);
}

// ================================================== LOGICAL

template <typename Bits>
struct BitwiseOp {
  template <typename T, class Op>
  scalar<T> operator()(const scalar<T> a, const scalar<T> b, const Op& op) const {
    static_assert(sizeof(T) == sizeof(Bits), "Float/int size mismatch");
    Bits ia, ib;
    CopyBytes<sizeof(Bits)>(&a, &ia);
    CopyBytes<sizeof(Bits)>(&b, &ib);
    ia = op(ia, ib);
    T ret;
    CopyBytes<sizeof(Bits)>(&ia, &ret);
    return scalar<T>(ret);
  }
};

// ------------------------------ Bitwise AND

template <typename T>
SIMD_INLINE scalar<T> operator&(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(a.raw & b.raw);
}
template <>
SIMD_INLINE scalar<float> operator&(const scalar<float> a, const scalar<float> b) {
  return BitwiseOp<int32_t>()(a, b, [](int32_t i, int32_t j) { return i & j; });
}
template <>
SIMD_INLINE scalar<double> operator&(const scalar<double> a, const scalar<double> b) {
  return BitwiseOp<int64_t>()(a, b, [](int64_t i, int64_t j) { return i & j; });
}

// ------------------------------ Bitwise AND-NOT

// Returns ~a & b.
template <typename T>
SIMD_INLINE scalar<T> andnot(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(~a.raw & b.raw);
}
template <>
SIMD_INLINE scalar<float> andnot(const scalar<float> a, const scalar<float> b) {
  return BitwiseOp<int32_t>()(a, b,
                              [](int32_t i, int32_t j) { return ~i & j; });
}
template <>
SIMD_INLINE scalar<double> andnot(const scalar<double> a, const scalar<double> b) {
  return BitwiseOp<int64_t>()(a, b,
                              [](int64_t i, int64_t j) { return ~i & j; });
}

// ------------------------------ Bitwise OR

template <typename T>
SIMD_INLINE scalar<T> operator|(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(a.raw | b.raw);
}
template <>
SIMD_INLINE scalar<float> operator|(const scalar<float> a, const scalar<float> b) {
  return BitwiseOp<int32_t>()(a, b, [](int32_t i, int32_t j) { return i | j; });
}
template <>
SIMD_INLINE scalar<double> operator|(const scalar<double> a, const scalar<double> b) {
  return BitwiseOp<int64_t>()(a, b, [](int64_t i, int64_t j) { return i | j; });
}

// ------------------------------ Bitwise XOR

template <typename T>
SIMD_INLINE scalar<T> operator^(const scalar<T> a, const scalar<T> b) {
  return scalar<T>(a.raw ^ b.raw);
}
template <>
SIMD_INLINE scalar<float> operator^(const scalar<float> a, const scalar<float> b) {
  return BitwiseOp<int32_t>()(a, b, [](int32_t i, int32_t j) { return i ^ j; });
}
template <>
SIMD_INLINE scalar<double> operator^(const scalar<double> a, const scalar<double> b) {
  return BitwiseOp<int64_t>()(a, b, [](int64_t i, int64_t j) { return i ^ j; });
}

// ------------------------------ Select/blend

// Returns mask ? b : a. Each lane of "mask" must equal T(0) or ~T(0).
template <typename T>
SIMD_INLINE scalar<T> select(const scalar<T> a, const scalar<T> b,
                             const scalar<T> mask) {
  return (mask & b) | andnot(mask, a);
}

// ================================================== MEMORY

// ------------------------------ Load

template <typename T>
SIMD_INLINE scalar<T> load(Scalar<T>, const T* SIMD_RESTRICT aligned) {
  T t;
  CopyBytes<sizeof(T)>(aligned, &t);
  return scalar<T>(t);
}

template <typename T>
SIMD_INLINE scalar<T> load_unaligned(Scalar<T> d, const T* SIMD_RESTRICT p) {
  return load(d, p);
}

// no load_dup128: that requires at least 128-bit vectors.

// ------------------------------ Store

template <typename T>
SIMD_INLINE void store(const scalar<T> v, Scalar<T>, T* SIMD_RESTRICT aligned) {
  CopyBytes<sizeof(T)>(&v.raw, aligned);
}

template <typename T>
SIMD_INLINE void store_unaligned(const scalar<T> v, Scalar<T> d,
                                 T* SIMD_RESTRICT p) {
  return store(v, d, p);
}

// ------------------------------ "Non-temporal" stores

template <typename T>
SIMD_INLINE void stream(const scalar<T> v, Scalar<T> d,
                        T* SIMD_RESTRICT aligned) {
  return store(v, d, aligned);
}

// ================================================== CONVERT

template <typename FromT, typename ToT>
SIMD_INLINE scalar<ToT> convert_to(Desc<ToT, 1, NONE>,
                                   const scalar<FromT> from) {
  return scalar<ToT>(from.raw);
}

SIMD_INLINE scalar<float> convert_to(Scalar<float>, const scalar<int32_t> v) {
  return scalar<float>(v.raw);
}

// Truncates (rounds toward zero).
SIMD_INLINE scalar<int32_t> convert_to(Scalar<int32_t>, const scalar<float> v) {
  const float f = v.raw;
  return scalar<int32_t>(f);
}

// Approximation of round-to-nearest for numbers representable as int32_t.
SIMD_INLINE scalar<int32_t> nearest_int(const scalar<float> v) {
  const float f = v.raw;
  const float bias = f < 0.0f ? -0.5f : 0.5f;
  return scalar<int32_t>(f + bias);
}

// ================================================== SWIZZLE

// Unsupported: shift_bytes_*, extract_concat_bytes, interleave_*, other_half,
// shuffle_*, sums_of_u8x8, horz_sum - these require more than one lane and/or
// actual 128-bit vectors.

// ------------------------------ Broadcast/splat any lane

template <int kLane, typename T>
SIMD_INLINE scalar<T> broadcast(const scalar<T> v) {
  static_assert(kLane == 0, "Scalar only has one lane");
  return v;
}

// ------------------------------ Zip/unpack

SIMD_INLINE scalar<uint16_t> zip_lo(const scalar<uint8_t> a,
                                  const scalar<uint8_t> b) {
  return scalar<uint16_t>((uint32_t(b.raw) << 8) + a.raw);
}
SIMD_INLINE scalar<uint32_t> zip_lo(const scalar<uint16_t> a,
                                  const scalar<uint16_t> b) {
  return scalar<uint32_t>((uint32_t(b.raw) << 16) + a.raw);
}
SIMD_INLINE scalar<uint64_t> zip_lo(const scalar<uint32_t> a,
                                  const scalar<uint32_t> b) {
  return scalar<uint64_t>((uint64_t(b.raw) << 32) + a.raw);
}
SIMD_INLINE scalar<int16_t> zip_lo(const scalar<int8_t> a, const scalar<int8_t> b) {
  return scalar<int16_t>((uint32_t(b.raw) << 8) + a.raw);
}
SIMD_INLINE scalar<int32_t> zip_lo(const scalar<int16_t> a, const scalar<int16_t> b) {
  return scalar<int32_t>((uint32_t(b.raw) << 16) + a.raw);
}
SIMD_INLINE scalar<int64_t> zip_lo(const scalar<int32_t> a, const scalar<int32_t> b) {
  return scalar<int64_t>((uint64_t(b.raw) << 32) + a.raw);
}

template <typename T>
SIMD_INLINE auto zip_hi(const scalar<T> a, const scalar<T> b)
    -> decltype(zip_lo(a, b)) {
  return zip_lo(a, b);
}

// ------------------------------ Parts

template <typename T, typename T2>
SIMD_INLINE scalar<T> set_part(Scalar<T>, const T2 t) {
  return scalar<T>(t);
}

template <typename T>
SIMD_INLINE T get_part(Scalar<T>, const scalar<T> v) {
  return v.raw;
}

template <typename T>
SIMD_INLINE scalar<T> any_part(Scalar<T>, const scalar<T> v) {
  return v;
}

template <int kLane, typename T>
SIMD_INLINE scalar<T> broadcast_part(Scalar<T>, const scalar<T> v) {
  static_assert(kLane == 0, "Invalid kLane");
  return v;
}

// ================================================== MISC

// "Extensions": useful but not quite performance-portable operations. We add
// functions to this namespace in multiple places.
namespace ext {

// Returns a bit array of the most significant bit of each byte in "v", i.e.
// sum_i=0..15 of (v[i] >> 7) << i; v[0] is the least-significant byte of "v".
// This is useful for testing/branching based on comparison results.
SIMD_INLINE uint32_t movemask(const scalar<uint8_t> v) { return v.raw >> 7; }

// Returns the most significant bit of each float/double lane (see above).
SIMD_INLINE uint32_t movemask(const scalar<float> v) { return v.raw < 0.0f; }
SIMD_INLINE uint32_t movemask(const scalar<double> v) { return v.raw < 0.0; }

// Returns whether all lanes are equal to zero. Supported for all integer T.
template <typename T>
SIMD_INLINE bool all_zero(const scalar<T> v) {
  return v.raw == 0;
}

// Sum of all lanes, i.e. the only one.
template <typename T>
SIMD_INLINE scalar<T> horz_sum(const scalar<T> v0) {
  return v0;
}

}  // namespace ext
