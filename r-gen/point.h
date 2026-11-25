//
// Created by andre on 11/11/2026.
//

#ifndef ECE650_A4_POINT_H
#define ECE650_A4_POINT_H

namespace rgen {

    /**
     * @class Point
     * @brief Represents a point in a 2D Cartesian coordinate system.
     *
     * @note This class assumes a Cartesian coordinate system.
     */
    class Point {
        int x_;
        int y_;

      public:
        Point(const int x, const int y)
            : x_(x), y_(y)
        {
        }

        [[nodiscard]] int x() const
        {
            return x_;
        }

        [[nodiscard]] int y() const
        {
            return y_;
        }

        friend bool operator==(const Point& lhs, const Point& rhs)
        {
            return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
        }

        friend bool operator!=(const Point& lhs, const Point& rhs)
        {
            return !(lhs == rhs);
        }
    };
} // namespace rgen

#endif // ECE650_A4_POINT_H
