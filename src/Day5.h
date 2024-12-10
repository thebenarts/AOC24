#pragma once
#include "Utility.h"
#include "Day.h"

#include <unordered_map>
#include <vector>
#include <span>
#include <thread>
#include <mutex>

template<utility::InputVersion version = utility::InputVersion::release>
class Day5 : public DayBase<version>
{
public:

    static constexpr std::string_view sDay{ "day5" };
private:
    using PageNumber = int32_t;

    template<utility::IsIntegralRange T>
    struct ElementResultType
    {
        using type = std::vector<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>;
    };

    // returns the requirements that make elementToCheck invalid
    template<utility::IsIntegralRange T>
    ElementResultType<T>::type GetInvalidElementsForElement(T&& range, PageNumber elementToCheck)
    {
        const auto orderingIterator{ mOriginalOrdering.find(elementToCheck) };
        if (orderingIterator == mOriginalOrdering.end())
        {
            return {};
        }

        using ElementType = std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>;
        typename ElementResultType<T>::type result;

        const auto elementToCheckIterator{ std::ranges::find(range,elementToCheck) };
        assert(elementToCheckIterator != range.end());

        const auto& requirements{ orderingIterator->second };
        auto numbersPriorToCurrentNumber{ std::ranges::subrange(range.begin(), elementToCheckIterator) };
        for (const auto requirementNumber : requirements)
        {
            if (std::ranges::any_of(numbersPriorToCurrentNumber, [requirementNumber](const auto& otherNumber) {return otherNumber == requirementNumber; }))
            {
                result.push_back(requirementNumber);
            }
        }

        return result;
    }

    // returns the numbers that fail their own requirements
    template<utility::IsIntegralRange T>
    ElementResultType<T>::type GetInvalidElements(T&& range)
    {
        using ElementType = std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>;
        typename ElementResultType<T>::type result;

        for (const auto [index, number] : range | std::ranges::views::enumerate | std::ranges::views::reverse)
        {
            auto requirementsThatMakeThisElementFail{ GetInvalidElementsForElement(range, number) };
            if (!requirementsThatMakeThisElementFail.empty())
            {
                result.push_back(number);
            }
        }

        return result;
    }

    template<utility::IsIntegralRange T>
    bool SatisfiesOrderingRequirements(T&& range)
    {
        const auto invalidElements{ GetInvalidElements(range) };
        return invalidElements.empty();
    }

    template<utility::IsIntegralRange T>
    void FixOrderingOfElement(T&& range, PageNumber elementToFix)
    {
        const auto elementToFixIterator{ std::ranges::find(range,elementToFix) };
        assert(elementToFixIterator != range.end());

        const auto failingRequirementsForThisElement{ GetInvalidElementsForElement(range,elementToFix) };
        assert(!failingRequirementsForThisElement.empty());

        const auto elementToMoveAhead{ std::ranges::find_first_of(range, failingRequirementsForThisElement) };
        assert(elementToMoveAhead != range.end());

        std::ranges::rotate(elementToMoveAhead, elementToFixIterator, elementToFixIterator + 1);
    }

    template<utility::IsIntegralRange T>
    void FixOrdering(T&& range)
    {
        using ElementType = std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>;
        typename ElementResultType<T>::type invalidElements;

        while (!(invalidElements = GetInvalidElements(range)).empty())
        {
            FixOrderingOfElement(range, invalidElements.front());
        }

        mFixedUpdates.push_back(range);
    }


    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day5, version> inputReader;
        mBuffer = inputReader.Read();
        bool updatesNext{ false };
        for (const auto rowInput : mBuffer | std::ranges::views::split("\n"sv)
            | std::ranges::views::transform([](auto&& range) {return std::string_view{ range }; }))
        {
            if (rowInput.empty())
            {
                updatesNext = true;
                continue;
            }

            std::vector<PageNumber> numbers{ utility::GetNumbers(rowInput) };
            assert(!numbers.empty());

            // Ordering
            if (!updatesNext)
            {
                const auto [iterator, _] {mOriginalOrdering.try_emplace(numbers.front())};
                // no need to check if inserted, we only care about the vector being there, which is guaranteed.
                iterator->second.push_back(numbers.back());
            }
            else
            {
                mUpdates.push_back(std::move(numbers));
            }
        }
    }

    void PerformFirst() override
    {
        PageNumber result{};
        for (const auto& update : mUpdates)
        {
            if (SatisfiesOrderingRequirements(update))
            {
                const auto middleElementIndex{ update.size() / 2 };
                result += update[middleElementIndex];
            }
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PerformSecond() override
    {
        PageNumber result{};
        for (auto& update : mUpdates)
        {
            if (!SatisfiesOrderingRequirements(update))
            {
                FixOrdering(update);
            }
        }

        for (const auto& fixedOrder : mFixedUpdates)
        {
            const auto middleElementIndex{ fixedOrder.size() / 2 };
            result += fixedOrder[middleElementIndex];
        }
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::unordered_map<PageNumber, std::vector<PageNumber>> mOriginalOrdering;
    std::vector<std::vector<PageNumber>> mUpdates;
    std::vector<std::vector<PageNumber>> mFixedUpdates;
};
