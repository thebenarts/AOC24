#pragma once
#include "Utility.h"
#include "Day.h"

#include <iostream>
#include <ranges>
#include <expected>
#include <optional>

template<utility::InputVersion version = utility::InputVersion::release>
class Day6 : public DayBase<version>
{
    enum class Direction
    {
        up,
        right,
        down,
        left,
    };

    enum class FieldType
    {
        empty,
        guardStart,
        wall,
    };

    struct Position
    {
        using PositionType = int32_t;
        union
        {
            PositionType mRow{};  // vertical
            PositionType mY;    // vertical
        };
        union
        {
            PositionType mCol{};  // horizontal
            PositionType mX;    // horizontal
        };

        auto operator<=>(const Position& other) const
        {
            if (auto cmp{ mRow <=> other.mRow }; cmp != 0)
            {
                return cmp;
            }
            return mCol <=> other.mCol;
        }
        bool operator==(const Position& other) const
        {
            return mRow == other.mRow && mCol == other.mCol;
        }
        // Should be defining operators for Field<=>, but I am too lazy to move them outside of the scope here.
    };

    struct Field
    {
        using Number = int32_t;

        auto operator<=>(const Field& other) const
        {
            if (auto cmp{ mPosition <=> other.mPosition }; cmp != 0)
            {
                return cmp;
            }
            return mType <=> other.mType;
        }
        bool operator==(const Field& other) const
        {
            return mPosition == other.mPosition && mType == other.mType;
        }

        auto operator<=>(const Position& other) const
        {
            if (auto cmp{ mPosition.mRow <=> other.mRow }; cmp != 0)
            {
                return cmp;
            }
            return mPosition.mCol <=> other.mCol;
        }
        bool operator==(const Position& other) const
        {
            return mPosition.mRow == other.mRow && mPosition.mCol == other.mCol;
        }

        size_t GetNumberOfTimesVisited() const
        {
            return mDirectionsApproachedFrom.size();
        };

        bool IsVisitedMultipleTimesFromSameDirectionConsecutively() const
        {
            if (GetNumberOfTimesVisited() <= 1)
            {
                return false;
            }

            assert(mDirectionsApproachedFrom.size() > 1);

            const auto& lastDirectionIterator{ mDirectionsApproachedFrom.rbegin() };
            const auto& directionBeforeLastDirectionIterator{ std::next(lastDirectionIterator,1) };
            return *lastDirectionIterator == *directionBeforeLastDirectionIterator;
        }

        Position mPosition{};
        std::vector<Direction> mDirectionsApproachedFrom;
        FieldType mType{ FieldType::empty };
    };

    enum class ErrorType
    {
        outOfBounds,
        wallVisitedMultipleTimes,
    };

    struct ScratchData
    {
        Field& MarkFieldAsVisited(Position position, FieldType type, Direction direction)
        {
            const auto resultIterator{ std::ranges::find_if(mVisitedFields,[position](const auto& field) {return field == position; }) };
            if (resultIterator == mVisitedFields.end())
            {
                mVisitedFields.emplace_back(position, std::vector<Direction>{direction}, type);
                return mVisitedFields.back();
            }

            resultIterator->mDirectionsApproachedFrom.emplace_back(direction);
            assert(resultIterator->mType == type);
            return *resultIterator;
        }

        Field& MarkWallAsVisited(Position position, FieldType type, Direction direction)
        {
            assert(type == FieldType::wall);

            const auto resultIterator{ std::ranges::find_if(mVisitedWalls,[position](const auto& field) {return field == position; }) };
            if (resultIterator == mVisitedWalls.end())
            {
                mVisitedWalls.emplace_back(position, std::vector<Direction>{direction}, type);
                return mVisitedFields.back();
            }

            resultIterator->mDirectionsApproachedFrom.emplace_back(direction);
            assert(resultIterator->mType == type);
            return *resultIterator;
        }

