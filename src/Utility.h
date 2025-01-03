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
#include <regex>
#include <type_traits>
#include <functional>
#include <cstdint>
#include <expected>

class AbstractDay;

namespace utility
{
    template<typename T>
    concept IsOrderingOperation = requires(T)
    {
        std::is_same_v<T, std::greater<>> || std::is_same_v<T, std::less<>>;
    };

    template<typename T>
    concept IsInt32Range = requires(T)
    {
        std::ranges::input_range<T>&& std::convertible_to<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>, int32_t>;
    };

    template<typename T>
    concept IsDayAndHasDayString = requires(T)
    {
        std::derived_from<T, AbstractDay>;
        std::same_as<std::remove_cvref_t<decltype(T::sDay)>, std::string_view>;
    };

    template<typename T>
    concept Integral = requires(T)
    {
        std::is_integral_v<T>;
    };

    template<typename T>
    concept SignedIntegral = requires(T)
    {
        Integral<T>;
        std::is_signed_v<T>;
    };

    template<typename T>
    concept UnsignedIntegral = requires(T)
    {
        Integral<T>;
        !SignedIntegral<T>;
    };

    template<typename T>
    concept IsIntegralRange = requires(T)
    {
        std::ranges::input_range<T>;
        Integral<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>;
    };

    template<typename T>
    concept IsSignedIntegralRange = requires(T)
    {
        std::ranges::input_range<T>;
        SignedIntegral<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>;
    };

    template<typename T>
    concept IsUnsignedIntegralRange = requires(T)
    {
        std::ranges::input_range<T>;
        UnsignedIntegral<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>;
    };

    template<typename T>
    concept IsMultiDimensionalRandomAccessRange = requires(T)
    {
        std::ranges::random_access_range<T>;
        std::ranges::random_access_range<typename std::ranges::range_value_t<T>>;
    };

    template<typename T>
    concept IsIntegralOperatorFunction = requires(T)
    {
        Integral<typename T::result_type>;
        std::is_convertible_v<T, std::function<typename T::result_type(typename T::result_type, typename T::result_type)>>;
    };

    template<typename T>
    concept IsIntegralOperatorFunctionRange = requires(T)
    {
        std::ranges::input_range<T>;
        IsIntegralOperatorFunction<std::ranges::range_value_t<T>>;
    };

    template<typename T>
    concept IsMultiDimensionalOperatorFunctionRange = requires(T)
    {
        std::ranges::input_range<T>;
        IsIntegralOperatorFunctionRange<std::ranges::range_value_t<T>>;
    };

    // This is probably screwed up.
    template<typename T, typename ...Args>
    concept IsVariadicIntegralOperatorFunction = requires(T, Args...)
    {
        sizeof ... (Args) > 0;
        Integral<T>;
        (std::is_convertible_v<Args, std::function<T(T, T)>> && ...);
    };

    template<IsMultiDimensionalRandomAccessRange T>
    void PrintElements(T&& vec)
    {
        for (const auto& row : vec)
        {
            for (const auto& element : row)
            {
                std::cout << element << '\n';
            }
        }
    }

    template<Integral T>
    std::function<T(T, T)> ConvertIntegralCFunctionPointer(T(*func)(T, T))
    {
        return std::function<T(T, T)>{func};
    }

    constexpr std::string_view sReleaseFileName{ "input.txt" };
    constexpr std::string_view sTestFileName{ "test.txt" };

    template<utility::Integral T>
    T GetNumberOfDigitsByDivision(T number)
    {
        T result{ 1 };
        while (number /= 10)
        {
            result++;
        }

        return result;
    }

    template<Integral T>
    [[nodiscard]] T GetNumberOfDigitsByLog10(T number)
    {
        return std::log10(number) + 1;
    }

    template<Integral T>
    [[nodiscard]] T GetNumberOfDigitsByString(T number)
    {
        auto numberString{ std::to_string(number) };
        return numberString.length();
    }

    template<Integral T>
    [[nodiscard]] T GetNumberOfDigits(T number)
    {
        return GetNumberOfDigitsByLog10(number);
    }

    template<utility::Integral T>
    T Sum(T element1, T element2)
    {
        return element1 + element2;
    }

    template<utility::Integral T>
    T Product(T element1, T element2)
    {
        return element1 * element2;
    }

    template<utility::Integral T>
    T Concatenate(T element1, T element2)
    {
        std::string num1{ std::to_string(element1) };
        num1.append(std::to_string(element2));

        return ToNumber<T>(num1);
    }

    template<typename... T>
    struct Overload : T...
    {
        using T::operator()...;
    };

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

    template<IsDayAndHasDayString T, InputVersion sInputVersion = InputVersion::release>
    class InputReader
    {
    public:
        [[nodiscard]] std::string Read() const
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

