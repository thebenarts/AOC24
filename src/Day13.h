#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <span>
#include <vector>

template<utility::InputVersion version = utility::InputVersion::release>
class Day13 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day13" };

private:
    using Number = uint64_t;
    using Position = utility::Position<Number>;
    using ClawMove = Position;

    struct SlotMachine
    {
        Position mButtonA;
        Position mButtonB;
        Position mPrize;
    };

    Number GetWinningClawMove(const SlotMachine& slotMachine, Number offset)
    {
        // Cramer's rule.
        // https://byjus.com/maths/cramers-rule/
        // https://byjus.com/cramers-rule-calculator/
        Number sPriceA{ 3 };
        Number sPriceB{ 1 };
        double aX{ static_cast<double>(slotMachine.mButtonA.mRow) };
        double aY{ static_cast<double>(slotMachine.mButtonA.mCol) };
        double bX{ static_cast<double>(slotMachine.mButtonB.mRow) };
        double bY{ static_cast<double>(slotMachine.mButtonB.mCol) };
        double pX{ static_cast<double>(slotMachine.mPrize.mRow) };
        double pY{ static_cast<double>(slotMachine.mPrize.mCol) };
        pX += offset;
        pY += offset;

        auto numA{ (pX * bY - pY * bX) / (aX * bY - aY * bX) };
        auto numB{ (aX * pY - aY * pX) / (aX * bY - aY * bX) };
        if (std::modf(numA, &numA) == 0.0f && std::modf(numB, &numB) == 0.0f)
        {
            Number resultA{ static_cast<Number>(numA) };
            Number resultB{ static_cast<Number>(numB) };
            return resultA * sPriceA + resultB * sPriceB;
        }

        return 0;
    }

    // Numbers are all positive.
    // only additions occur
    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day13, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            auto numbers{ utility::GetNumbers<Number>(rowInput) };
            assert(numbers.size() == 6);
            mSlotMachines.emplace_back(Position{ numbers[0], numbers[1] }, Position{ numbers[2], numbers[3] }, Position{ numbers[4], numbers[5] });
        }
    }

    void PerformFirst() override
    {
        Number result{};
        for (const auto& slotMachine : mSlotMachines)
        {
            auto winningMovePrice{ GetWinningClawMove(slotMachine, 0) };
            result += winningMovePrice;
        }
        utility::PrintDetails(version, utility::Part::first);
        utility::PrintResult(result);
    }

    void PerformSecond() override
    {
        Number result{};
        for (const auto& slotMachine : mSlotMachines)
        {
            auto winningMovePrice{ GetWinningClawMove(slotMachine, 1'000'000'000'000'0) };
            result += winningMovePrice;
        }
        utility::PrintDetails(version, utility::Part::second);
        utility::PrintResult(result);
    }

private:
    std::string mBuffer;
    std::vector<SlotMachine> mSlotMachines;
};
