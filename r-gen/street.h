
#ifndef ECE650_A4_STREET_H
#define ECE650_A4_STREET_H

#include <string>
#include <vector>

#include "point.h"

namespace rgen {
    class Street {
        const std::string name_;
        const std::vector<Point> points_;

    public:
        explicit Street(std::string street_name, std::vector<Point> points)
            : name_{std::move(street_name)}, points_{std::move(points)} {
        }

        [[nodiscard]] const std::string& name() const;

        [[nodiscard]] const std::vector<Point>& points() const;
    };
}

#endif
