"""Прототип проверки стыка трубы с неподвижным портом фитинга."""

from dataclasses import dataclass
from enum import Enum
from math import atan2, dist, hypot, isfinite

Point3 = tuple[float, float, float]


@dataclass(frozen=True)
class Port:
    center: Point3
    outward: Point3


class FitStatus(Enum):
    ALIGNED = "aligned"
    AUTO_ADJUST = "auto_adjust"
    NEEDS_CONFIRMATION = "needs_confirmation"
    OUT_OF_TOLERANCE = "out_of_tolerance"


@dataclass(frozen=True)
class PortFit:
    status: FitStatus
    gap_mm: float
    angle_rad: float
    target: Port | None


def unit_direction(vector: Point3) -> Point3:
    x, y, z = vector
    length = hypot(x, y, z)
    if not isfinite(length) or length == 0:
        raise ValueError("Port direction must be finite and nonzero")
    return x / length, y / length, z / length


def assess_port_fit(
    pipe: Port,
    fitting: Port,
    *,
    position_tolerance_mm: float,
    angle_tolerance_rad: float,
    automatic: bool,
) -> PortFit:
    """Оценивает подгонку конца трубы в глобальных координатах сборки."""
    tolerances = (position_tolerance_mm, angle_tolerance_rad)
    if any(not isfinite(value) or value < 0 for value in tolerances):
        raise ValueError("Tolerances must be finite and nonnegative")
    if not all(isfinite(coord) for coord in (*pipe.center, *fitting.center)):
        raise ValueError("Port centers must be finite")

    ax, ay, az = unit_direction(pipe.outward)
    fx, fy, fz = unit_direction(fitting.outward)
    bx, by, bz = -fx, -fy, -fz
    cross_norm = hypot(ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx)
    angle = atan2(cross_norm, ax * bx + ay * by + az * bz)
    gap = dist(pipe.center, fitting.center)

    if gap > position_tolerance_mm or angle > angle_tolerance_rad:
        return PortFit(FitStatus.OUT_OF_TOLERANCE, gap, angle, None)
    if gap == 0 and angle == 0:
        return PortFit(FitStatus.ALIGNED, gap, angle, None)

    target = Port(fitting.center, (bx, by, bz))
    status = FitStatus.AUTO_ADJUST if automatic else FitStatus.NEEDS_CONFIRMATION
    return PortFit(status, gap, angle, target)
