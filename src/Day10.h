#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>
#include <variant>
#include <optional>

template<utility::InputVersion version = utility::InputVersion::release>
class Day10 : public DayBase<version>
{
    using Number = uint64_t;
    using PositionIndex = int32_t;
    using Height = int8_t;
    using Position = utility::Position<PositionIndex>;

    struct ScratchData
    {
        std::vector<Position> mFoundPeeks;
    };

public:
    static constexpr std::string_view sDay{ "day10" };

private:

    template<bool starting = false>
    void TraverseTrail(Position position, utility::DirectionData fromDirection, Height previousHeight, ScratchData& scratchData, PositionIndex maximumSlope = 1)
    {
        static constexpr Position sEmptyDirection{ 0,0 };
        auto currentHeight{ utility::GetItemAt(mData,position) };
        Height currentHeightValue{};
        if (currentHeight.has_value())
        {
            currentHeightValue = *currentHeight.value();
            if (!starting && previousHeight + maximumSlope != currentHeightValue)
            {
                return;
            }
            if (currentHeightValue == static_cast<int8_t>(9))
            {
                scratchData.mFoundPeeks.emplace_back(position);
                return;
            }
        }
        else
        {
            // Out of bounds
            return;
        }

        for (auto [direction, offset] : utility::sBaseDirectionsMap)
        {
            // optional implicitly checked
            if (fromDirection == offset)
            {
                continue;
            }
            auto oppositeOffset{ offset * -1 };

            TraverseTrail<false>(position + offset, oppositeOffset, currentHeightValue, scratchData, maximumSlope);
        }
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day10, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto [index, rowInput] : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }) | std::ranges::views::enumerate)
        {
            mData.emplace_back();
            auto& row{ mData.back() };
            row.reserve(rowInput.size());
            for (const auto character : rowInput)
            {
                if (character == '\0')
                {
                    continue;
                }

                row.emplace_back(utility::ToNumber<Height>(character));
                if (character == '0')
                {
                    mTrailheads.push_back(Position{ static_cast<PositionIndex>(index), static_cast<PositionIndex>(row.size() - 1) });
                }
            }
        }
    }

    void PerformFirst() override
    {
        std::vector<ScratchData> scratchDatas(mTrailheads.size());
        for (auto&& [position, scratchData] : std::ranges::views::zip(mTrailheads, scratchDatas))
        {
            TraverseTrail<true>(position, Position{ 0,0 }, 0, scratchData);
        }
        Number result{};
        for (auto& scratchData : scratchDatas)
        {
            std::ranges::sort(scratchData.mFoundPeeks, std::less<>{});
            const auto [firstToErase, lastToErase] {std::ranges::unique(scratchData.mFoundPeeks)};
            scratchData.mFoundPeeks.erase(firstToErase, lastToErase);
            result += scratchData.mFoundPeeks.size();
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        std::vector<ScratchData> scratchDatas(mTrailheads.size());
        for (auto&& [position, scratchData] : std::ranges::views::zip(mTrailheads, scratchDatas))
        {
            TraverseTrail<true>(position, Position{ 0,0 }, 0, scratchData);
        }
        Number result{};
        for (auto& scratchData : scratchDatas)
        {
            result += scratchData.mFoundPeeks.size();
        }
        std::cout << result << '\n';
        utility::PrintDetails(version, utility::Part::second);
    }

private:
    std::string mBuffer;
    std::vector<std::vector<Height>> mData;
    std::vector<Position> mTrailheads;
};
