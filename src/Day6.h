#pragma once
#include "Utility.h"
#include "Day.h"

#include <iostream>
#include <ranges>

//class Day6 : public AbstractDay
//{
//public:
//    static constexpr std::string_view sDay{ "day6" };
//
//    void Perform(utility::Part part)
//    {
//        utility::InputReader<Day6> inputReader;
//        const auto input{ inputReader.Read() };
//        std::cout << input;
//        const auto splitInput{ utility::GetStringSplitBy(input) };
//        // If this breaks, it means it's not using working draft version. (requires c++ 23)
//        for (const auto [index, line] : splitInput | std::ranges::views::enumerate)
//        {
//            std::cout << '\n' << index << " " << line;
//        }
//    }
//
//private:
//    void PerformFirst() {};
//    void PerformSecond() {};
//};