    [[nodiscard]] std::vector<std::string_view> GetStringSplitBy(const std::string& inputString, std::string_view delimiter = "\n")
    {
        std::vector<std::string_view> result{};
        for (const auto& splitElement : std::ranges::views::split(inputString, delimiter))
        {
            result.emplace_back(splitElement);
        }

        return result;
    }

    template<Integral T = int32_t>
    [[nodiscard]] T ToNumber(char inputData)
    {
        if (inputData < '0' || inputData > '9')
        {
            assert(false);
        }

        return static_cast<T>(inputData & 0x0F);
    }

    template<Integral T = int32_t>
    [[nodiscard]] T ToNumber(std::string_view inputData)
    {
        bool isNegative{ false };
        if (inputData.front() == '-')
        {
            isNegative = true;
            inputData = inputData.substr(1);
        }

        T result{};
        for (const char c : inputData)
        {
            if (c < '0' || c > '9')
            {
                std::cout << "invalid input: " << inputData << '\n';
                break;
            }
            result *= 10;
            result += static_cast<T>(c & 0x0F);
        }

        if (isNegative)
        {
            result *= -1;
        }

        return result;
    }

    template<Integral T = int32_t>
    [[nodiscard]] std::vector<T> GetNumbers(std::string_view data)
    {
        constexpr std::string_view sMatchExpression{ "\\d{1,}" };
        std::regex baseRegex{ sMatchExpression.data() };

        std::regex_token_iterator<std::string_view::iterator> endSentinelIterator;
        std::regex_token_iterator<std::string_view::iterator> matchIterator{ data.begin(), data.end(), baseRegex };

        std::vector<T> result;
        result.reserve(2);
        while (matchIterator != endSentinelIterator)
        {
            if (matchIterator->first - 1 >= data.begin() && *(matchIterator->first - 1) == '-')
            {
                result.emplace_back(utility::ToNumber<T>(std::string_view{ matchIterator->first - 1, matchIterator->second }));
            }
            else
            {
                result.emplace_back(utility::ToNumber<T>(std::string_view{ matchIterator->first, matchIterator->second }));
            }

            matchIterator++;
        }

        return result;
    }

    void PrintDetails(InputVersion version, Part part)
    {
        std::cout << "[Version]: " << "\033[7;1;31m" <<
            sInputVersionStringMap.at(version) <<
            "\033[0m" <<
            " [Part]: " << "\033[7;1;31m" << sPartStringMap.at(part) <<
            "\033[0m"
            << " Result: ";
    }

    template<Integral T = int32_t>
    struct Position
    {
        using PositionType = T;
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

        Position operator-(const Position& otherPosition)
        {
            return { this->mRow - otherPosition.mRow, this->mCol - otherPosition.mCol };
        }

        Position operator+(const Position& otherPosition)
        {
            return { this->mRow + otherPosition.mRow, this->mCol + otherPosition.mCol };
        }

        Position operator*(const Position& otherPosition)
        {
            return { this->mRow * otherPosition.mRow, this->mCol * otherPosition.mCol };
        }

        Position operator/(const Position& otherPosition)
        {
            return { this->mRow / otherPosition.mRow, this->mCol / otherPosition.mCol };
        }

        template<Integral U = T>
        Position operator-(U scalar) const
        {
            return { mRow - scalar, mCol - scalar };
        }

        template<Integral U = T>
        Position operator+(U scalar) const
        {
            return { mRow + scalar, mCol + scalar };
        }

        template<Integral U = T>
        Position operator*(U scalar) const
        {
            return { mRow * scalar, mCol * scalar };
        }

        template<Integral U = T>
        Position operator/(U scalar) const
        {
            return { mRow / scalar, mCol / scalar };
        }

        Position& operator-=(const Position& otherPosition)
        {
            this->mRow -= otherPosition.mRow;
            this->mCol -= otherPosition.mCol;
            return *this;
        }

        template<Integral U = T>
        Position& operator-=(U scalar)
        {
            this->mRow -= scalar;
            this->mCol -= scalar;
            return *this;
        }

        Position& operator+=(const Position& otherPosition)
        {
            this->mRow += otherPosition.mRow;
            this->mCol += otherPosition.mCol;
            return *this;
        }

        template<Integral U = T>
        Position& operator+=(U scalar)
        {
            this->mRow += scalar;
            this->mCol += scalar;
            return *this;
        }

        Position& operator*=(const Position& otherPosition)
        {
            this->mRow *= otherPosition.mRow;
            this->mCol *= otherPosition.mCol;
            return *this;
        }

        template<Integral U = T>
        Position& operator*=(U scalar)
        {
            this->mRow *= scalar;
            this->mCol *= scalar;
            return *this;
        }

        Position& operator/=(const Position& otherPosition)
        {
            this->mRow /= otherPosition.mRow;
            this->mCol /= otherPosition.mCol;
            return *this;
        }

