#include "sketch3d.h"

#include <cmath>

namespace pipes {

namespace {

bool IsFinite(Point3 point) {
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

}  // namespace

Sketch3d::Edit Sketch3d::Insert(std::size_t index, Point3 point) {
    if (index > points_.size()) return Edit::invalid_index;
    if (!IsFinite(point)) return Edit::invalid_point;
    if ((index > 0 && points_[index - 1] == point) ||
        (index < points_.size() && points_[index] == point)) {
        return Edit::zero_length_segment;
    }

    points_.insert(points_.begin() + static_cast<std::ptrdiff_t>(index), point);
    return Edit::ok;
}

Sketch3d::Edit Sketch3d::Replace(std::size_t index, Point3 point) {
    if (index >= points_.size()) return Edit::invalid_index;
    if (!IsFinite(point)) return Edit::invalid_point;
    if ((index > 0 && points_[index - 1] == point) ||
        (index + 1 < points_.size() && points_[index + 1] == point)) {
        return Edit::zero_length_segment;
    }

    points_[index] = point;
    return Edit::ok;
}

Sketch3d::Edit Sketch3d::Erase(std::size_t index) {
    if (index >= points_.size()) return Edit::invalid_index;
    if (index > 0 && index + 1 < points_.size() && points_[index - 1] == points_[index + 1]) {
        return Edit::zero_length_segment;
    }

    points_.erase(points_.begin() + static_cast<std::ptrdiff_t>(index));
    return Edit::ok;
}

}  // namespace pipes