        std::optional<Field> TryGetFieldOverride(Position position)
        {
            if (const auto resultIterator{ std::ranges::find_if(mFieldOverrides,[position](const auto& field) {return field == position; }) }; resultIterator != mFieldOverrides.end())
            {
                return *resultIterator;
            }

            return {};
        }

        std::vector<Field> mVisitedFields;
        std::vector<Field> mVisitedWalls;

        std::vector<Field> mFieldOverrides;
    };

    static constexpr std::array<Direction, 4> sDirectionOrder{ Direction::up, Direction::right, Direction::down, Direction::left };
    static constexpr std::array<Position, 4> sDirectionValuesInOrder{
        Position{.mRow = -1, .mCol = 0},
        Position{.mRow = 0, .mCol = 1},
        Position{.mRow = 1, .mCol = 0},
        Position{.mRow = 0, .mCol = -1} };

    template<utility::IsMultiDimensionalRandomAccessRange T>
    struct TwoDimensionalVectorType
    {
        using type = std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>>>;
        using pointer_type = type*;
    };

    template<utility::IsMultiDimensionalRandomAccessRange T>
    std::expected<typename TwoDimensionalVectorType<T>::pointer_type, ErrorType> GetItemAt(T&& vector, Position position)
    {
        if (position.mRow < 0 || position.mRow >= vector.size() || position.mCol < 0 || position.mCol >= vector[position.mRow].size())
        {
            return std::unexpected{ ErrorType::outOfBounds };
        }

        return &vector[position.mRow][position.mCol];
    }

public:
    static constexpr std::string_view sDay{ "day6" };

private:
    static constexpr size_t GetDirectionIndex(Direction direction)
    {
        const auto directionIterator{ std::ranges::find(sDirectionOrder,direction) };
        if (directionIterator == sDirectionOrder.end())
        {
            assert(false);
            return 0;
        }

        return std::distance(sDirectionOrder.begin(), directionIterator);
    }

    static constexpr size_t GetNextDirectionIndex(Direction direction)
    {
        return (GetDirectionIndex(direction) + 1) % sDirectionOrder.size();
    }

    static constexpr Direction GetNextDirection(Direction direction)
    {
        return sDirectionOrder[GetNextDirectionIndex(direction)];
    }

    static_assert(GetNextDirection(Direction::up) == Direction::right);
    static_assert(GetNextDirection(Direction::right) == Direction::down);
    static_assert(GetNextDirection(Direction::down) == Direction::left);
    static_assert(GetNextDirection(Direction::left) == Direction::up);

    static constexpr Position GetDirectionValue(Direction direction)
    {
        return sDirectionValuesInOrder[GetDirectionIndex(direction)];
    }

