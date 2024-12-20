#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>

namespace day14::helper
{
    static constexpr int32_t sOffByOne{ 1 };
}

template<utility::InputVersion version = utility::InputVersion::release>
class Day14 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day14" };

private:
    using Number = int32_t;
    using Position = utility::Position<Number>;
    using Velocity = Position;

    struct TileAABB
    {
        Position mMinimum;
        Position mMaximum;
    };

    struct RobotData
    {
        Position mPosition;
        Velocity mVelocity;
    };

    struct ScratchData
    {
        std::vector<RobotData> mData;
    };

    void CorrectRobotPositionIfNeeded(Number& robotPos, Number minimumBound, Number maximumBound)
    {
        if (robotPos > maximumBound)
        {
            robotPos = (robotPos % (maximumBound + day14::helper::sOffByOne)) + minimumBound;
        }
        else if (robotPos < minimumBound)
        {
            Number difference{ minimumBound - robotPos };
            robotPos = (maximumBound + day14::helper::sOffByOne) - (difference % maximumBound);
        }
    }

    void UpdateRobotPosition(RobotData& robotData)
    {
        robotData.mPosition += robotData.mVelocity;
        CorrectRobotPositionIfNeeded(robotData.mPosition.mX, mTileBound.mMinimum.mX, mTileBound.mMaximum.mX);
        CorrectRobotPositionIfNeeded(robotData.mPosition.mY, mTileBound.mMinimum.mY, mTileBound.mMaximum.mY);
    }

    void UpdateRobotPositions(ScratchData& scratchData, Number timesToUpdate)
    {
        for (Number i = 0; i < timesToUpdate; i++)
        {
            for (auto& robotData : scratchData.mData)
            {
                UpdateRobotPosition(robotData);
            }
        }
    }

    inline bool IsInsideOfAABB(Position position, const TileAABB& aabb)
    {
        return position.mX >= aabb.mMinimum.mX && position.mX <= aabb.mMaximum.mX && position.mY >= aabb.mMinimum.mY && position.mY <= aabb.mMaximum.mY;
    }

    template<std::ranges::input_range T>
    auto GetRobotsFromAABB(T&& range, const TileAABB& aabb) // supposed to constrain type here.
    {
        return range | std::ranges::views::filter([=, this](const auto& value) {return IsInsideOfAABB(value.mPosition, aabb); });
    }

    std::vector<TileAABB> GetQuadrants(const TileAABB& tileAABB)
    {
        std::vector<TileAABB> quadrants;

        Number xDifference{ tileAABB.mMaximum.mX - tileAABB.mMinimum.mX + day14::helper::sOffByOne };
        Number yDifference{ tileAABB.mMaximum.mY - tileAABB.mMinimum.mY + day14::helper::sOffByOne };
        bool xIsEven{ xDifference % 2 == 0 };
        bool yIsEven{ yDifference % 2 == 0 };
        assert(!xIsEven);
        assert(!yIsEven);

        Number halfX{ xDifference / 2 };
        Number halfY{ yDifference / 2 };
        TileAABB topLeft{ Position{tileAABB.mMinimum},
            Position{.mY = tileAABB.mMinimum.mY + halfY - 1, .mX = tileAABB.mMinimum.mX + halfX - 1 } };
        TileAABB topRight{ Position{.mY = tileAABB.mMinimum.mY , .mX = tileAABB.mMinimum.mX + halfX + 1},
            Position{.mY = tileAABB.mMinimum.mY + halfY - 1, .mX = tileAABB.mMaximum.mX} };
        TileAABB bottomLeft{ Position{.mY = tileAABB.mMinimum.mY + halfY + 1, .mX = tileAABB.mMinimum.mX},
            Position{.mY = tileAABB.mMaximum.mY, .mX = tileAABB.mMinimum.mX + halfX - 1} };
        TileAABB bottomRight{ Position{.mY = tileAABB.mMinimum.mY + 1 + halfY , .mX = tileAABB.mMinimum.mX + halfX + 1},
            Position{tileAABB.mMaximum} };

        quadrants.push_back(std::move(topLeft));
        quadrants.push_back(std::move(topRight));
        quadrants.push_back(std::move(bottomLeft));
        quadrants.push_back(std::move(bottomRight));

        return quadrants;
    }

    void PrintRobots(ScratchData& scratchData, std::ostream& outStream)
    {
        for (Number row = mTileBound.mMinimum.mRow; row < mTileBound.mMaximum.mRow; row++)
        {
            for (Number col = mTileBound.mMinimum.mCol; col < mTileBound.mMaximum.mCol; col++)
            {
                if (std::ranges::find(scratchData.mData, Position{ .mRow = row, .mCol = col }, &RobotData::mPosition) != scratchData.mData.end())
                {
                    outStream << '#';
                }
                else
                {
                    outStream << ' ';
                }
            }
            outStream << '\n';
        }
    }

    bool HasRobotOnPoint(ScratchData& scratchData, Position position)
    {
        return std::ranges::find(scratchData.mData, position, &RobotData::mPosition) != scratchData.mData.end();
    }

    bool AllMiddlePointsOccupied(ScratchData& scratchData)
    {
        bool result{ true };
        Number middleCol{ mTileBound.mMaximum.mCol / 2 };
        for (auto [index, row] : scratchData.mData | std::ranges::views::enumerate)
        {
            if (!HasRobotOnPoint(scratchData, Position{ .mRow = static_cast<Number>(index), .mCol = middleCol }))
            {
                return false;
            }
        }

        return true;
    }

    Number GetLargestContiguousBlock(ScratchData& scratchData, std::vector<std::vector<bool>>& visitedFields)
    {
        Number result{ 0 };
        for (Number row = 0; row < mTileBound.mMaximum.mRow; row++)
        {
            for (Number col = 0; col < mTileBound.mMaximum.mCol; col++)
            {
                Number currentResult{};
                GetLargestContiguousBlock(scratchData, visitedFields, { row,col }, currentResult);
                result = std::max(currentResult, result);
            }
        }

        return result;
    }

    void GetLargestContiguousBlock(ScratchData& scratchData, std::vector<std::vector<bool>>& visitedFields, Position position, Number& currentResult)
    {
        if (!IsInsideOfAABB(position, mTileBound) || visitedFields[position.mRow][position.mCol])
        {
            return;
        }

        visitedFields[position.mRow][position.mCol] = true;
        if (!HasRobotOnPoint(scratchData, position))
        {
            return;
        }

        ++currentResult;
        for (auto& offset : utility::sFullDirectionsMap | std::ranges::views::values)
        {
            GetLargestContiguousBlock(scratchData, visitedFields, position + offset, currentResult);
        }
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day14, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            auto robotStats{ utility::GetNumbers<Number>(rowInput) };
            mRobotData.emplace_back(Position{ robotStats[1],robotStats[0] }, Velocity{ robotStats[3],robotStats[2] });
        }

        if (version == utility::InputVersion::test)
        {
            mTileBound.mMinimum = Position{ 0,0 };
            mTileBound.mMaximum = Position{ 7 - day14::helper::sOffByOne, 11 - day14::helper::sOffByOne };
        }
        else
        {
            mTileBound.mMinimum = Position{ 0,0 };
            mTileBound.mMaximum = Position{ 103 - day14::helper::sOffByOne, 101 - day14::helper::sOffByOne };
        }
    }

    void PerformFirst() override
    {
        ScratchData scratchData;
        scratchData.mData = mRobotData;
        UpdateRobotPositions(scratchData, 100);
        const auto quadrantBoundingBoxes{ GetQuadrants(mTileBound) };
        std::vector<Number> numberOfRobotsInQuadrant{};
        for (auto&& quadrantBoundingBox : quadrantBoundingBoxes)
        {
            auto robotsInQuadrantView{ GetRobotsFromAABB(scratchData.mData,quadrantBoundingBox) };
            numberOfRobotsInQuadrant.push_back(std::distance(robotsInQuadrantView.begin(), robotsInQuadrantView.end()));
        }

        Number result{ std::reduce(numberOfRobotsInQuadrant.begin(),numberOfRobotsInQuadrant.end(),Number{1}, [](auto sum, auto value)->Number {return value > 0 ? sum * value : sum; }) };
        utility::PrintDetails(version, utility::Part::first);
        utility::PrintResult(result);
    }

    void PerformSecond() override
    {
        utility::PrintDetails(version, utility::Part::second);
        ScratchData scratchData;
        scratchData.mData = mRobotData;
        Number numToAdvance{};
        Number numberOfAdvancements{};

        std::vector<std::vector<bool>> visitedFields(mTileBound.mMaximum.mRow + day14::helper::sOffByOne, std::vector<bool>(mTileBound.mMaximum.mCol + day14::helper::sOffByOne, false));
        while (std::cin >> numToAdvance)
        {
            if (numToAdvance == 0)
            {
                break;
            }

            for (int i = 0; i < numToAdvance; i++)
            {
                while (scratchData.mData.size() / 5 > GetLargestContiguousBlock(scratchData, visitedFields))
                {
                    numberOfAdvancements++;
                    UpdateRobotPositions(scratchData, 1);

                    // Clear visited fields 
                    for (auto& boolVec : visitedFields)
                    {
                        boolVec.clear();
                        boolVec.resize(mTileBound.mMaximum.mCol + day14::helper::sOffByOne, false);
                        // vector of bool is fcked up...
                    }
                }
                std::cout << "Advancement: " << numberOfAdvancements << '\n';
                PrintRobots(scratchData, std::cout);
            }
        }
    }

private:
    std::string mBuffer;
    std::vector<RobotData> mRobotData;
    TileAABB mTileBound;
};
