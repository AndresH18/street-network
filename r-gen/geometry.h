//
// Created by andre on 11/11/2026.
//

#ifndef ECE650_A4_GEOMETRY_H
#define ECE650_A4_GEOMETRY_H

#include <vector>

#include "point.h"
#include "segment.h"

namespace rgen {
    enum class Orientation {
        Collinear = 0,
        Clockwise = -1,
        CounterClockwise = 1,
    };

    /**
     * @brief Calculates the @c Orientation between three Points
     * @param a
     * @param b
     * @param c
     * @return
     */
    [[nodiscard]]
    Orientation orientation(const Point& a, const Point& b, const Point& c);

    /**
     * @brief Checks if a point @p p is located on @p segment.
     *
     * @remark Considers that the @p segment and the @p point are @ref Orientation::Collinear
     *
     * @param p The point to check if is on the segment
     * @param segment The segment to check
     * @return returns @c true if the point is on the segment, @c false otherwise
     */
    [[nodiscard]]
    bool point_on_segment(const Point& p, const Segment& segment);

    /**
     * @brief Checks if two segments overlap.
     * @param first The first Segment
     * @param second The other Segment
     * @return Returns @c true if the segments overlap, @c false otherwise.
     */
    [[nodiscard]]
    bool segments_overlap(const Segment& first, const Segment& second);

    /**
     * @brief Checks if the @p candidate @ref Segment overlaps any Segment from @p segments
     * @param candidate The Segment to test
     * @param segments The segments to test with
     * @return Returns true if any Segment from @p segments overlap with @p candidate; @c false otherwise
     */
    [[nodiscard]]
    bool overlaps_any(const Segment& candidate, const std::vector<Segment>& segments);
} // namespace rgen

#endif // ECE650_A4_GEOMETRY_H