        template<Integral U = T>
        Position& operator/=(U scalar)
        {
            this->mRow /= scalar;
            this->mCol /= scalar;
            return *this;
        }
    };

    enum class ErrorType
    {
        outOfBounds,
    };

    template<utility::IsMultiDimensionalRandomAccessRange T>
    struct TwoDimensionalVectorType
    {
        using type = std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<std::remove_cvref_t<std::remove_pointer_t<std::ranges::range_value_t<T>>>>>>;
        using pointer_type = type*;
    };

    template<utility::IsMultiDimensionalRandomAccessRange T, SignedIntegral U>
    std::expected<typename TwoDimensionalVectorType<T>::pointer_type, ErrorType> GetItemAt(T&& vector, Position<U> position)
    {
        if (position.mRow < 0 || position.mRow >= vector.size() || position.mCol < 0 || position.mCol >= vector[position.mRow].size())
        {
            return std::unexpected{ ErrorType::outOfBounds };
        }

        return &vector[position.mRow][position.mCol];
    }

    enum class Direction
    {
        left,
        right,
        up,
        down,
        leftUp,
        leftDown,
        rightUp,
        rightDown,
    };

    using DirectionData = utility::Position<int32_t>;
    static const std::unordered_map<Direction, DirectionData> sBaseDirectionsMap
    {
        {Direction::left,   {0,-1} },
        {Direction::right,  {0,1}},
        {Direction::up,     {-1, 0}},
        {Direction::down,   {1, 0}},
    };

    static const std::unordered_map<Direction, DirectionData> sFullDirectionsMap
    {
        {Direction::left,   {0,-1} },
        {Direction::right,  {0,1}},
        {Direction::up,     {-1, 0}},
        {Direction::down,   {1, 0}},
        {Direction::leftUp, {-1,-1}},
        {Direction::leftDown, {1,-1}},
        {Direction::rightDown, {1, 1}},
        {Direction::rightUp, {-1,1}},
    };

    static constexpr std::array<std::pair<DirectionData, Direction>, 8> sFullReverseDirectionsArray
    {
        std::pair<DirectionData,Direction>{{0,-1},Direction::left },
        {{0,1}, Direction::right},
        {{-1, 0}, Direction::up},
        {{1, 0}, Direction::down},
        {{-1,-1}, Direction::leftUp},
        {{1,-1}, Direction::leftDown},
        {{1, 1}, Direction::rightDown},
        {{-1,1}, Direction::rightUp},
    };

    static constexpr std::array<Direction, 4> sBaseDirectionOrder{ Direction::up, Direction::right, Direction::down, Direction::left };
    static constexpr std::array<Position<int32_t>, 4> sBaseDirectionValuesInOrder{
        Position<int32_t>{.mRow = -1, .mCol = 0},
        Position<int32_t>{.mRow = 0, .mCol = 1},
        Position<int32_t>{.mRow = 1, .mCol = 0},
        Position<int32_t>{.mRow = 0, .mCol = -1} };

    static constexpr size_t GetDirectionIndex(Direction direction)
    {
        const auto directionIterator{ std::ranges::find(sBaseDirectionOrder,direction) };
        if (directionIterator == sBaseDirectionOrder.end())
        {
            assert(false);
            return 0;
        }

        return std::distance(sBaseDirectionOrder.begin(), directionIterator);
    }

    static constexpr size_t GetNextDirectionIndex(Direction direction)
    {
        return (GetDirectionIndex(direction) + 1) % sBaseDirectionOrder.size();
    }

    static constexpr Direction GetNextDirection(Direction direction)
    {
        return sBaseDirectionOrder[GetNextDirectionIndex(direction)];
    }

    static_assert(GetNextDirection(Direction::up) == Direction::right);
    static_assert(GetNextDirection(Direction::right) == Direction::down);
    static_assert(GetNextDirection(Direction::down) == Direction::left);
    static_assert(GetNextDirection(Direction::left) == Direction::up);

    template<SignedIntegral T>
    static constexpr Position<T> GetDirectionValue(Direction direction)
    {
        return sBaseDirectionValuesInOrder[GetDirectionIndex(direction)];
    }

    template<SignedIntegral T>
    static constexpr Position<T> GetNextDirectionValue(Direction direction)
    {
        return sBaseDirectionValuesInOrder[GetNextDirectionIndex(direction)];
    }

    DirectionData GetDirectionData(Direction direction)
    {
        if (auto directionIterator{ sFullDirectionsMap.find(direction) }; directionIterator != sFullDirectionsMap.end())
        {
            return directionIterator->second;
        }

        assert(false);
        return {};
    }

    template<typename... T>
    void PrintResult(T&&... args)
    {
        std::cout << "\033[4;1;32m";
        (std::cout << ... << std::forward<T>(args));
        std::cout << '\n' << "\033[0m";
    }
}
