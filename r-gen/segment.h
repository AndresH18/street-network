//
// Created by andre on 11/11/2026.
//

#ifndef ECE650_A4_SEGMENT_H
#define ECE650_A4_SEGMENT_H

namespace rgen {

    /**
     * @class Segment
     * @brief Represents a line segment defined by two endpoints in a 2D Cartesian coordinate system.
     *
     * @details
     * A segment is characterized using two endpoints represented as (x1, y1)
     * and (x2, y2). It can be used in various geometric computations, such as
     * distance calculations and determining intersections.
     */
    class Segment {
        // std::shared_ptr<Point> start_point;
        // std::shared_ptr<Point> end_point;
        Point start_;
        Point end_;

      public:
        // Segment(std::shared_ptr<Point> start_point, std::shared_ptr<Point> end_point);
        [[nodiscard]] Segment(const Point& start, const Point& end)
            : start_(start), end_(end)
        {
        }

        [[nodiscard]] bool is_zero_length() const
        {
            return start_ == end_;
        }

        [[nodiscard]] const Point& start() const
        {
            return start_;
        }

        [[nodiscard]] const Point& end() const
        {
            return end_;
        }

        friend bool operator==(const Segment& lhs, const Segment& rhs)
        {
            return lhs.start_ == rhs.start_ && lhs.end_ == rhs.end_;
        }

        friend bool operator!=(const Segment& lhs, const Segment& rhs)
        {
            return !(lhs == rhs);
        }
    };

} // namespace rgen

#endif // ECE650_A4_SEGMENT_H
