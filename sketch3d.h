#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace pipes {

// Points are in millimetres in the assembly coordinate system.
struct Point3 {
    double x;
    double y;
    double z;
    bool operator==(const Point3&) const = default;
};

// The manual route draft. Adjacent points define straight segments.
class Sketch3d {
public:
    enum class Edit { ok, invalid_index, invalid_point, zero_length_segment };

    Edit Insert(std::size_t index, Point3 point);
    Edit Replace(std::size_t index, Point3 point);
    Edit Erase(std::size_t index);

    [[nodiscard]] std::span<const Point3> Points() const { return points_; }
    [[nodiscard]] bool HasSegments() const { return points_.size() >= 2; }

private:
    std::vector<Point3> points_;
};

}  // namespace pipes
