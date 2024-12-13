#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>

template<utility::InputVersion version = utility::InputVersion::release>
class Day8 : public DayBase<version>
{
    using Number = int32_t;
    using Field = char;
    using Position = utility::Position<Number>;

    struct AntinodeOffset
    {
        Position mPosition;
        Position mOffset;
    };

    struct ScratchData
    {
        std::vector<Position> mAntinodePositions;
    };

public:
    static constexpr std::string_view sDay{ "day8" };

private:

    std::vector<AntinodeOffset> CalculateAntinodeOffsets(Position position1, Position position2)
    {
        std::vector<AntinodeOffset> result;
        Position offsetForPos1{ position1 - position2 };
        Position offsetForPos2{ position2 - position1 };
        result.emplace_back(position1, offsetForPos1);
        result.emplace_back(position2, offsetForPos2);
        return result;
    }

    std::vector<AntinodeOffset> CalculateAntinodeOffsets(std::span<const Position> positions)
    {
        std::vector<AntinodeOffset> result;
        for (const auto& [currentIndex, position1] : positions | std::ranges::views::enumerate)
        {
            for (const auto& position2 : std::ranges::subrange(positions.begin() + currentIndex + 1, positions.end()))
            {
                auto currentResult{ CalculateAntinodeOffsets(position1, position2) };
                result.insert(result.end(), std::make_move_iterator(currentResult.begin()), std::make_move_iterator(currentResult.end()));
            }
        }

        return result;
    }

    void GatherAntinodeOffsets(Field field, std::span<const Position> positions)
    {
        mFieldAntinodeOffsets.emplace(field, CalculateAntinodeOffsets(positions));
    }

    std::vector<Position> CalculateAntinodes(AntinodeOffset antinodeOffset)
    {
        std::vector<Position> result;
        Position currentAntinodePosition{ antinodeOffset.mPosition };
        while (IsPositionInBounds(currentAntinodePosition))
        {
            result.push_back(currentAntinodePosition);
            currentAntinodePosition += antinodeOffset.mOffset;
        }

        return result;
    }

    std::vector<Position> CalculateAntinodes(std::span<const AntinodeOffset> antinodeOffsets)
    {
        std::vector<Position> result;
        for (const auto& antinodeOffset : antinodeOffsets)
        {
            auto currentResult{ CalculateAntinodes(antinodeOffset) };
            result.insert(result.end(), std::make_move_iterator(currentResult.begin()), std::make_move_iterator(currentResult.end()));
        }

        return result;
    }

    void GatherAntinodePositions(std::span<const AntinodeOffset> antinodeOffsets, ScratchData& scratchData)
    {
        auto antinodePositions{ CalculateAntinodes(antinodeOffsets) };
        scratchData.mAntinodePositions.insert(scratchData.mAntinodePositions.end(), std::make_move_iterator(antinodePositions.begin()), std::make_move_iterator(antinodePositions.end()));
    }

    bool IsPositionInBounds(Position position)
    {
        if (position.mRow < 0 || position.mRow >= mData.size() || position.mCol < 0 || position.mCol >= mData[position.mRow].size())
        {
            return false;
        }

        return true;
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day8, version> inputReader;
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

                row.emplace_back(character);
                if (character != '.')
                {
                    const auto [iterator, _] {mFieldPositions.try_emplace(character)};
                    iterator->second.push_back(Position{ static_cast<Number>(index), static_cast<Number>(row.size() - 1) });
                }
            }
        }

        for (const auto& [field, positions] : mFieldPositions)
        {
            GatherAntinodeOffsets(field, positions);
        }
    }

    void PerformFirst() override
    {
        Number result{};
        std::vector<Position> uniquePositions;
        for (auto& [field, antinodeOffsets] : mFieldAntinodeOffsets)
        {
            for (auto& [fieldPosition, antinodeOffset] : antinodeOffsets)
            {
                auto antinodePosition{ fieldPosition + antinodeOffset };
                if (IsPositionInBounds(antinodePosition))
                {
                    uniquePositions.emplace_back(antinodePosition);
                }
            }
        }
        std::ranges::sort(uniquePositions, std::less{});
        const auto [eraseFirst, eraseLast] {std::ranges::unique(uniquePositions)};
        uniquePositions.erase(eraseFirst, eraseLast);
        result = uniquePositions.size();
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        Number result{};
        ScratchData scracthData;
        for (auto& [field, antinodeOffsets] : mFieldAntinodeOffsets)
        {
            for (auto& antinodeOffset : antinodeOffsets)
            {
                GatherAntinodePositions(antinodeOffsets, scracthData);
            }
        }
        auto& uniquePositions{ scracthData.mAntinodePositions };
        std::ranges::sort(uniquePositions, std::less{});
        const auto [eraseFirst, eraseLast] {std::ranges::unique(uniquePositions)};
        uniquePositions.erase(eraseFirst, eraseLast);
        result = uniquePositions.size();
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::vector<std::vector<Field>> mData;  // Turned out to be unnecessary. could be replaced with width and height.
    std::unordered_map<Field, std::vector<Position>> mFieldPositions;
    std::unordered_map<Field, std::vector<AntinodeOffset>> mFieldAntinodeOffsets;
};
