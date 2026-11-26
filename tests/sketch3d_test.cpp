#include "sketch3d.h"

#include <cstdlib>
#include <limits>

namespace {

void Require(bool condition) {
    if (!condition) std::abort();
}

}  // namespace

int main() {
    using pipes::Point3;
    using pipes::Sketch3d;
    using Edit = Sketch3d::Edit;

    Sketch3d draft;
    const Point3 origin{0, 0, 0};
    const Point3 x{10, 0, 0};
    const Point3 xy{10, 20, 0};
    const Point3 xyz{10, 20, 30};

    Require(!draft.HasSegments());
    Require(draft.Insert(1, origin) == Edit::invalid_index);
    Require(draft.Insert(0, origin) == Edit::ok);
    Require(!draft.HasSegments());
    Require(draft.Insert(1, origin) == Edit::zero_length_segment);
    Require(draft.Insert(1, xyz) == Edit::ok);
    Require(draft.Insert(1, x) == Edit::ok);
    Require(draft.Insert(2, xy) == Edit::ok);
    Require(draft.HasSegments() && draft.Points().size() == 4);
    Require(draft.Points()[1] == x && draft.Points()[2] == xy && draft.Points().back() == xyz);

    const auto inf = std::numeric_limits<double>::infinity();
    Require(draft.Replace(0, {inf, 0, 0}) == Edit::invalid_point);
    Require(draft.Replace(1, xy) == Edit::zero_length_segment);
    Require(draft.Points()[1] == x);
    Require(draft.Replace(3, x) == Edit::ok);
    Require(draft.Erase(2) == Edit::zero_length_segment);
    Require(draft.Points().size() == 4 && draft.Points()[2] == xy);
    Require(draft.Erase(3) == Edit::ok);
    Require(draft.Erase(0) == Edit::ok);
    Require(draft.Erase(0) == Edit::ok);
    Require(!draft.HasSegments() && draft.Points().front() == xy);
    Require(draft.Erase(1) == Edit::invalid_index);
}
