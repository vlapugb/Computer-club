#pragma once

#include "error_handler.h"
#include <charconv>
#include <chrono>
#include <deque>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

enum class EventType {
    IN = 1,
    SEATING = 2,
    WAITING = 3,
    OUT = 4,
    OUTGOING_OUT = 11,
    OUTGOING_SEATING = 12,
    OUTGOING_ERROR = 13
};

struct MetaData {
    size_t tableCount;
    std::pair<std::string, std::string> workingTime;
    unsigned long long cost;
};

struct Event {
    std::string time;
    EventType type;
    std::string ClientName;
    std::optional<size_t> tableNumber = std::nullopt;
};

int64_t toMinutes(const std::string& time);

class Parser {
  public:
    explicit Parser(ErrorHandler& errorHandler, const std::string& filename);
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) noexcept = default;
    Parser& operator=(Parser&&) noexcept = default;
    ~Parser() = default;

    void parseFile();
    void flushBuffer() const;

    std::deque<Event>& getEvents() noexcept { return events_; }
    MetaData& getMetaInfo() noexcept { return metadata_; }

  private:
    void openFile();
    void parseMetaInfo();
    void parseTableValues();
    void parseWorkingTime();
    void parseCost();
    void parseEvents();
    template <typename T, bool IsEnum = std::is_enum_v<T>> struct ConvValueType {
        using type = T;
    };

    template <typename T> struct ConvValueType<T, true> {
        using type = std::underlying_type_t<T>;
    };

    template <typename T> T convToType(const std::string& str) {
        using ValueType = typename ConvValueType<T>::type;
        ValueType value;

        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc() || ptr != str.data() + str.size()) {
            throw std::invalid_argument("bad numeric value: " + str);
        }

        return static_cast<T>(value);
    }

    ErrorHandler& errorHandler_;
    std::string filename_;
    std::ifstream file_;
    std::vector<std::string> buffer_;
    MetaData metadata_;
    std::deque<Event> events_;
};
