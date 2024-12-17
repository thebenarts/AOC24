#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>
#include <variant>
#include <optional>
#include <list>
#include <forward_list>
#include <numeric>

template<utility::InputVersion version = utility::InputVersion::release>
class Day11 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day11" };

private:
    using Number = uint64_t;
    using Vector = std::vector<Number>;
    using Cache = std::unordered_map<Number, Number>;

    struct ScratchData
    {
        Vector mNumberResults;
        std::mutex mLock;
    };

    enum class Rule
    {
        IsZero,
        IsEvenDigits,
        IsOther,
    };

    bool SatisfiesZeroRequirement(Number number)
    {
        return number == 0;
    };

    bool SatisfiesEvenNumberofDigits(Number number)
    {
        return utility::GetNumberOfDigits(number) % 2 == 0;
    }

    Rule GetRuleToApply(Number number)
    {
        if (SatisfiesZeroRequirement(number))
        {
            return Rule::IsZero;
        }
        else if (SatisfiesEvenNumberofDigits(number))
        {
            return Rule::IsEvenDigits;
        }

        return Rule::IsOther;
    }

    void ApplyIsZero(Number count, Cache& cache)
    {
        auto [iterator, inserted] {cache.try_emplace(1, 0)};
        iterator->second += count;
    }

    template<utility::Integral T>
    [[nodiscard]] std::pair<T, T> SplitNumber(T number)
    {
        auto numberString{ std::to_string(number) };
        auto numberDigits{ numberString.size() };
        std::string_view firstHalf{ numberString.begin(), numberString.begin() + numberDigits / 2 };
        std::string_view secondHalf{ numberString.begin() + numberDigits / 2, numberString.end() };
        return { utility::ToNumber<Number>(firstHalf), utility::ToNumber<Number>(secondHalf) };
    }

    void ApplyIsEvenNumberOfDigits(Number number, Number count, Cache& cache)
    {
        auto numbers{ SplitNumber(number) };
        auto [firstIterator, _] {cache.try_emplace(numbers.first, 0)};
        firstIterator->second += count;
        auto [secondIterator, __] {cache.try_emplace(numbers.second, 0)};
        secondIterator->second += count;
    }

    void ApplyOtherRule(Number number, Number count, Cache& cache)
    {
        auto [iterator, _] {cache.try_emplace(number * 2024, 0)};
        iterator->second += count;
    }

    void ApplyRule(Number number, Number count, Cache& cache, Rule ruleToApply)
    {
        switch (ruleToApply)
        {
        case Rule::IsZero:
        {
            ApplyIsZero(count, cache);
            return;
        }
        break;
        case Rule::IsEvenDigits:
        {
            ApplyIsEvenNumberOfDigits(number, count, cache);
            return;
        }
        break;
        case Rule::IsOther:
        {
            ApplyOtherRule(number, count, cache);
            return;
        }
        break;
        }
        assert(false);
    }

    void PerformRequiredAction(Number number, Number count, Cache& cache)
    {
        return ApplyRule(number, count, cache, GetRuleToApply(number));
    }

    void PrintStones(const Vector& vector)
    {
        std::ranges::for_each(vector, [](const auto& num) {std::cout << " " << num << " "; });
    }

    Number GetNumberOfStonesInBlinks(const Vector& stones, Number numberOfBlinks)
    {
        ScratchData scratchData;
        std::vector<std::thread> workers;
        workers.reserve(stones.size());
        for (auto&& [index, number] : stones | std::ranges::views::enumerate)
        {
            workers.emplace_back([=, this, &scratchData] {
                Cache cache;
                cache.try_emplace(number, 1);
                for (Number blink = 1; blink <= numberOfBlinks; ++blink)
                {
                    Cache newCache;
                    newCache.reserve(cache.size());
                    for (const auto& [stoneNumber, stoneCount] : cache)
                    {
                        PerformRequiredAction(stoneNumber, stoneCount, newCache);
                    }

                    cache = std::move(newCache);
                }

                auto sumCount = [](auto sum, auto pair)->Number {return sum + pair.second; };
                Number result{ std::reduce(cache.begin(), cache.end(), Number{0}, sumCount) };
                std::unique_lock<std::mutex> lock{ scratchData.mLock };
                scratchData.mNumberResults.push_back(result);
                std::cout << "WorkerIndex: " << index << " Blinked: " << numberOfBlinks << '\n';
                std::cout << "StoneNumber: " << number << " Resulted In: " << result << "\n";
                });
        }

        std::ranges::for_each(workers, &std::thread::join);
        Number result{ std::reduce(scratchData.mNumberResults.begin(), scratchData.mNumberResults.end()) };
        return result;
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day11, version> inputReader;
        mBuffer = inputReader.Read();
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            mStones = utility::GetNumbers<Number>(rowInput);
        }
    }

    void PerformFirst() override
    {
        const auto result{ GetNumberOfStonesInBlinks(mStones,25) };
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        const auto result{ GetNumberOfStonesInBlinks(mStones,75) };
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    Vector mStones;
};
