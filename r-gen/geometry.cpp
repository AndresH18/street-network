//
// Created by andre on 11/11/2026.
//

#include "geometry.h"

#include <algorithm>
#include <vector>

rgen::Orientation rgen::orientation(const Point& a, const Point& b, const Point& c)
{
    // use long long to avoid arithmetic constraint
    const long long value = static_cast<long long>(b.x() - a.x()) * (c.y() - a.y()) -
                            static_cast<long long>(b.y() - a.y()) * (c.x() - a.x());

    if (value == 0)
        return Orientation::Collinear;
    if (value > 0)
        return Orientation::CounterClockwise;
    return Orientation::Clockwise;
}

bool rgen::point_on_segment(const Point& p, const Segment& segment)
{
    const Point& start = segment.start();
    const Point& end = segment.end();

    if (orientation(start, end, p) != Orientation::Collinear)
        return false;

    const auto min_x = std::min(start.x(), end.x());
    const auto max_x = std::max(start.x(), end.x());

    const auto min_y = std::min(start.y(), end.y());
    const auto max_y = std::max(start.y(), end.y());

    const auto x = p.x(), y = p.y();

    return x >= min_x && x <= max_x && y >= min_y && y <= max_y;

    // return min_x <= p.get_x() && p.get_x() <= max_x &&
    //        min_y <= p.get_y() && p.get_y() <= max_y;
}

static bool intervals_overlap_with_positive_length(const int first_start,
                                                   const int first_end,
                                                   const int second_start,
                                                   const int second_end)
{
    const int first_min = std::min(first_start, first_end);
    const int first_max = std::max(first_start, first_end);

    const int second_min = std::min(second_start, second_end);
    const int second_max = std::max(second_start, second_end);

    const int overlap_start = std::max(first_min, second_min);
    const int overlap_end = std::min(first_max, second_max);

    return overlap_start < overlap_end; // if start and end are the same, that is not overlap
}

bool rgen::segments_overlap(const Segment& first, const Segment& second)
{
    const Point& a = first.start();
    const Point& b = first.end();

    const Point& c = second.start();
    const Point& d = second.end();

    if (orientation(a, b, c) != Orientation::Collinear || orientation(a, b, d) != Orientation::Collinear) {
        return false;
    }

    if (a.x() != b.x()) {
        return intervals_overlap_with_positive_length(a.x(), b.x(), c.x(), d.x());
    }

    return intervals_overlap_with_positive_length(a.y(), b.y(), c.y(), d.y());
}

bool rgen::overlaps_any(const Segment& candidate, const std::vector<Segment>& segments)
{
    // There is this better approach from ranges, directly accepts the vector
    return std::ranges::any_of(segments, [&candidate](const Segment& segment) {
        return segments_overlap(candidate, segment);
    });

    /* This is a nice way by using iterators
    return std::any_of(
        segments.begin(),
        segments.end(),
        [&candidate](const Segment& segment) {
            return segments_overlap(candidate, segment);
        }
    );
    */

    /* This is the traditional way
    for (const Segment& segment: segments) {
        if (segments_overlap(candidate, segment)) {
            return true;
        }
    }
    return false;
    */
}
