#pragma once
#include "Utility.h"
#include "Day.h"

#include <ranges>
#include <vector>
#include <span>
#include <unordered_map>
#include <variant>

template<utility::InputVersion version = utility::InputVersion::release>
class Day9 : public DayBase<version>
{
    using Number = uint64_t;

    struct Data
    {
        using DataType = std::variant<std::monostate, Number>;
        Data(Number size, DataType type) : mData(size, type) {}

        std::vector<DataType> mData;
    };

    bool IsEmptyField(Data::DataType variantValue)
    {
        return std::visit(utility::Overload{ [](std::monostate) {return true; },
                [](Number) {return false; } }, variantValue);
    }

    int32_t GetFirstEmptyField(const Data& data)
    {
        for (const auto [index, value] : data.mData | std::ranges::views::enumerate)
        {
            if (IsEmptyField(value))
            {
                return index;
            }
        }

        return -1;
    };

    std::pair<int32_t, int32_t> GetNumberOfConsecutiveFreeFields(const Data& data)
    {
        const auto firstEmptyField{ GetFirstEmptyField(data) };
        if (firstEmptyField == -1)
        {
            return { -1, 0 };
        }

        return { firstEmptyField, static_cast<int32_t>(std::distance(data.mData.begin() + firstEmptyField, data.mData.end())) };
    }

    int32_t GetFirstValueField(const Data& data)
    {
        for (const auto [index, value] : data.mData | std::ranges::views::enumerate)
        {
            if (!IsEmptyField(value))
            {
                return index;
            }
        }

        return -1;
    }

    void BringDataToFront(Data& orderData)
    {
        auto firstValueField{ GetFirstValueField(orderData) };
        auto firstEmptyField{ GetFirstEmptyField(orderData) };
        if (firstEmptyField == -1 || firstValueField == -1)
        {
            return;
        }

        if (firstValueField < firstEmptyField)
        {
            assert(false);
            return;
        }

        std::ranges::rotate(orderData.mData.begin() + firstEmptyField, orderData.mData.begin() + firstValueField, orderData.mData.end());
    }

    template<std::ranges::random_access_range T>
    void MigrateData(T&& migrateTo, T&& migrateFrom)
    {
        for (auto [elementToMigrateTo, elementToMigrateFrom] : std::ranges::views::zip(migrateTo, migrateFrom))
        {
            if (IsEmptyField(elementToMigrateFrom))
            {
                break;
            }

            std::swap(elementToMigrateTo, elementToMigrateFrom);
        }
    }

    template<std::ranges::random_access_range T>
    void SwapData(T&& migrateTo, T&& migrateFrom)
    {
        assert(migrateTo.size() == migrateFrom.size());
        for (auto [elementToMigrateTo, elementToMigrateFrom] : std::ranges::views::zip(migrateTo, migrateFrom))
        {
            //if (IsEmptyField(elementToMigrateFrom))
            //{
            //    break;
            //}

            std::swap(elementToMigrateTo, elementToMigrateFrom);
        }
        //std::ranges::swap(migrateTo, migrateFrom);
    }

public:
    static constexpr std::string_view sDay{ "day9" };

private:
    void InterpretData(std::string_view data)
    {
        Number dataId{ 0 };
        bool isDataPoint{ true };
        mData.reserve(data.size());
        for (const auto character : data)
        {
            Number dataSize{ utility::ToNumber<Number>(character) };
            if (dataSize != 0)
            {
                if (isDataPoint)
                {
                    mData.emplace_back(dataSize, dataId++);
                }
                else
                {
                    mData.emplace_back(dataSize, std::monostate{});
                }
            }
            isDataPoint = !isDataPoint;
        }
    }

