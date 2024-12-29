#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>


namespace day15::helper
{
    using FieldType = char;

    constexpr FieldType sEmptyField{ '.' };
    constexpr FieldType sRobotField{ '@' };
    constexpr FieldType sBoxField{ 'O' };
    constexpr FieldType sWallField{ '#' };
}

template<utility::InputVersion version = utility::InputVersion::release>
class Day15 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day15" };

private:
    using Number = int32_t;
    using Position = utility::Position<Number>;
    using Velocity = Position;
    using Direction = utility::Direction;
    using Wall = Position;

    struct Box
    {
        Position mLeftSide;
        Position mRightSide;
        bool mUpdate;
    };

    struct ScratchData
    {
        std::vector<Box> mBoxes;
        std::vector<Wall> mWalls;
        Position mRobotPosition;
    };

    [[nodiscard]] Box* FindBoxOnPosition(ScratchData& scratchData, Position position)
    {
        auto boxIterator{ std::ranges::find_if(scratchData.mBoxes, [position](auto&& box) { return box.mLeftSide == position || box.mRightSide == position; }) };
        if (boxIterator == scratchData.mBoxes.end())
        {
            return nullptr;
        }

        return &*boxIterator;
    }

    [[nodiscard]] bool HasWallOnPosition(ScratchData& scratchData, Position position)
    {
        return std::ranges::any_of(scratchData.mWalls, [=](auto&& wall) {return wall == position; });
    }

    [[nodiscard]] bool IsValidInstruction(ScratchData& scratchData, Position positionToMoveFrom, Direction direction)
    {
        Position positionToMoveTo{ positionToMoveFrom + utility::GetDirectionData(direction) };
        Box* boxOnCurrentPosition{ FindBoxOnPosition(scratchData, positionToMoveFrom) };
        Box* boxOnNextPosition{ FindBoxOnPosition(scratchData, positionToMoveTo) };
        const bool wallOnNextPosition{ HasWallOnPosition(scratchData, positionToMoveTo) };
        // early out in case it's moving into a wall
        if (wallOnNextPosition)
        {
            return false;
        }
        // early out in case it's moving to an empty field or is a duplicated move
        if ((!boxOnNextPosition || boxOnNextPosition == boxOnCurrentPosition))
        {
            if (boxOnCurrentPosition)
            {
                boxOnCurrentPosition->mUpdate = true;
            }
            return true;
        }

        bool result{ IsValidInstruction(scratchData, boxOnNextPosition->mLeftSide, direction) && IsValidInstruction(scratchData, boxOnNextPosition->mRightSide, direction) };
        if (result && boxOnCurrentPosition)
        {
            boxOnCurrentPosition->mUpdate = true;
        }

        return result;
    }

    void ApplyInstruction(ScratchData& scratchData, Direction instruction)
    {
        if (IsValidInstruction(scratchData, scratchData.mRobotPosition, instruction))
        {
            utility::DirectionData directionOffset{ utility::GetDirectionData(instruction) };
            scratchData.mRobotPosition += directionOffset;
            for (auto& box : scratchData.mBoxes | std::ranges::views::filter([](auto&& box) {return box.mUpdate == true; }))
            {
                box.mLeftSide += directionOffset;
                box.mRightSide += directionOffset;
                box.mUpdate = false;
            }
        }

        for (auto& box : scratchData.mBoxes)
        {
            box.mUpdate = false;
        }
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day15, version> inputReader;
        mBuffer = inputReader.Read();
        std::vector<std::string_view> splitViews;
        for (auto splitView : mBuffer | std::ranges::views::split("\n\n"sv) | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            splitViews.push_back(splitView);
        }

        // process map
        for (auto [rowIndex, rowInput] : splitViews[0] | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }) | std::ranges::views::enumerate)
        {
            auto& row{ mData.emplace_back() };
            row.reserve(rowInput.size());
            for (const auto [colIndex, field] : rowInput | std::ranges::views::enumerate)
            {
                if (field == day15::helper::sRobotField)
                {
                    mRobotOrigin = Position{ .mRow = Number(rowIndex), .mCol = Number(colIndex) };
                }

                row.push_back(field);
            }
        }

        //process instructions
        for (const auto rowInput : splitViews[1] | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            mInstructions.reserve(mInstructions.size() + rowInput.size());
            for (const auto field : rowInput)
            {
                switch (field)
                {
                case '^':
                {
                    mInstructions.push_back(Direction::up);
                }
                break;
                case 'v':
                {
                    mInstructions.push_back(Direction::down);
                }
                break;
                case '<':
                {
                    mInstructions.push_back(Direction::left);
                }
                break;
                case '>':
                {
                    mInstructions.push_back(Direction::right);
                }
                break;
                case '\0':
                {
                    return;
                }
                break;
                default:
                {
                    assert(false);
                }
                break;
                }
            }
        }
    }

    void PerformFirst() override
    {
        ScratchData scratchData;
        for (auto [rowIndex, row] : mData | std::ranges::views::enumerate)
        {
            for (auto [colIndex, field] : row | std::ranges::views::enumerate)
            {
                if (field == day15::helper::sBoxField)
                {
                    scratchData.mBoxes.emplace_back(Position{ .mRow = static_cast<Number>(rowIndex),.mCol = static_cast<Number>(colIndex) }, Position{ .mRow = static_cast<Number>(rowIndex),.mCol = static_cast<Number>(colIndex) });
                }
                else if (field == day15::helper::sWallField)
                {
                    scratchData.mWalls.push_back(Position{ .mRow = static_cast<Number>(rowIndex),.mCol = static_cast<Number>(colIndex) });
                }
            }
        }
        scratchData.mRobotPosition = { .mRow = mRobotOrigin.mRow , .mCol = mRobotOrigin.mCol };
        for (auto instruction : mInstructions)
        {
            ApplyInstruction(scratchData, instruction);
        }

        Number result{};
        for (const auto& box : scratchData.mBoxes)
        {
            result += box.mLeftSide.mRow * 100 + box.mLeftSide.mCol;
        }

        utility::PrintDetails(version, utility::Part::first);
        utility::PrintResult(result);
    }

    void PerformSecond() override
    {
        ScratchData scratchData;
        for (auto [rowIndex, row] : mData | std::ranges::views::enumerate)
        {
            for (auto [colIndex, field] : row | std::ranges::views::enumerate)
            {
                Number newColIndex{ static_cast<Number>(colIndex) * 2 };
                if (field == day15::helper::sBoxField)
                {
                    scratchData.mBoxes.emplace_back(Position{ .mRow = static_cast<Number>(rowIndex),.mCol = newColIndex }, Position{ .mRow = static_cast<Number>(rowIndex),.mCol = newColIndex + 1 });
                }
                else if (field == day15::helper::sWallField)
                {
                    scratchData.mWalls.push_back(Position{ .mRow = static_cast<Number>(rowIndex),.mCol = newColIndex });
                    scratchData.mWalls.push_back(Position{ .mRow = static_cast<Number>(rowIndex),.mCol = newColIndex + 1 });
                }
            }
        }
        scratchData.mRobotPosition = { .mRow = mRobotOrigin.mRow , .mCol = mRobotOrigin.mCol * 2 };

        for (auto instruction : mInstructions)
        {
            ApplyInstruction(scratchData, instruction);
        }

        Number result{};
        for (const auto& box : scratchData.mBoxes)
        {
            result += box.mLeftSide.mRow * 100 + box.mLeftSide.mCol;
        }

        utility::PrintDetails(version, utility::Part::second);
        utility::PrintResult(result);
    }

private:
    std::string mBuffer;
    Position mRobotOrigin;
    std::vector<std::vector<day15::helper::FieldType>> mData;
    std::vector<Direction> mInstructions;
};
