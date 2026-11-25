//
// Created by andre on 11/11/2026.
//

#include "generator.h"

#include <atomic>
#include <format>
#include <iostream>
#include <string_view>
#include <thread>

#include "geometry.h"
#include "random.h"

using namespace rgen;

static std::atomic_bool running;

namespace rgen {

} // namespace

Point rgen::generate_point(const int coordinate_limit)
{
    const int x = random::next_int(-coordinate_limit, coordinate_limit);
    const int y = random::next_int(-coordinate_limit, coordinate_limit);

    return {x, y};
}

std::optional<Segment> rgen::generate_valid_segment(const Point& start,
                                                    const int coordinate_limit,
                                                    const std::vector<Segment>& existing_segments)
{
    for (int attempt = 0; attempt < MAX_SEGMENT_RETRY_COUNT; ++attempt) {
        Point end = generate_point(coordinate_limit);

        if (start == end) {
            continue;
        }

        if (const Segment candidate(start, end); !overlaps_any(candidate, existing_segments)) {
            return candidate;
        }
    }

    return std::nullopt;
}

std::optional<Street> rgen::generate_street(const std::string& name,
                                            const int segment_count,
                                            const int coordinate_limit,
                                            std::vector<Segment>& all_segments)
{
    std::vector<Point> points;
    points.reserve(segment_count + 1);

    Point current_point = generate_point(coordinate_limit);
    points.push_back(current_point);

    for (int i = 0; i < segment_count; ++i) {
        auto segment = generate_valid_segment(current_point, coordinate_limit, all_segments);

        if (!segment.has_value()) {
            return std::nullopt;
        }

        all_segments.push_back(*segment);

        current_point = segment->end();
        points.push_back(current_point);
    }

    return Street{name, std::move(points)};
}

std::optional<std::vector<Street>>
rgen::generate_specification(const int max_streets, const int max_segments, const int coordinate_limit)
{
    const int street_count = random::next_int(MIN_STREETS, max_streets);

    std::vector<Street> streets;
    std::vector<Segment> all_segments;

    streets.reserve(street_count);

    for (int i = 0; i < street_count; ++i) {
        const int segment_count = random::next_int(MIN_SEGMENTS, max_segments);

        const std::string name = "street" + std::to_string(i);

        const auto street = generate_street(name, segment_count, coordinate_limit, all_segments);

        if (!street) {
            return std::nullopt;
        }

        streets.push_back(*street);
    }

    return streets;
}

std::vector<Street> rgen::generate_valid_specification()
{
    for (int attempt = 0; attempt < MAX_SPECIFICATION_RETRY_COUNT; ++attempt) {
        const auto specification = generate_specification(DEFAULT_MAX_STREETS,
                                                          DEFAULT_MAX_SEGMENTS,
                                                          DEFAULT_MAX_COORDINATE_LIMIT);

        if (specification) {
            return *specification;
        }
    }

    throw std::runtime_error(std::format("Failed to generate valid input for {} simultaneous attempts",
                                         MAX_SPECIFICATION_RETRY_COUNT));
}
