#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <unordered_set>
#include <unordered_map>

template<utility::InputVersion version = utility::InputVersion::release>
class Day7 : public DayBase<version>
{
    using Number = uint64_t;

public:
    static constexpr std::string_view sDay{ "day7" };

private:
    using IntegralFunctionOperator = std::function<Number(Number, Number)>;
    using IntegralFunctionOperatorsResult = std::vector<std::vector<IntegralFunctionOperator>>;
    struct ScratchData
    {
        std::unordered_map<Number, IntegralFunctionOperatorsResult> mResults;
        std::mutex mResultMapLock;
    };

    template<utility::Integral T, utility::IsVariadicIntegralOperatorFunction... U>
    std::vector<std::vector<std::common_type_t<U...>>> GetAllOperationsForNumberOfOperands(T&& numberOfTimesOperatorsAreNeeded, U&&... operatorFunctions)
    {
        assert(numberOfTimesOperatorsAreNeeded >= 1);
        static constexpr std::size_t numberOfOperators{ sizeof... (U) };
        assert(numberOfOperators >= 1);

        const auto vectorOfOperators{ CreateVectorFromOperators(operatorFunctions...) };
        std::vector<std::remove_cvref_t<decltype(vectorOfOperators)>> result;
        std::remove_cvref_t<decltype(vectorOfOperators)> current;
        GeneratePermutations(vectorOfOperators, numberOfTimesOperatorsAreNeeded, result, current);
        return result;
    }

    template<utility::IsVariadicIntegralOperatorFunction... U>
    std::vector<std::common_type_t<U...>> CreateVectorFromOperators(U&&... operatorFunctions)
    {
        static constexpr std::size_t numberOfOperators{ sizeof... (U) };
        assert(numberOfOperators >= 1);

        std::vector<std::common_type_t<U...>> result{ operatorFunctions... };
        return result;
    }

    template<utility::IsIntegralOperatorFunctionRange FunctionRange, utility::Integral Integral, utility::IsMultiDimensionalOperatorFunctionRange ResultRange, utility::IsIntegralOperatorFunctionRange AccumulatingRange>
    void GeneratePermutations(FunctionRange&& operatorFunctions, Integral numberOfOperands, ResultRange&& result, AccumulatingRange&& current)
    {
        if (static_cast<int32_t>(current.size()) == numberOfOperands)
        {
            result.push_back(current);
            return;
        }
        for (const auto& element : operatorFunctions)
        {
            current.push_back(element);
            GeneratePermutations(operatorFunctions, numberOfOperands, result, current);
            current.pop_back();
        }
    }

    template<utility::IsIntegralRange T>
    bool SatisfiesCalibrationRequirements(ScratchData& scratchData, Number desiredResult, T&& range)
    {
        const auto numberOfOperands{ range.size() - 1 };
        const auto functionOperatorIterator{ scratchData.mResults.find(numberOfOperands) };
        assert(functionOperatorIterator != scratchData.mResults.end());
        auto& functionOperatorPermutations{ functionOperatorIterator->second };

        const auto rangeViewExcludingFirstElement{ std::ranges::subrange(range.begin() + 1, range.end()) };
        for (auto&& functionOperatorPermutation : functionOperatorPermutations)
        {
            Number accumulator{ range[0] };
            assert(functionOperatorPermutation.size() == rangeViewExcludingFirstElement.size());
            for (const auto& [operatorFunction, number] : std::ranges::views::zip(functionOperatorPermutation, rangeViewExcludingFirstElement))
            {
                accumulator = operatorFunction(accumulator, number);
            }
            if (accumulator == desiredResult)
            {
                return true;
            }
        }

        return false;
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day7, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            mData.push_back(utility::GetNumbers<Number>(rowInput));
            assert(mData.back().size() > 2);
            mNumberofOperandsNeeded.emplace(static_cast<Number>(mData.back().size() - 2));
        }

        assert(utility::Concatenate(10, 24) == 1024);
        assert(utility::Concatenate(2, 2000) == 22000);
    }

    void PerformFirst() override
    {
        // Calculate all the different permutations of operands
        std::vector<std::thread> workers;
        workers.reserve(mNumberofOperandsNeeded.size());
        ScratchData scratchData;
        for (const auto numberOfOperands : mNumberofOperandsNeeded)
        {
            const auto task = [this, &scratchData, numberOfOperands]()
                {
                    auto result{ GetAllOperationsForNumberOfOperands(numberOfOperands,
                        utility::ConvertIntegralCFunctionPointer(&utility::Sum<Number>), utility::ConvertIntegralCFunctionPointer(&utility::Product<Number>)) };
                    std::unique_lock<std::mutex> guard{ scratchData.mResultMapLock };
                    scratchData.mResults.emplace(numberOfOperands, std::move(result));
                };
            workers.emplace_back(task);
        }
        std::ranges::for_each(workers, &std::thread::join);

        Number result{};
        for (auto& row : mData)
        {
            Number desiredNumber{ row[0] };
            auto range{ std::ranges::subrange(row.begin() + 1, row.end()) };
            if (SatisfiesCalibrationRequirements(scratchData, desiredNumber, range))
            {
                result += desiredNumber;
            }
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        std::vector<std::thread> workers;
        workers.reserve(mNumberofOperandsNeeded.size());
        ScratchData scratchData;
        for (const auto numberOfOperands : mNumberofOperandsNeeded)
        {
            const auto task = [this, &scratchData, numberOfOperands]()
                {
                    auto result{ GetAllOperationsForNumberOfOperands(numberOfOperands,
                        utility::ConvertIntegralCFunctionPointer(&utility::Sum<Number>), utility::ConvertIntegralCFunctionPointer(&utility::Product<Number>),
                        utility::ConvertIntegralCFunctionPointer(&utility::Concatenate<Number>)) };
                    std::unique_lock<std::mutex> guard{ scratchData.mResultMapLock };
                    scratchData.mResults.emplace(numberOfOperands, std::move(result));
                };
            workers.emplace_back(task);
        }
        std::ranges::for_each(workers, &std::thread::join);

        Number result{};
        for (auto& row : mData)
        {
            Number desiredNumber{ row[0] };
            auto range{ std::ranges::subrange(row.begin() + 1, row.end()) };
            if (SatisfiesCalibrationRequirements(scratchData, desiredNumber, range))
            {
                result += desiredNumber;
            }
        }
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::vector<std::vector<Number>> mData;
    std::unordered_set<Number> mNumberofOperandsNeeded;
};
