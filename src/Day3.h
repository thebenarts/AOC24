#pragma once

#include "Utility.h"
#include "Day.h"
#include <regex>

template<utility::InputVersion version = utility::InputVersion::release>
class Day3 : public DayBase<version>
{
public:
    static constexpr std::string_view sDay{ "day3" };

    enum class Instruction
    {
        enable,     // do
        disable,    // don't
        mul,        // mul
    };

private:

    void StoreMatch(const std::regex_token_iterator<std::string::iterator>& match, Instruction instruction)
    {
        mMatches.push_back({ std::string_view{ match->first, match->second }, instruction });
    }

    int32_t MultiplyNumbers(std::vector<int32_t> numbers)
    {
        if (numbers.empty())
        {
            return {};
        }

        int32_t result{ 1 };
        for (const auto number : numbers)
        {
            result *= number;
        }
        return result;
    }

    void GetInstruction(Instruction type)
    {
        constexpr std::string_view sMatchMulExpression{ "mul\\(\\d{1,},\\d{1,}\\)" };
        constexpr std::string_view sMatchDoExpression{ "do\\(\\)" };
        constexpr std::string_view sMatchDontExpression{ "don't\\(\\)" };

        std::regex baseRegex{};
        switch (type)
        {
        case Instruction::enable:
        {
            baseRegex = std::regex{ sMatchDoExpression.data() };
        }break;
        case Instruction::disable:
        {
            baseRegex = std::regex{ sMatchDontExpression.data() };
        }break;
        case Instruction::mul:
        {
            baseRegex = std::regex{ sMatchMulExpression.data() };
        }break;
        }
        std::regex_token_iterator<std::string::iterator> endSentinelIterator;
        std::regex_token_iterator<std::string::iterator> matchIterator{ mBuffer.begin(), mBuffer.end(), baseRegex };
        while (matchIterator != endSentinelIterator)
        {
            StoreMatch(matchIterator++, type);
        }
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day3, version> inputReader;
        mBuffer = inputReader.Read();
        GetInstruction(Instruction::mul);
        GetInstruction(Instruction::enable);
        GetInstruction(Instruction::disable);
        std::ranges::sort(mMatches, [](const auto& pair1, const auto& pair2) {return pair1.first.data() < pair2.first.data(); });
    }

    void PerformFirst() override
    {
        int32_t result{};
        for (const auto [match, instructionType] : mMatches)
        {
            if (instructionType == Instruction::mul)
            {
                result += MultiplyNumbers(utility::GetNumbers(match));
            }
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << "\n";
    }

    void PerformSecond() override
    {
        bool enabled{ true };
        int32_t result{};
        for (const auto [match, instructionType] : mMatches)
        {
            switch (instructionType)
            {
            case Instruction::enable:
            {
                enabled = true;
            }break;
            case Instruction::disable:
            {
                enabled = false;
            }break;
            case Instruction::mul:
            {
                if (enabled)
                {
                    result += MultiplyNumbers(utility::GetNumbers(match));
                }
            } break;
            }
        }
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::vector<std::pair<std::string_view, Instruction>> mMatches;
};
