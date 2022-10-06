#pragma once

#include <cstddef>
#include <optional>
#include <vector>
#include <stdint.h>

struct FilterResponse
{
    bool accept = false;
    std::optional<std::vector<uint8_t>> overridePacketData = std::nullopt;

    // set override packet data from C-style packetData + packetSize
    // (values are copied at call time; any changes made afterwards are not reflected)
    void setOverridePacketData(uint8_t const* packetData, size_t packetSize)
    {
        overridePacketData.emplace(packetData, packetData + packetSize);
    }

    // set override packet data from C++ containers
    // (values are copied at call time; any changes made afterwards are not reflected)
    template <typename Container>
    void setOverridePacketData(Container const& c)
    {
        overridePacketData.emplace(std::begin(c), std::end(c));
    }

    // clear any previously-set override packet data
    // (the response starts out with no override data set)
    void clearOverridePacketData()
    {
        overridePacketData.reset();
    }
};

const FilterResponse ACCEPT = { true };
const FilterResponse DROP = { false };

enum Direction { INCOMING, OUTGOING };

FilterResponse processPacket(uint8_t const* packetData, size_t packetSize, Direction direction);
