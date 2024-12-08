#pragma once
#include "Utility.h"
#include "Day.h"

#include <iostream>
#include <ranges>
#include <optional>

template<utility::InputVersion version = utility::InputVersion::release>
class Day1 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day1" };

private:
    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day1, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto numbersInString : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            for (const auto [index, value] : numbersInString | std::ranges::views::split("   "sv)
                | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; })
                | std::ranges::views::enumerate)
            {
                if (index == 0)
                {
                    mFirstNumbers.emplace_back(utility::ToNumber(value));
                }
                else if (index == 1)
                {
                    mSecondNumbers.emplace_back(utility::ToNumber(value));
                }
                else
                {
                    assert(false);
                }
            }
        }
        std::ranges::sort(mFirstNumbers, std::less<int32_t>{});
        std::ranges::sort(mSecondNumbers, std::less<int32_t>{});
    };

    void PerformFirst() override
    {
        int32_t result{};
        for (const auto [firstListElement, secondListElement] : std::ranges::views::zip(mFirstNumbers, mSecondNumbers))
        {
            result += std::abs(firstListElement - secondListElement);
        }

        utility::PrintDetails(version, utility::Part::first);
        std::cout << sFirstPartResultString << result << "\n";
    };

    void PerformSecond() override
    {
        int32_t result{};

        std::vector<int32_t>::iterator secondListNextLowerBoundIterator{ mSecondNumbers.begin() };
        std::optional<int32_t> currentNumber{};
        int32_t currentNumberCount{};
        for (const auto firstSideNumber : mFirstNumbers)
        {
            // optional == operator handles optional validity check.
            if (currentNumber == firstSideNumber)
            {
                result += currentNumberCount;
                continue;
            }

            currentNumber = firstSideNumber;
            auto [lowerBound, upperBound] {std::equal_range(secondListNextLowerBoundIterator, mSecondNumbers.end(), currentNumber)};
            currentNumberCount = currentNumber.value() * std::distance(lowerBound, upperBound);

            result += currentNumberCount;
            secondListNextLowerBoundIterator = upperBound;
        }

        utility::PrintDetails(version, utility::Part::second);
        std::cout << sSecondPartResultString << result << "\n";
    };

private:
    std::string mBuffer;
    std::vector<int32_t> mFirstNumbers;
    std::vector<int32_t> mSecondNumbers;
};
