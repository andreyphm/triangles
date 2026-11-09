#pragma once

#include "vector2.hpp"
#include "vector3.hpp"

#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <vector>

#if defined(__FAST_MATH__) || defined(_M_FP_FAST)
#error "robust predicates require strict floating point: do not compile with -ffast-math or /fp:fast"
#endif

namespace geometry {

enum class PredicateSign {
    negative = -1,
    zero = 0,
    positive = 1
};

namespace detail {

template <std::floating_point T> struct TwoComponent {
    T low;
    T high;
};

template <std::floating_point T> TwoComponent<T> two_sum(T first, T second) {
    const T high = first + second;
    const T recovered_second = high - first;
    const T recovered_first = high - recovered_second;
    const T second_error = second - recovered_second;
    const T first_error = first - recovered_first;
    return {first_error + second_error, high};
}

template <std::floating_point T> TwoComponent<T> two_product(T first, T second) {
    const T high = first * second;
    return {std::fma(first, second, -high), high};
}

template <std::floating_point T> std::vector<T> grow_expansion(const std::vector<T>& expansion, T value) {
    std::vector<T> result;
    result.reserve(expansion.size() + 1);

    T accumulator = value;
    for (T component : expansion) {
        const TwoComponent<T> sum = two_sum(accumulator, component);
        if (sum.low != T{0}) {
            result.push_back(sum.low);
        }
        accumulator = sum.high;
    }

    if (accumulator != T{0}) {
        result.push_back(accumulator);
    }
    return result;
}

template <std::floating_point T> std::vector<T> add_expansions(const std::vector<T>& first, const std::vector<T>& second) {
    std::vector<T> result = first;

    for (T component : second) {
        result = grow_expansion(result, component);
    }
    return result;
}

template <std::floating_point T> std::vector<T> scale_expansion(const std::vector<T>& expansion, T scalar) {
    if (scalar == T{0} || expansion.empty()) {
        return {};
    }

    std::vector<T> result;
    for (T component : expansion) {
        const TwoComponent<T> product = two_product(component, scalar);
        if (product.low != T{0}) {
            result = grow_expansion(result, product.low);
        }
        if (product.high != T{0}) {
            result = grow_expansion(result, product.high);
        }
    }
    return result;
}

template <std::floating_point T> class Expansion {
public:
    Expansion() = default;
    Expansion(T exact_value) {
        if (exact_value != T{0}) {
            components_.push_back(exact_value);
        }
    }

    PredicateSign sign() const {
        if (components_.empty()) {
            return PredicateSign::zero;
        }
        return (components_.back() < T{0}) ? PredicateSign::negative : PredicateSign::positive;
    }

    Expansion add(const Expansion& other) const {
        Expansion result;
        result.components_ = add_expansions(components_, other.components_);
        return result;
    }

    Expansion sub(const Expansion& other) const {
        Expansion negative;
        for (T component : other.components_) {
            negative.components_.push_back(-component);
        }
        return add(negative);
    }

    Expansion mul(const Expansion& other) const {
        Expansion result;

        for (T component : other.components_) {
            result.components_ = add_expansions(result.components_, scale_expansion(components_, component));
        }
        return result;
    }

private:
    std::vector<T> components_;
};

template <std::floating_point T> constexpr T epsilon = std::numeric_limits<T>::epsilon() / T{2};
template <std::floating_point T> constexpr T orient2d_error_bound = (T{3} + T{16} * epsilon<T>) * epsilon<T>;
template <std::floating_point T> constexpr T orient3d_error_bound = (T{7} + T{56} * epsilon<T>) * epsilon<T>;

template <std::floating_point T> PredicateSign sign_of(T value) {
    if (value > T{0}) {
        return PredicateSign::positive;
    }
    if (value < T{0}) {
        return PredicateSign::negative;
    }
    return PredicateSign::zero;
}

template <std::floating_point T> Expansion<T> exact_sub(T first, T second) {
    return Expansion<T>(first).sub(Expansion<T>(second));
}

template <std::floating_point T> void require_finite(const Vector2<T>& first, const Vector2<T>& second, const Vector2<T>& third) {
    if (!first.is_finite() || !second.is_finite() || !third.is_finite()) {
        throw std::invalid_argument("orient2d arguments must be finite");
    }
}

template <std::floating_point T> void require_finite(const Point3<T>& first, const Point3<T>& second,
                                                     const Point3<T>& third, const Point3<T>& fourth) {
    if (!first.is_finite() || !second.is_finite() || !third.is_finite() || !fourth.is_finite()) {
        throw std::invalid_argument("orient3d arguments must be finite");
    }
}

} // namespace detail

template <std::floating_point T> PredicateSign orient2d(const Vector2<T>& first, const Vector2<T>& second, const Vector2<T>& third) {
    detail::require_finite(first, second, third);

    const T first_x  = first.x  - third.x;
    const T first_y  = first.y  - third.y;
    const T second_x = second.x - third.x;
    const T second_y = second.y - third.y;

    const T left  = first_x * second_y;
    const T right = first_y * second_x;

    const T determinant = left - right;
    const T permanent = std::abs(left) + std::abs(right);
    const T error = detail::orient2d_error_bound<T> * permanent;

    if (determinant > error || determinant < -error) {
        return detail::sign_of(determinant);
    }

    using detail::exact_sub;
    using detail::Expansion;

    const Expansion<T> exact_first_x  = exact_sub(first.x, third.x);
    const Expansion<T> exact_first_y  = exact_sub(first.y, third.y);
    const Expansion<T> exact_second_x = exact_sub(second.x, third.x);
    const Expansion<T> exact_second_y = exact_sub(second.y, third.y);

    return exact_first_x.mul(exact_second_y).sub(exact_first_y.mul(exact_second_x)).sign();
}

template <std::floating_point T> PredicateSign orient3d(const Point3<T>& first, const Point3<T>& second,
                                                        const Point3<T>& third, const Point3<T>& fourth) {
    detail::require_finite(first, second, third, fourth);

    const T first_x  = first.x  - fourth.x;
    const T first_y  = first.y  - fourth.y;
    const T first_z  = first.z  - fourth.z;
    const T second_x = second.x - fourth.x;
    const T second_y = second.y - fourth.y;
    const T second_z = second.z - fourth.z;
    const T third_x  = third.x  - fourth.x;
    const T third_y  = third.y  - fourth.y;
    const T third_z  = third.z  - fourth.z;

    const T second_x_third_y = second_x * third_y;
    const T third_x_second_y = third_x  * second_y;
    const T third_x_first_y  = third_x  * first_y;
    const T first_x_third_y  = first_x  * third_y;
    const T first_x_second_y = first_x  * second_y;
    const T second_x_first_y = second_x * first_y;

    const T determinant =
          first_z  * (second_x_third_y - third_x_second_y)
        - second_z * (first_x_third_y  - third_x_first_y)
        + third_z  * (first_x_second_y - second_x_first_y);

    const T permanent =
          (std::abs(second_x_third_y) + std::abs(third_x_second_y)) * std::abs(first_z)
        + (std::abs(third_x_first_y)  + std::abs(first_x_third_y))  * std::abs(second_z)
        + (std::abs(first_x_second_y) + std::abs(second_x_first_y)) * std::abs(third_z);

    const T error = detail::orient3d_error_bound<T> * permanent;

    if (determinant > error || determinant < -error) {
        return detail::sign_of(determinant);
    }

    using detail::exact_sub;
    using detail::Expansion;

    const Expansion<T> exact_first_x  = exact_sub(first.x,  fourth.x);
    const Expansion<T> exact_first_y  = exact_sub(first.y,  fourth.y);
    const Expansion<T> exact_first_z  = exact_sub(first.z,  fourth.z);
    const Expansion<T> exact_second_x = exact_sub(second.x, fourth.x);
    const Expansion<T> exact_second_y = exact_sub(second.y, fourth.y);
    const Expansion<T> exact_second_z = exact_sub(second.z, fourth.z);
    const Expansion<T> exact_third_x  = exact_sub(third.x,  fourth.x);
    const Expansion<T> exact_third_y  = exact_sub(third.y,  fourth.y);
    const Expansion<T> exact_third_z  = exact_sub(third.z,  fourth.z);

    const Expansion<T> first_minor  = exact_second_x.mul(exact_third_y).sub(exact_third_x.mul(exact_second_y));
    const Expansion<T> second_minor = exact_first_x.mul(exact_third_y).sub(exact_third_x.mul(exact_first_y));
    const Expansion<T> third_minor  = exact_first_x.mul(exact_second_y).sub(exact_second_x.mul(exact_first_y));

    return exact_first_z.mul(first_minor).sub(exact_second_z.mul(second_minor)).add(exact_third_z.mul(third_minor)).sign();
}

} // namespace geometry
