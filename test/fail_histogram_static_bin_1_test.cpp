#include <boost/histogram.hpp>
#include <vector>

using namespace boost::histogram;
int main() {
  auto h = make_static_histogram(axis::integer<>(0, 2),
                                 axis::integer<>(0, 2));
  h.bin(std::vector<int>(-2, 0));
}
