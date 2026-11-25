#include "street.h"

const std::string& rgen::Street::name() const
{
    return name_;
}

const std::vector<rgen::Point>& rgen::Street::points() const
{
    return points_;
}
