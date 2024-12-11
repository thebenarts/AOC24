#pragma once
#include "Utility.h"
#include <chrono>

static constexpr std::string_view sFirstPartResultString{ "First Part Result : " };
static constexpr std::string_view sSecondPartResultString{ "Second Part Result : " };

class AbstractDay
{
    virtual void Perform(utility::Part part) = 0;

protected:
    virtual void ReadInput() = 0;
    virtual void PerformFirst() = 0;
    virtual void PerformSecond() = 0;
};

template<utility::InputVersion version = utility::InputVersion::release>
class DayBase : public AbstractDay
{
public:
    void Perform(utility::Part part) override
    {
        ReadInput();
        switch (part)
        {
        case utility::Part::first:
        {
            PerformFirst();
        }
        break;
        case utility::Part::second:
        {
            PerformSecond();
        }
        break;
        case utility::Part::both:
        {
            PerformFirst();
            PerformSecond();
        }
        break;
        }
    }
};

template<template<utility::InputVersion version = utility::InputVersion::release> class Day>
class DayWrapper
{
public:
    void Perform()
    {
        Day<utility::InputVersion::test> testVersion;
        const auto testStart{ std::chrono::high_resolution_clock::now() };
        testVersion.Perform(utility::Part::both);
        const auto testEnd{ std::chrono::high_resolution_clock::now() };
        std::chrono::nanoseconds testDurationInMilliseconds{ testEnd - testStart };
        std::cout << "Test took: " << testDurationInMilliseconds.count() << " nanoseconds \n";

        Day<utility::InputVersion::release> releaseVersion;
        const auto releaseStart{ std::chrono::high_resolution_clock::now() };
        releaseVersion.Perform(utility::Part::both);
        const auto releaseEnd{ std::chrono::high_resolution_clock::now() };
        std::chrono::nanoseconds releaseDurationInMilliseconds{ releaseEnd - releaseStart };
        std::cout << "Release took: " << releaseDurationInMilliseconds.count() << " nanoseconds\n";

    };
};