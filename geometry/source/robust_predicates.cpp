#include "robust_predicates.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace geometry {

namespace {

struct TwoComponent {
    double low;
    double high;
};

TwoComponent two_sum(double first, double second) {
    const double high = first + second;
    const double recovered_second = high - first;
    const double recovered_first = high - recovered_second;
    const double second_error = second - recovered_second;
    const double first_error = first - recovered_first;
    return {first_error + second_error, high};
}

TwoComponent two_product(double first, double second) {
    const double high = first * second;
    return {std::fma(first, second, -high), high};
}

std::vector<double> grow_expansion(const std::vector<double>& expansion, double value) {
    std::vector<double> result;
    result.reserve(expansion.size() + 1);

    double accumulator = value;
    for (double component : expansion) {
        const TwoComponent sum = two_sum(accumulator, component);
        if (sum.low != 0.0) {
            result.push_back(sum.low);
        }
        accumulator = sum.high;
    }

    if (accumulator != 0.0) {
        result.push_back(accumulator);
    }
    return result;
}

std::vector<double> add_expansions(const std::vector<double>& first, const std::vector<double>& second) {
    std::vector<double> result = first;

    for (double component : second) {
        result = grow_expansion(result, component);
    }
    return result;
}

std::vector<double> scale_expansion(const std::vector<double>& expansion, double scalar) {
    if (scalar == 0.0 || expansion.empty()) {
        return {};
    }

    std::vector<double> result;
    for (double component : expansion) {
        const TwoComponent product = two_product(component, scalar);
        if (product.low != 0.0) {
            result = grow_expansion(result, product.low);
        }
        if (product.high != 0.0) {
            result = grow_expansion(result, product.high);
        }
    }
    return result;
}

class Expansion {
public:
    Expansion() = default;
    Expansion(double exact_value) {
        if (exact_value != 0.0) {
            components_.push_back(exact_value);
        }
    }

    PredicateSign sign() const {
        if (components_.empty()) {
            return PredicateSign::zero;
        }
        return (components_.back() < 0.0) ? PredicateSign::negative : PredicateSign::positive;
    }

    Expansion add(const Expansion& other) const {
        Expansion result;
        result.components_ = add_expansions(components_, other.components_);
        return result;
    }

    Expansion sub(const Expansion& other) const {
        Expansion negative;
        for (double component : other.components_) {
            negative.components_.push_back(-component);
        }
        return add(negative);
    }

    Expansion mul(const Expansion& other) const {
        Expansion result;

        for (double component : other.components_) {
            result.components_ = add_expansions(result.components_, scale_expansion(components_, component));
        }
        return result;
    }

private:
    std::vector<double> components_;
};

constexpr double epsilon = std::numeric_limits<double>::epsilon() * 0.5;
constexpr double orient2d_error_bound = (3.0 + 16.0 * epsilon) * epsilon;
constexpr double orient3d_error_bound = (7.0 + 56.0 * epsilon) * epsilon;

PredicateSign sign_of(double value) {
    if (value > 0.0) {
        return PredicateSign::positive;
    }
    if (value < 0.0) {
        return PredicateSign::negative;
    }
    return PredicateSign::zero;
}

Expansion exact_sub(double first, double second) {
    return Expansion(first).sub(Expansion(second));
}

void require_finite(const Vector2& first, const Vector2& second, const Vector2& third) {
    if (!first.is_finite() || !second.is_finite() || !third.is_finite()) {
        throw std::invalid_argument("orient2d arguments must be finite");
    }
}

void require_finite(const Point3& first, const Point3& second, const Point3& third, const Point3& fourth) {
    if (!first.is_finite() || !second.is_finite() || !third.is_finite() || !fourth.is_finite()) {
        throw std::invalid_argument("orient3d arguments must be finite");
    }
}

} // namespace

PredicateSign orient2d(const Vector2& first, const Vector2& second, const Vector2& third) {
    require_finite(first, second, third);

    const double first_x  = first.x()  - third.x();
    const double first_y  = first.y()  - third.y();
    const double second_x = second.x() - third.x();
    const double second_y = second.y() - third.y();

    const double left  = first_x * second_y;
    const double right = first_y * second_x;

    const double determinant = left - right;
    const double permanent = std::abs(left) + std::abs(right);
    const double error = orient2d_error_bound * permanent;

    if (determinant > error || determinant < -error) {
        return sign_of(determinant);
    }

    const Expansion exact_first_x  = exact_sub(first.x(), third.x());
    const Expansion exact_first_y  = exact_sub(first.y(), third.y());
    const Expansion exact_second_x = exact_sub(second.x(), third.x());
    const Expansion exact_second_y = exact_sub(second.y(), third.y());

    return exact_first_x.mul(exact_second_y).sub(exact_first_y.mul(exact_second_x)).sign();
}

PredicateSign orient3d(const Point3& first, const Point3& second, const Point3& third, const Point3& fourth) {
    require_finite(first, second, third, fourth);

    const double first_x  = first.x()  - fourth.x();
    const double first_y  = first.y()  - fourth.y();
    const double first_z  = first.z()  - fourth.z();
    const double second_x = second.x() - fourth.x();
    const double second_y = second.y() - fourth.y();
    const double second_z = second.z() - fourth.z();
    const double third_x  = third.x()  - fourth.x();
    const double third_y  = third.y()  - fourth.y();
    const double third_z  = third.z()  - fourth.z();

    const double second_x_third_y = second_x * third_y;
    const double third_x_second_y = third_x  * second_y;
    const double third_x_first_y  = third_x  * first_y;
    const double first_x_third_y  = first_x  * third_y;
    const double first_x_second_y = first_x  * second_y;
    const double second_x_first_y = second_x * first_y;

    const double determinant =
          first_z  * (second_x_third_y - third_x_second_y)
        - second_z * (first_x_third_y  - third_x_first_y)
        + third_z  * (first_x_second_y - second_x_first_y);

    const double permanent =
          (std::abs(second_x_third_y) + std::abs(third_x_second_y)) * std::abs(first_z)
        + (std::abs(third_x_first_y)  + std::abs(first_x_third_y))  * std::abs(second_z)
        + (std::abs(first_x_second_y) + std::abs(second_x_first_y)) * std::abs(third_z);

    const double error = orient3d_error_bound * permanent;

    if (determinant > error || determinant < -error) {
        return sign_of(determinant);
    }

    const Expansion exact_first_x  = exact_sub(first.x(),  fourth.x());
    const Expansion exact_first_y  = exact_sub(first.y(),  fourth.y());
    const Expansion exact_first_z  = exact_sub(first.z(),  fourth.z());
    const Expansion exact_second_x = exact_sub(second.x(), fourth.x());
    const Expansion exact_second_y = exact_sub(second.y(), fourth.y());
    const Expansion exact_second_z = exact_sub(second.z(), fourth.z());
    const Expansion exact_third_x  = exact_sub(third.x(),  fourth.x());
    const Expansion exact_third_y  = exact_sub(third.y(),  fourth.y());
    const Expansion exact_third_z  = exact_sub(third.z(),  fourth.z());

    const Expansion first_minor  = exact_second_x.mul(exact_third_y).sub(exact_third_x.mul(exact_second_y));
    const Expansion second_minor = exact_first_x.mul(exact_third_y).sub(exact_third_x.mul(exact_first_y));
    const Expansion third_minor  = exact_first_x.mul(exact_second_y).sub(exact_second_x.mul(exact_first_y));

    return exact_first_z.mul(first_minor).sub(exact_second_z.mul(second_minor)).add(exact_third_z.mul(third_minor)).sign();
}

} // namespace geometry
