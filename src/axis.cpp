// Copyright 2015-2016 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/histogram/axis.hpp>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>

namespace boost {
namespace histogram {

namespace {
    std::string escape(const std::string& s) {
        std::ostringstream os;
        os << '\'';
        for (unsigned i = 0; i < s.size(); ++i) {
            const char c = s[i];
            if (c == '\'' && (i == 0 || s[i - 1] != '\\'))
                os << "\\\'";
            else
                os << c;
        }
        os << '\'';
        return os.str();
    }
}

axis_base::axis_base(unsigned size,
                     const std::string& label,
                     bool uoflow) :
    size_(size),
    uoflow_(uoflow),
    label_(label)
{}

bool axis_base::operator==(const axis_base& o) const
{ return size_ == o.size_ && label_ == o.label_; }

regular_axis::regular_axis(unsigned n, double min, double max,
                           const std::string& label, bool uoflow):
    axis_base(n, label, uoflow),
    min_(min),
    delta_((max - min) / n)
{
    if (min >= max)
        throw std::logic_error("regular_axis: min must be less than max");
}

double
regular_axis::operator[](int idx)
    const
{
    if (idx < 0)
        return -std::numeric_limits<double>::infinity();
    if (idx > bins())
        return std::numeric_limits<double>::infinity();
    const double z = double(idx) / bins();
    return (1.0 - z) * min_ + z * (min_ + delta_ * bins());
}

bool
regular_axis::operator==(const regular_axis& o) const
{
    return axis_base::operator==(o) &&
           min_ == o.min_ &&
           delta_ == o.delta_;
}

polar_axis::polar_axis(unsigned n, double start,
                       const std::string& label) :
    axis_base(n, label, false),
    start_(start)
{}

double
polar_axis::operator[](int idx)
    const
{
    using namespace boost::math::double_constants;
    const double z = double(idx) / bins();
    return z * two_pi + start_;
}

bool
polar_axis::operator==(const polar_axis& o) const
{
    return axis_base::operator==(o) &&
           start_ == o.start_;
}

variable_axis::variable_axis(const variable_axis& o) :
    axis_base(o),
    x_(new double[bins() + 1])
{
    std::copy(o.x_.get(), o.x_.get() + bins() + 1, x_.get());
}

variable_axis&
variable_axis::operator=(const variable_axis& o)
{
    if (this != &o) {
        axis_base::operator=(o);
        x_.reset(new double[bins() + 1]);
        std::copy(o.x_.get(), o.x_.get() + bins() + 1, x_.get());
    }
    return *this;
}

double
variable_axis::operator[](int idx)
    const
{
    if (idx < 0)
        return -std::numeric_limits<double>::infinity();
    if (idx > bins())
        return std::numeric_limits<double>::infinity();
    return x_[idx];
}

bool
variable_axis::operator==(const variable_axis& o) const
{
    if (!axis_base::operator==(o))
        return false;
    for (unsigned i = 0, n = bins() + 1; i < n; ++i)
        if (x_[i] != o.x_[i])
            return false;
    return true;
}

category_axis::category_axis(const std::initializer_list<std::string>& c) :
    categories_(c)
{}

bool
category_axis::operator==(const category_axis& o) const
{ return categories_ == o.categories_; }

integer_axis::integer_axis(int min, int max,
                           const std::string& label,
                           bool uoflow) :
    axis_base(max + 1 - min, label, uoflow),
    min_(min)
{}

bool
integer_axis::operator==(const integer_axis& o) const
{
    return axis_base::operator==(o) &&
           min_ == o.min_;
}

std::ostream& operator<<(std::ostream& os, const regular_axis& a)
{
    os << "regular_axis(" << a.bins() << ", " << a[0] << ", " << a[a.bins()];
    if (!a.label().empty())
        os << ", label=" << escape(a.label());
    if (!a.uoflow())
        os << ", uoflow=False";
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const polar_axis& a)
{
    os << "polar_axis(" << a.bins();
    if (a[0] != 0.0)
        os << ", " << a[0];
    if (!a.label().empty())
        os << ", label=" << escape(a.label());
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const variable_axis& a)
{
    os << "variable_axis(" << a[0];
    for (int i = 1; i <= a.bins(); ++i)
        os << ", " << a.left(i);
    if (!a.label().empty())
        os << ", label=" << escape(a.label());
    if (!a.uoflow())
        os << ", uoflow=False";
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const integer_axis& a)
{
    os << "integer_axis(" << a[0] << ", " << a[a.bins() - 1];
    if (!a.label().empty())
        os << ", label=" << escape(a.label());
    if (!a.uoflow())
        os << ", uoflow=False";
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const category_axis& a)
{
    os << "category_axis(";
    for (int i = 0; i < a.bins(); ++i)
        os << escape(a[i])  << (i == (a.bins() - 1)? ")" : ", ");
    return os;
}

}
}