    static constexpr Position GetNextDirectionValue(Direction direction)
    {
        return sDirectionValuesInOrder[GetNextDirectionIndex(direction)];
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day6, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            mData.emplace_back(rowInput.size(), FieldType::empty);
            auto& row{ mData.back() };
            for (const auto [index, character] : rowInput | std::ranges::views::enumerate)
            {
                if (character == '\0')
                {
                    break;
                }

                if (character == '#')
                {
                    row[index] = FieldType::wall;
                    continue;
                }

                if (character == '^')
                {
                    row[index] = FieldType::guardStart;
                    mGuardOrigin.mRow = mData.size() - 1;
                    mGuardOrigin.mCol = index;
                }
            }
        }
    };

    // If a Field with FieldType is found, it returns the position of the that field.
    // i.e. when searching for walls it will return the position of the wall not where the guard would turn
    std::expected<Position, ErrorType> FindNextFieldTypeInDirection(Position position, Direction direction, ScratchData* scratchData, FieldType typeToSearchFor = FieldType::wall)
    {
        assert(scratchData);

        const auto itemResult{ GetItemAt(mData, position) };
        if (!itemResult.has_value())
        {
            return std::unexpected{ itemResult.error() };
        }

        assert(itemResult.value());
        auto itemResultType{ *itemResult.value() };
        if (auto optionalFieldOverride{ scratchData->TryGetFieldOverride(position) })
        {
            itemResultType = optionalFieldOverride->mType;
        }

        if (typeToSearchFor == itemResultType)
        {
            auto& wallField{ scratchData->MarkWallAsVisited(position, itemResultType, direction) };
            if (wallField.IsVisitedMultipleTimesFromSameDirectionConsecutively())
            {
                return std::unexpected{ ErrorType::wallVisitedMultipleTimes };
            }
            return position;
        }

        // Only mark tiles that are visited by the guard. Walls aren't actually visited.
        scratchData->MarkFieldAsVisited(position, itemResultType, direction);

        {
            const auto directionValue{ GetDirectionValue(direction) };
            position.mRow += directionValue.mRow;
            position.mCol += directionValue.mCol;
        }

        return FindNextFieldTypeInDirection(position, direction, scratchData, typeToSearchFor);
    }

    void PerformFirst() override
    {
        int32_t result{};
        Position guardPosition{ mGuardOrigin };
        Direction guardDirection{ Direction::up };
        ScratchData scratchData;
        while (true)
        {
            const auto searchResult{ FindNextFieldTypeInDirection(guardPosition, guardDirection, &scratchData) };
            if (!searchResult.has_value())
            {
                if (searchResult.error() == ErrorType::outOfBounds)
                {
                    break;
                }
                if (searchResult.error() == ErrorType::wallVisitedMultipleTimes)
                {
                    assert(false);
                }
            }

            auto& wallPosition{ searchResult.value() };

            auto directionValue{ GetDirectionValue(guardDirection) };
            guardPosition = wallPosition;
            guardPosition.mRow -= directionValue.mRow;
            guardPosition.mCol -= directionValue.mCol;
            guardDirection = GetNextDirection(guardDirection);
        }
        result = scratchData.mVisitedFields.size();

        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    };

    void PerformSecond() override
    {
        std::atomic_int32_t result{};
        std::vector<std::thread> workers;

        for (int32_t rowIndex = 0; rowIndex < mData.size(); rowIndex++)
        {
            const auto task = [this, rowIndex, &result]
                {
                    int32_t taskResult{};
                    for (const auto [columnIndex, fieldType] : mData[rowIndex] | std::ranges::views::enumerate
                        | std::ranges::views::filter([](const auto& indexValuePair) { return std::get<1>(indexValuePair) == FieldType::empty; }))
                    {
                        Position guardPosition{ mGuardOrigin };
                        Direction guardDirection{ Direction::up };
                        ScratchData scratchData;
                        scratchData.mFieldOverrides.emplace_back(Position{ .mRow = rowIndex, .mCol = static_cast<Position::PositionType>(columnIndex) }, std::vector<Direction>{}, FieldType::wall);

                        while (true)
                        {
                            const auto searchResult{ FindNextFieldTypeInDirection(guardPosition, guardDirection, &scratchData) };
                            if (!searchResult.has_value())
                            {
                                if (searchResult.error() == ErrorType::outOfBounds)
                                {
                                    break;
                                }
                                if (searchResult.error() == ErrorType::wallVisitedMultipleTimes)
                                {
                                    result++;
                                    break;
                                }
                            }

                            auto& wallPosition{ searchResult.value() };

                            auto directionValue{ GetDirectionValue(guardDirection) };
                            guardPosition = wallPosition;
                            guardPosition.mRow -= directionValue.mRow;
                            guardPosition.mCol -= directionValue.mCol;
                            guardDirection = GetNextDirection(guardDirection);
                        }
                    }
                    result.fetch_add(taskResult, std::memory_order_relaxed);
                };

            workers.emplace_back(task);
        }

        for (auto& worker : workers)
        {
            worker.join();
        }


        utility::PrintDetails(version, utility::Part::second);
        std::cout << result.load(std::memory_order_acquire) << '\n';
    };

private:
    std::string mBuffer;
    std::vector<std::vector<FieldType>> mData;
    Position mGuardOrigin{};
};
