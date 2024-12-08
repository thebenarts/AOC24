#pragma once
#include <assert.h>
#include <string>
#include <string_view>
#include "DirectoryMacro.h"
#include <concepts>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <iostream>
#include <unordered_map>

class AbstractDay;

namespace utility
{
    constexpr std::string_view sReleaseFileName{ "input.txt" };
    constexpr std::string_view sTestFileName{ "test.txt" };

    enum class Part
    {
        first,
        second,
        both,
    };

    enum class InputVersion
    {
        release,
        test,
    };

    static const std::unordered_map<InputVersion, std::string_view> sInputVersionStringMap
    {
        {InputVersion::release, "Release"},
        { InputVersion::test, "Test" }
    };

    static const std::unordered_map<Part, std::string_view> sPartStringMap
    {
        {Part::first, "First"},
        {Part::second, "Second"},
        {Part::both, "Both"}
    };


    template<typename T>
    concept IsDayAndHasDayString = requires(T)
    {
        requires std::derived_from<T, AbstractDay>;
        requires std::same_as<std::remove_cvref_t<decltype(T::sDay)>, std::string_view>;
    };

    template<IsDayAndHasDayString T, InputVersion sInputVersion = InputVersion::release>
    class InputReader
    {
    public:
        std::string Read() const
        {
            std::filesystem::path path{ INPUT_DIR };
            path.append(T::sDay);
            switch (sInputVersion)
            {
            case InputVersion::release:
            {
                path /= sReleaseFileName;
            }
            break;
            case InputVersion::test:
            {
                path /= sTestFileName;
            }
            break;
            }

            if (std::ifstream inputReader{ path.make_preferred() ,std::ios::ate })
            {
                const auto size{ inputReader.tellg() };
                std::string result(size, '\0'); // use (), so we don't have to do narrowing conversion by hand. :(
                inputReader.seekg(0);
                inputReader.read(&result[0], size);
                return result;
            }

            return {};
        };
    };

    std::vector<std::string_view> GetStringSplitBy(const std::string& inputString, std::string_view delimiter = "\n")
    {
        std::vector<std::string_view> result{};
        for (const auto& splitElement : std::ranges::views::split(inputString, delimiter))
        {
            result.emplace_back(splitElement);
        }

        return result;
    }

    int32_t ToNumber(std::string_view inputData)
    {
        int32_t result{};
        for (const char c : inputData)
        {
            if (c < '0' || c > '9')
            {
                std::cout << "invalid input: " << inputData << '\n';
                break;
            }
            result *= 10;
            result += static_cast<int32_t>(c & 0x0F);
        }
        return result;
    }

    void PrintDetails(InputVersion version, Part part)
    {
        std::cout << "[Version]: " << sInputVersionStringMap.at(version) << " [Part]: " << sPartStringMap.at(part) << " Result: ";
    }
}
