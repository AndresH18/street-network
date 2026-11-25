//
// Created by andre on 11/11/2026.
//

#ifndef ECE650_A4_GENERATOR_H
#define ECE650_A4_GENERATOR_H

#include <vector>
#include <optional>

#include "point.h"
#include "segment.h"
#include "street.h"

namespace rgen {
    /**
     * Maximum number of times to create a valid segment
     */
    constexpr int MAX_SEGMENT_RETRY_COUNT = 25;
    /**
     * Maximum number of times to attempt to create a valid Street specification
     */
    constexpr int MAX_SPECIFICATION_RETRY_COUNT = 25;

    constexpr int MIN_STREETS = 2;
    constexpr int MIN_SEGMENTS = 1;
    constexpr int MIN_COORDINATE_LIMIT = 1;
    constexpr int MIN_WAIT = 5;
    //
    constexpr int DEFAULT_MAX_STREETS = 10;
    constexpr int DEFAULT_MAX_SEGMENTS = 5;
    constexpr int DEFAULT_MAX_COORDINATE_LIMIT = 20;
    constexpr int DEFAULT_MAX_WAIT = 10;


    /**
     * @brief Generates a new @c Point with random coordinates within range [-@p coordinate_limit, +@p coordinate_limit]
     * @param coordinate_limit Absolute range for the coordinates of the new @c Point
     * @return Returns a new @c Point
     */
    Point generate_point(int coordinate_limit);

    /**
     * Generates a new @c Segment, which does not overlap with any of the Segments in @p existing_segments. If
     * after @c MAX_SEGMENT_RETRY_COUNT attempts, a new segment cannot be made, returns @c std::nullopt
     * @param start The starting point of the new @c Segment
     * @param coordinate_limit Absolute range for the coordinates of the new Segment's Point
     * @param existing_segments The existing segments
     * @return Returns a new Segment or @c std::nullopt if it cannot create a valid Segment
     */
    std::optional<Segment> generate_valid_segment(const Point& start,
                                                  int coordinate_limit,
                                                  const std::vector<Segment>& existing_segments);

    /**
     * @brief Generates a new @c Street that does not overlap with any previous @c Segment
     * @param name the street name
     * @param segment_count The number of segments that the new street must have
     * @param coordinate_limit Absolute range for the Street Points' coordinates
     * @param all_segments All previously created segments
     * @return Returns a new Street or @c std::nullopt if it cannot create a valid Street
     */
    std::optional<Street> generate_street(const std::string& name,
                                          int segment_count,
                                          int coordinate_limit,
                                          std::vector<Segment>& all_segments);

    /**
     * @brief Generates a new Street specification.
     * @param max_streets Maximum number of Streets to generate
     * @param max_segments Maximum number of Segments in each Street
     * @param coordinate_limit Absolute range for the Street Points' coordinates
     * @return Returns a @c std::vector<Street> or @c std::nullopt if if cannot generate the a a street
     */
    std::optional<std::vector<Street> >
    generate_specification(int max_streets, int max_segments, int coordinate_limit);

    /**
     * @brief Generates a new Street specification. Unlike #generate_street this function throws if it cannot generate a
     * valid specification after @c MAX_SPECIFICATION_RETRY_COUNT attempts.
     * @return Returns a new Street Specification
     */
    std::vector<Street> generate_valid_specification();
} // namespace rgen
#endif // ECE650_A4_GENERATOR_H
