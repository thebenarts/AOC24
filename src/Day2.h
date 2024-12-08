#pragma once
#include "Utility.h"
#include "Day.h"

#include <functional>
#include <span>

namespace day2::helper
{
    template<utility::IsInt32Range T, utility::IsOrderingOperation O>
    bool IsFollowingOrdering(T record, O orderingOperation)
    {
        for (const auto [data1, data2] : record | std::ranges::views::adjacent<2>)
        {
            if (orderingOperation(data1, data2))
            {
                return false;
            }
        }

        return true;
    }

    template<utility::IsInt32Range T>
    bool IsInAscendingOrder(T record)
    {
        return IsFollowingOrdering(record, std::greater<>{});
    }

    template<utility::IsInt32Range T>
    bool IsInDescendingOrder(T record)
    {
        return IsFollowingOrdering(record, std::less<>{});
    }

    template<utility::IsInt32Range T>
    bool SatisfiesOrderingRequirements(T record)
    {
        return IsInAscendingOrder(record) || IsInDescendingOrder(record);
    }

    template<utility::IsInt32Range T>
    bool SatisfiesDifferenceLimitOfAdjacentElements(T record, int32_t minimumDifference = 1, int32_t maximumDifference = 3)
    {
        for (const auto [data1, data2] : record | std::ranges::views::adjacent<2>)
        {
            int32_t difference{ std::abs(data1 - data2) };
            if (difference < minimumDifference || difference > maximumDifference)
            {
                return false;
            }
        }

        return true;
    }

    template<utility::IsInt32Range T>
    bool Validate(T report)
    {
        return SatisfiesOrderingRequirements(report) && SatisfiesDifferenceLimitOfAdjacentElements(report);
    }

    struct Dampener
    {
        template<utility::IsInt32Range T>
        bool operator()(T report)
        {
            const auto reportSize{ report.size() };
            for (int i = 0; i < reportSize; i++)
            {
                auto filteredView{ report | std::ranges::views::filter([i, report](auto&& value) { return &value != &report[i]; }) };
                if (Validate(filteredView))
                {
                    return true;
                }
            }

            return false;
        }

    };

    template<utility::IsInt32Range T, typename D>
    bool Validate(T report, D dampener)
    {
        return dampener(report);
    }
}

template<utility::InputVersion version = utility::InputVersion::release>
class Day2 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day2" };

private:

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day2, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto numbersInString : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            auto& report{ mReports.emplace_back() };
            for (const auto value : numbersInString | std::ranges::views::split(" "sv)
                | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; })
                )
            {
                report.push_back(utility::ToNumber(value));
            }
        }
    }

    void PerformFirst() override
    {
        int32_t result{ static_cast<int32_t>(std::ranges::count_if(mReports,
            [this](std::span<int32_t> report) { return day2::helper::Validate(report); })) };
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        int32_t result{ static_cast<int32_t>(std::ranges::count_if(mReports,
            [this](std::span<int32_t> report) { return day2::helper::Validate(report, day2::helper::Dampener{}); })) };
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << "\n";
    }

private:
    std::string mBuffer;
    std::vector<std::vector<int32_t>> mReports;
};