    std::vector<Data> CompactData()
    {
        std::vector<Data> compactData{ mData };
        int forwardIndex{ 0 };
        int backwardsIndex{ static_cast<int>(compactData.size() - 1) };
        while (forwardIndex < backwardsIndex)
        {
            auto& migrateTo{ compactData[forwardIndex] };
            auto emptyFieldIndexForwardIterator{ GetFirstEmptyField(migrateTo) };
            if (-1 == emptyFieldIndexForwardIterator)
            {
                forwardIndex++;
                continue;
            }

            auto& migrateFrom{ compactData[backwardsIndex] };
            auto valueFieldIndexBackwardsIterator{ GetFirstValueField(migrateFrom) };
            if (-1 == valueFieldIndexBackwardsIterator)
            {
                backwardsIndex--;
                continue;
            }

            MigrateData(std::ranges::subrange(migrateTo.mData.begin() + emptyFieldIndexForwardIterator, migrateTo.mData.end()),
                std::ranges::subrange(migrateFrom.mData.begin() + valueFieldIndexBackwardsIterator, migrateFrom.mData.end()));
            BringDataToFront(migrateFrom);
        }

        return compactData;
    }

    std::vector<Data> CompactDataWithKeepingItIntact()
    {
        std::vector<Data> compactData{ mData };

        for (auto [index, dataToMigrate] : compactData | std::ranges::views::enumerate | std::ranges::views::reverse)
        {
            const auto firstValueFieldIndex{ GetFirstValueField(dataToMigrate) };
            if (firstValueFieldIndex == -1)
            {
                continue;
            }
            assert(firstValueFieldIndex == 0);
            const auto sizeToMigrate{ dataToMigrate.mData.size() };

            for (auto& emptyFieldToMigrateTo : std::ranges::subrange(compactData.begin(), compactData.begin() + index))
            {
                if (GetFirstEmptyField(emptyFieldToMigrateTo) == -1)
                {
                    continue;
                }

                if (const auto [emptyFieldStartIndex, emptyFieldSize] {GetNumberOfConsecutiveFreeFields(emptyFieldToMigrateTo)}; emptyFieldSize >= sizeToMigrate)
                {
                    SwapData(std::ranges::subrange(emptyFieldToMigrateTo.mData.begin() + emptyFieldStartIndex, emptyFieldToMigrateTo.mData.begin() + emptyFieldStartIndex + sizeToMigrate),
                        std::ranges::subrange(dataToMigrate.mData.begin(), dataToMigrate.mData.end()));
                    break;
                }
            }
        }

        return compactData;
    }

    void ReadInput() override
    {
        using namespace std::literals;
        utility::InputReader<Day9, version> inputReader;
        mBuffer = inputReader.Read();
        InterpretData(mBuffer);
    }

    void PerformFirst() override
    {
        auto compactedData{ CompactData() };
        int32_t index{ 0 };
        Number result{ 0 };
        for (const auto& data : compactedData)
        {
            for (const auto& value : data.mData)
            {
                if (IsEmptyField(value))
                {
                    break;
                }

                result += (std::get<Number>(value) * index);
                ++index;
            }
        }
        utility::PrintDetails(version, utility::Part::first);
        std::cout << result << '\n';
    }

    void PrintData(std::span<Data> dataToPrint)
    {
        for (const auto& data : dataToPrint)
        {
            for (const auto& value : data.mData)
            {
                if (IsEmptyField(value))
                {
                    std::cout << '.';
                    continue;
                }
                std::cout << std::get<Number>(value);
            }
        }
    }

    void PerformSecond() override
    {
        auto compactedData{ CompactDataWithKeepingItIntact() };
        int32_t index{ 0 };
        Number result{ 0 };

        for (const auto& data : compactedData)
        {
            for (const auto& value : data.mData)
            {
                if (IsEmptyField(value))
                {
                    ++index;
                    continue;
                }

                result += (std::get<Number>(value) * index);
                ++index;
            }
        }
        utility::PrintDetails(version, utility::Part::second);
        std::cout << result << '\n';
    }

private:
    std::string mBuffer;
    std::vector<Data> mData;
};
