#include "parser.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

Parser::Parser(ErrorHandler& handler, const std::string& file)
    : errorHandler_(handler), filename_(file) {}

void Parser::openFile() {
    file_.open(filename_);
    if (auto err = errorHandler_.tryOpenFile(file_))
        throw std::runtime_error("File opening failed");
}

void Parser::parseTableValues() {
    std::string line;
    if (!std::getline(file_, line))
        throw std::invalid_argument("Missing table count line");

    metadata_.tableCount = convToType<size_t>(line);
    buffer_.push_back(std::move(line));
    if (metadata_.tableCount == 0)
        throw std::invalid_argument("Table count must be positive");
}

void Parser::parseWorkingTime() {
    std::string line;
    if (!std::getline(file_, line))
        throw std::invalid_argument("Missing working time line");
    buffer_.push_back(line);
    if (auto err = errorHandler_.workingTimeChecker(line))
        throw std::invalid_argument("Invalid working time");

    std::istringstream iss(line);
    iss >> metadata_.workingTime.first >> metadata_.workingTime.second;
}

void Parser::parseCost() {
    std::string line;
    if (!std::getline(file_, line))
        throw std::invalid_argument("Missing cost line");

    metadata_.cost = convToType<unsigned long long>(line);
    buffer_.push_back(std::move(line));
    if (metadata_.cost == 0)
        throw std::invalid_argument("Cost must be positive");
}

void Parser::parseMetaInfo() {
    parseTableValues();
    parseWorkingTime();
    parseCost();
}

void Parser::parseEvents() {
    std::string line;
    int64_t previousTime = -1;
    while (std::getline(file_, line)) {
        buffer_.push_back(line);
        if (auto err = errorHandler_.eventChecker(line))
            throw std::invalid_argument("Invalid event");

        std::istringstream iss(line);
        std::vector<std::string> tok;
        std::string t;
        while (iss >> t)
            tok.push_back(t);

        Event ev;
        ev.time = tok[0];
        if (toMinutes(ev.time) < previousTime) {
            throw std::invalid_argument("Invalid time");
        }
        previousTime = toMinutes(ev.time);
        ev.type = convToType<EventType>(tok[1]);
        ev.ClientName = tok[2];
        if (tok.size() == 4)
            ev.tableNumber = convToType<size_t>(tok[3]);

        events_.push_back(std::move(ev));
    }
}

void Parser::parseFile() {
    try {
        openFile();
        parseMetaInfo();
        parseEvents();
    } catch (const std::invalid_argument& err) {
        flushBuffer();
        /*
        @brief Uncommit if you wan't to see what's error threw
        */
        // std::cout << err.what() << "\n";
        throw;
    }
}

void Parser::flushBuffer() const {
    if (!buffer_.empty())
        std::cout << buffer_.back() << '\n';
}

int64_t toMinutes(const std::string& time) {
    return std::stoi(time.substr(0, 2)) * 60 + std::stoi(time.substr(3, 2));
}
