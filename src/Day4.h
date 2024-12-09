#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <unordered_map>
#include <array>

namespace day4::helper
{
    enum class Direction
    {
        left,
        right,
        up,
        down,
        leftUp,
        leftDown,
        rightUp,
        rightDown,
    };
}

template<utility::InputVersion version = utility::InputVersion::release>
class Day4 : public DayBase<version>
{
public:

    static constexpr std::string_view sDay{ "day4" };
private:
    using FieldType = char;
    using DirectionData = std::pair<int32_t, int32_t>;
    using Direction = day4::helper::Direction;

    const std::unordered_map<Direction, int32_t> mHorizontalDirections
    {
        {Direction::left,   -1},
        {Direction::right,  1},
    };
    const std::unordered_map<Direction, int32_t> mVerticalDirections
    {
        {Direction::up,     -1},
        {Direction::down,   1},
    };

    const std::unordered_map<Direction, DirectionData> mDirectionsMap
    {
        {Direction::left,   {0,-1} },
        {Direction::right,  {0,1}},
        {Direction::up,     {-1, 0}},
        {Direction::down,   {1, 0}},
        {Direction::leftUp, {-1,-1}},
        {Direction::leftDown,{1,-1}},
        {Direction::rightDown, {1, 1}},
        {Direction::rightUp, {-1,1}},
    };

    static constexpr std::array<FieldType, 4> sCharacterOrder{ 'X', 'M', 'A', 'S' };

    static constexpr int32_t GetCharacterIndex(FieldType character)
    {
        const auto characterIterator{ std::ranges::find(sCharacterOrder,character) };
        if (characterIterator == sCharacterOrder.end())
        {
            assert(false);
            return 0;
        }

        return std::distance(sCharacterOrder.begin(), characterIterator);
    }

    static constexpr int32_t GetNextCharacterIndex(FieldType character)
    {
        return (GetCharacterIndex(character) + 1) % sCharacterOrder.size();
    }

    static constexpr FieldType GetNextCharacter(FieldType character)
    {
        return sCharacterOrder[GetNextCharacterIndex(character)];
    }

    static_assert(GetNextCharacter('X') == 'M');
    static_assert(GetNextCharacter('M') == 'A');
    static_assert(GetNextCharacter('A') == 'S');
    static_assert(GetNextCharacter('S') == 'X');

    int32_t GetNumberOfMatches(int32_t row, int32_t col, FieldType expectedCharacter = 'X', FieldType endCharacter = 'S')
    {
        if (row >= mData.size() || col >= mData[row].size())
        {
            return 0;
        }

        FieldType currentCharacter{ mData[row][col] };
        if (expectedCharacter != currentCharacter)
        {
            return 0;
        }

        const auto nextExpectedCharacter{ GetNextCharacter(expectedCharacter) };
        int32_t result{};
        for (const auto change : mDirectionsMap)
        {
            const auto direction{ change.first };
            const auto [verticalChange, horizontalChange] {change.second};
            if (GetMatch(row + verticalChange, col + horizontalChange, direction, nextExpectedCharacter, endCharacter))
            {
                ++result;
            }
        }

        return result;
    }

    bool GetMatch(int32_t row, int32_t col, Direction direction, FieldType expectedCharacter = 'X', FieldType endCharacter = 'S')
    {
        // Bounds check
        if (row >= mData.size() || col >= mData[row].size())
        {
            return 0;
        }

        FieldType currentCharacter{ mData[row][col] };
        if (expectedCharacter != currentCharacter)
        {
            return 0;
        }

        if (expectedCharacter == endCharacter)
        {
            return 1;
        }

        FieldType nextExpectedCharacter{ GetNextCharacter(expectedCharacter) };
        const auto& [verticalChange, horizontalChange] { mDirectionsMap.at(direction) };
        return GetMatch(row + verticalChange, col + horizontalChange, direction, nextExpectedCharacter, endCharacter);
    }

    bool GetDiagonalMatch(int32_t row, int32_t col, Direction direction, FieldType character, FieldType otherCharacter)
    {
        const auto [directionVertical, directionHorizontal] {mDirectionsMap.at(direction)};
        const auto flippedVertical = -directionVertical;
        const auto flippedHorizontal = -directionHorizontal;

        if (GetMatch(row + directionVertical, col + directionHorizontal, direction, character, character))
        {
            if (GetMatch(row + flippedVertical, col + flippedHorizontal, direction, otherCharacter, otherCharacter))
            {
                return true;
            }
        }
        else if (GetMatch(row + directionVertical, col + directionHorizontal, direction, otherCharacter, otherCharacter))
        {
            if (GetMatch(row + flippedVertical, col + flippedHorizontal, direction, character, character))
            {
                return true;
            }
        }

        return false;
    }

    int32_t GetNumberOfDiagonalMatches(int32_t row, int32_t col, FieldType expectedCharacter = 'A')
    {
        if (row >= mData.size() || col >= mData[row].size())
        {
            return 0;
        }

        FieldType currentCharacter{ mData[row][col] };
        if (expectedCharacter != currentCharacter)
        {
            return 0;
        }

        int32_t result{};
        if (GetDiagonalMatch(row, col, Direction::leftUp, 'M', 'S'))
        {
            ++result;
        }
        if (GetDiagonalMatch(row, col, Direction::leftDown, 'M', 'S'))
        {
            ++result;
        }
        return result;
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day4, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            const auto rowSize{ rowInput.size() };
            auto& row{ mData.emplace_back() };
            row.reserve(rowSize);
            for (char character : rowInput)
            {
                row.emplace_back(character);
            }
        }
    }

    void PerformFirst() override
    {
        int32_t result{};
        for (const auto [rowIndex, row] : mData | std::ranges::views::enumerate)
        {
            for (const auto [colIndex, col] : row | std::ranges::views::enumerate)
            {
                result += GetNumberOfMatches(rowIndex, colIndex, sCharacterOrder.front(), sCharacterOrder.back());
            }
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        int32_t result{};
        for (const auto [rowIndex, row] : mData | std::ranges::views::enumerate)
        {
            for (const auto [colIndex, col] : row | std::ranges::views::enumerate)
            {
                if (2 == GetNumberOfDiagonalMatches(rowIndex, colIndex))
                {
                    ++result;
                }
            }
        }
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::vector<std::vector<FieldType>> mData;
};
