#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>
#include <mutex>

// Should have solved most issues iteratively instead of recursively, but I was getting tired.

template<utility::InputVersion version = utility::InputVersion::release>
class Day16 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day16" };

private:
    using Number = int32_t;
    using Position = utility::Position<Number>;
    using Velocity = Position;
    using Direction = utility::Direction;

    static constexpr Number sMaximumDistance{ std::numeric_limits<Number>::max() };

    enum FieldType
    {
        empty,
        wall,
        start,
        end,
    };

    struct Node
    {
        FieldType mType;
        Number mDistance;
        std::optional<Direction> mDirection;
    };

    struct ScratchData
    {
        std::vector<std::vector<Node>> mData;
        std::vector<Position> mNodePositionsPartOfShortestPaths;
        Position mStartPosition;
        Position mEndPosition;
    };

    [[nodiscard]] ScratchData CreateScratchData()
    {
        ScratchData scratchData;
        scratchData.mStartPosition = mStartPosition;
        scratchData.mEndPosition = mEndPosition;

        scratchData.mData.reserve(mData.size());
        for (auto& rowInput : mData)
        {
            auto& row{ scratchData.mData.emplace_back() };
            row.reserve(rowInput.size());
            for (auto type : rowInput)
            {
                if (type != FieldType::start)
                {
                    row.emplace_back(type, sMaximumDistance);
                }
                else
                {
                    row.emplace_back(type, Number{ 0 });
                }
            }
        }

        return scratchData;
    }

    // Should be taking it as const, but GetItemAt isn't written properly.
    [[nodiscard]] Number GetEndDistanceFromStart(/*const */ScratchData& scratchData)
    {
        const auto endItemResult{ utility::GetItemAt(scratchData.mData, scratchData.mEndPosition) };
        if (!endItemResult.has_value())
        {
            assert(false);
            return {};
        }
        assert(endItemResult.value());
        const auto& endNode{ *endItemResult.value() };

        return endNode.mDistance;
    }

    [[nodiscard]] Number GetShortestPathCost(ScratchData& scratchData, Direction direction)
    {
        ExploreOptions<true>(scratchData, scratchData.mStartPosition, 0, direction);

        return GetEndDistanceFromStart(scratchData);
    }

    template<bool start = false>
    void ExploreOptions(ScratchData& scratchData, Position position, Number value, Direction direction)
    {
        auto itemAtResult{ utility::GetItemAt(scratchData.mData,position) };
        if (!itemAtResult.has_value())
        {
            assert(false);
            return;
        }
        assert(itemAtResult.value());

        auto& node{ *itemAtResult.value() };
        if (node.mType == FieldType::wall)
        {
            return;
        }
        // safety net so we don't recurse forever, as we are using distance as a visited marker 
        if (!start && node.mDistance <= value)
        {
            return;
        }

        node.mDistance = value;
        node.mDirection = direction;
        for (int i = 0; i < 4; i++)
        {
            ExploreOptions<false>(scratchData, position + utility::GetDirectionValue<Number>(direction), value + (1000 * (i % 2) + 1), direction);
            direction = utility::GetNextDirection(direction);
        }
    }

    // Is part of optimal path if distance from start to position + distance from position to end == distance from start to end.
    bool IsPartOfOptimalPath(/*const*/ ScratchData& scratchData, Position position)
    {
        auto itemAtResult{ utility::GetItemAt(scratchData.mData,position) };
        if (!itemAtResult.has_value())
        {
            assert(false);
            return false;
        }
        assert(itemAtResult.value());
        const auto& node{ *itemAtResult.value() };

        if (node.mDistance == sMaximumDistance || !node.mDirection)
        {
            return false;
        }

        ScratchData newScratchData{ CreateScratchData() };
        newScratchData.mStartPosition = position;
        Number distanceFromPointToEnd{ GetShortestPathCost(newScratchData, *node.mDirection) };
        Number distanceFromOriginalStartToEnd{ GetEndDistanceFromStart(scratchData) };

        return distanceFromOriginalStartToEnd - node.mDistance == distanceFromPointToEnd;
    }

    void CollectOptimalNodes(ScratchData& scratchData)
    {
        std::vector<std::thread> workers;
        std::mutex mutex;
        for (auto&& [rowIndex, row] : scratchData.mData | std::ranges::views::enumerate)
        {
            workers.reserve(row.size());
            for (auto&& [colIndex, node] : row | std::ranges::views::enumerate)
            {
                if (node.mDistance == sMaximumDistance)
                {
                    continue;
                }

                Position nodePosition{ .mRow = static_cast<Number>(rowIndex), .mCol = static_cast<Number>(colIndex) };
                workers.emplace_back([this, nodePosition, &scratchData, &mutex]() {
                    if (IsPartOfOptimalPath(scratchData, nodePosition))
                    {
                        std::unique_lock<std::mutex> lock{ mutex };
                        scratchData.mNodePositionsPartOfShortestPaths.push_back(nodePosition);
                        std::cout << "Found item at row: " << nodePosition.mRow << " col: " << nodePosition.mCol << '\n';
                    }
                    });
            }
            std::ranges::for_each(workers, &std::thread::join);
            workers.clear();
        }

    }

    void PrintPaths(ScratchData& scratchData)
    {
        std::cout << '\n';
        for (auto [rowIndex, row] : scratchData.mData | std::ranges::views::enumerate)
        {
            for (auto [colIndex, node] : row | std::ranges::views::enumerate)
            {
                Position currentPosition{ .mRow = static_cast<Number>(rowIndex), .mCol = static_cast<Number>(colIndex) };
                if (std::ranges::any_of(scratchData.mNodePositionsPartOfShortestPaths, [currentPosition](auto&& pos) {return currentPosition == pos; }))
                {
                    std::cout << 'O';
                }
                else
                {
                    switch (node.mType)
                    {
                    case FieldType::empty:
                    {
                        std::cout << '.';
                    }
                    break;
                    case FieldType::wall:
                    {
                        std::cout << '#';
                    }
                    break;
                    case FieldType::start:
                    {
                        std::cout << 'S';
                    }
                    break;
                    case FieldType::end:
                    {
                        std::cout << 'E';
                    }
                    break;
                    }
                }
            }
            std::cout << '\n';
        }
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day16, version> inputReader;
        mBuffer = inputReader.Read();
        std::vector<std::string_view> splitViews;
        for (auto rowInput : mBuffer | std::ranges::views::split("\n"sv) | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            auto& row{ mData.emplace_back() };
            row.reserve(rowInput.size());
            for (auto [colIndex, character] : rowInput | std::ranges::views::enumerate)
            {
                switch (character)
                {
                case '.':
                {
                    row.emplace_back(FieldType::empty);
                }
                break;
                case '#':
                {
                    row.emplace_back(FieldType::wall);
                }
                break;
                case 'S':
                {
                    row.emplace_back(FieldType::start);
                    mStartPosition = Position{ .mRow = static_cast<Number>(mData.size()) - 1, .mCol = static_cast<Number>(colIndex) };
                }
                break;
                case 'E':
                {
                    row.emplace_back(FieldType::end);
                    mEndPosition = Position{ .mRow = static_cast<Number>(mData.size()) - 1, .mCol = static_cast<Number>(colIndex) };
                }
                break;
                case '\0':
                {
                    return;
                }break;
                }
            }
        }
    }

    void PerformFirst() override
    {
        utility::PrintDetails(version, utility::Part::first);
        auto scratchData{ CreateScratchData() };
        Number result{ GetShortestPathCost(scratchData, Direction::right) };
        utility::PrintResult(result);
    }

    void PerformSecond() override
    {
        utility::PrintDetails(version, utility::Part::second);
        auto scratchData{ CreateScratchData() };
        std::ignore = GetShortestPathCost(scratchData, Direction::right);
        CollectOptimalNodes(scratchData);
        Number result{ static_cast<Number>(scratchData.mNodePositionsPartOfShortestPaths.size()) };
        utility::PrintResult(result);
        PrintPaths(scratchData);
    }

private:
    std::string mBuffer;
    std::vector<std::vector<FieldType>> mData;
    Position mStartPosition;
    Position mEndPosition;
};
