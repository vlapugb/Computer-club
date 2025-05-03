#pragma once
#include <ostream>
#include <unordered_map>
#include "parser.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <vector>

struct clientInfo
{
    bool seated = false;
    bool inQueue = false;
    size_t table;
    std::chrono::minutes seatTime{};
};

class Club
{
private:
    std::unordered_map<std::string, clientInfo> clients_;
    Parser &parser_;
    std::deque<Event>& events_;
    MetaData& metadata_;
    std::chrono::minutes openTime;
    std::chrono::minutes endTime;
    size_t tableCount;
    std::deque<std::string> waiting_;
    std::vector<std::optional<std::string>> tables_;
    std::vector<std::chrono::minutes> placedTableTime;
    unsigned long long cost;
    void unwrapMetaInfo();
    bool freeTableExists() const;
    void countSummary();
public:
    explicit Club(Parser &parser);
    Club() = delete;
    ~Club() = default;
    void run();
    void messagesProcesser();
};
