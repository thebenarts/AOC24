#pragma once
#include "Utility.h"
#include "Day.h"

#include <iostream>
#include <ranges>

template<utility::InputVersion version = utility::InputVersion::release>
class Day6 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day6" };

private:
    void ReadInput() override
    {
        using namespace std::literals;
    };

private:
    void PerformFirst() override {};
    void PerformSecond() override {};
};
