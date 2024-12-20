#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <span>
#include <vector>

template<utility::InputVersion version = utility::InputVersion::release>
class Day12 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day12" };

private:
    using Number = uint64_t;
    using PositionType = int32_t;
    using Position = utility::Position<PositionType>;
    using PlotType = char;
    using VisitedFieldVector = std::vector<std::vector<bool>>;

    struct Region
    {
        PlotType mPlotType;
        std::vector<Position> mPositions;
    };

    struct RegionStats
    {
        Number mArea;
        Number mPerimeter;
    };

    enum class EdgeType
    {
        vertical,
        horizontal,
    };

    struct Edge
    {
        Position mPosition;
        EdgeType mEdgeType;
        utility::Direction mDirection;
        bool mHandled;
    };

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day12, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            mPlotGrid.emplace_back();
            auto& row{ mPlotGrid.back() };
            row.reserve(rowInput.size());
            for (const auto character : rowInput)
            {
                if (character == '\0')
                {
                    continue;
                }

                row.emplace_back(character);
            }
        }

        assert(!mPlotGrid.empty());
        VisitedFieldVector visitedFields(mPlotGrid.size(), std::vector<bool>(mPlotGrid[0].size(), false));
        for (auto [rowIndex, row] : mPlotGrid | std::ranges::views::enumerate)
        {
            for (auto [colIndex, plotType] : row | std::ranges::views::enumerate)
            {
                if (visitedFields[rowIndex][colIndex])
                {
                    continue;
                }

                auto& regionIterator{ mRegions.emplace_back(plotType) };
                GatherFieldsOfRegion({ static_cast<int32_t>(rowIndex),static_cast<int32_t>(colIndex) }, regionIterator, visitedFields);
            }
        }
    }

    void GatherFieldsOfRegion(Position position, Region& region, VisitedFieldVector& visitedFields)
    {
        auto currentField{ utility::GetItemAt(mPlotGrid, position) };
        if (!currentField.has_value())
        {
            return;     // out of bounds
        }

        const auto currentFieldValue{ *currentField.value() };
        if (currentFieldValue != region.mPlotType)
        {
            return;
        }

        auto visitedFieldValue{ visitedFields[position.mRow][position.mCol] };
        if (visitedFieldValue)
        {
            return;
        }

        visitedFieldValue = true;
        region.mPositions.push_back(position);

        for (auto offset : utility::sBaseDirectionsMap | std::ranges::views::values)
        {
            auto offsetPosition = position + offset;
            GatherFieldsOfRegion(offsetPosition, region, visitedFields);
        }
    }

    Number GetNumberOfOpenEdges(const Region& region, Position position)
    {
        Number openEdges{ 0 };
        for (auto offset : utility::sBaseDirectionsMap | std::ranges::views::values)
        {
            auto offsetPosition = position + offset;
            if (std::ranges::find(region.mPositions, offsetPosition) == region.mPositions.end())
            {
                ++openEdges;
            }
        }

        return openEdges;
    }

    RegionStats CalculateRegionStats(const Region& region)
    {
        Number perimeter{};
        for (auto&& position : region.mPositions)
        {
            perimeter += GetNumberOfOpenEdges(region, position);
        }

        return { region.mPositions.size(), perimeter };
    }

    std::vector<Edge> GetEdges(const Region& region, Position position)
    {
        std::vector<Edge> edges;
        for (auto [direction, offset] : utility::sBaseDirectionsMap)
        {
            auto offsetPosition = position + offset;
            if (std::ranges::find(region.mPositions, offsetPosition) == region.mPositions.end())
            {
                if (direction == utility::Direction::left || direction == utility::Direction::right)
                {
                    edges.emplace_back(offsetPosition, EdgeType::vertical, direction);
                }
                else
                {
                    edges.emplace_back(offsetPosition, EdgeType::horizontal, direction);
                }
            }
        }

        return edges;
    }

    Number GetNumberOfSides(std::vector<Edge> edges, EdgeType edgeType)
    {
        PositionType Position::* inspectedAxis{ edgeType == EdgeType::horizontal ? &Position::mCol : &Position::mRow };
        PositionType Position::* otherAxis{ edgeType == EdgeType::horizontal ? &Position::mRow : &Position::mCol };
        std::ranges::sort(edges, [=](const auto& edgeA, const auto& edgeB)
            {
                return edgeA.mPosition.*inspectedAxis <= edgeB.mPosition.*inspectedAxis;
            }
        );

        Number result{};
        for (auto&& [index, edge] : edges | std::ranges::views::enumerate)
        {
            if (edge.mHandled)
            {
                continue;
            }
            ++result;
            edge.mHandled = true;

            auto& otherAxisValue{ edge.mPosition.*otherAxis };
            auto lastInspectedAxisValue{ edge.mPosition.*inspectedAxis };
            auto filterForSameOtherAxis{ std::ranges::views::filter([=](const auto& otherEdge) {return otherAxisValue == otherEdge.mPosition.*otherAxis && edge.mDirection == otherEdge.mDirection; }) };
            for (auto&& otherElement : std::ranges::subrange(edges.begin() + index, edges.end()) | filterForSameOtherAxis)
            {
                if (otherElement.mPosition == edge.mPosition)
                {
                    continue;
                }

                if (otherElement.mPosition.*inspectedAxis - 1 > lastInspectedAxisValue)
                {
                    break;
                }
                lastInspectedAxisValue = otherElement.mPosition.*inspectedAxis;

                otherElement.mHandled = true;
            }
        }

        return result;
    }

    Number CalculatePlotSides(const Region& region)
    {
        std::vector<Edge> edges;
        for (auto&& position : region.mPositions)
        {
            auto posEdges{ GetEdges(region,position) };
            edges.insert(edges.end(), std::make_move_iterator(posEdges.begin()), std::make_move_iterator(posEdges.end()));
        }

        std::vector<Edge> verticalEdges;
        for (auto&& edge : edges)
        {
            if (edge.mEdgeType == EdgeType::vertical)
            {
                verticalEdges.emplace_back(edge);
            }
        }

        // Wasn't feeling it today so I didn't look into why these didn't work.
        //auto verticalEdges{ edges | std::ranges::views::filter([](auto& edge) {return edge.mEdgeType == EdgeType::vertical; }) | std::ranges::to };
        //edges.erase(std::ranges::remove_if(edges, [](auto&& edge) {return edge.mEdgeType == EdgeType::vertical; }), edges.end());

        edges.erase(std::remove_if(edges.begin(), edges.end(), [](auto&& edge) {return edge.mEdgeType == EdgeType::vertical; }), edges.end());
        Number numberOfVerticalEdges{ GetNumberOfSides(std::move(verticalEdges), EdgeType::vertical) };
        Number numberOfHorizontalEdges{ GetNumberOfSides(std::move(edges), EdgeType::horizontal) };
        return numberOfHorizontalEdges + numberOfHorizontalEdges;
    }

    void PerformFirst() override
    {
        Number result{};
        for (auto&& region : mRegions)
        {
            auto [area, perimeter] {CalculateRegionStats(region)};
            result += area * perimeter;
        }

        utility::PrintDetails(version, utility::Part::first);
        utility::PrintResult(result);
    }

    void PerformSecond() override
    {
        Number result{};
        for (auto&& region : mRegions)
        {
            const auto sides{ CalculatePlotSides(region) };
            result += region.mPositions.size() * sides;
            std::cout << "Region: " << region.mPlotType << " Area: " << region.mPositions.size() 
                << " Sides: " << sides << " = " << region.mPositions.size() * sides << '\n';
        }
        utility::PrintDetails(version, utility::Part::second);
        utility::PrintResult(result);
    }

private:
    std::string mBuffer;
    std::vector<std::vector<PlotType>> mPlotGrid;
    std::vector<Region> mRegions;
};
