#include "club.h"
#include <iomanip>
Club::Club(Parser& parser)
    : parser_(parser), events_(parser_.getEvents()), metadata_(parser_.getMetaInfo()) {}

void Club::messagesProcesser() {

    auto printErrorMessage = [&](const std::string& errorText, const std::string& time) {
        std::cout << time << " 13 " << errorText << "\n";
    };

    unwrapMetaInfo();
    std::cout << metadata_.workingTime.first << "\n";

    while (!events_.empty()) {
        Event& currentEvent = events_.front();
        std::string eventTimeStr = currentEvent.time;
        EventType type = currentEvent.type;
        std::string clientUserName = currentEvent.ClientName;
        size_t tableNumber = currentEvent.tableNumber.value_or(0);
        std::chrono::minutes eventTime{toMinutes(eventTimeStr)};

        if (currentEvent.tableNumber.has_value()) {
            std::cout << currentEvent.time << " " << static_cast<int>(type) << " " << clientUserName
                      << " " << tableNumber << "\n";
        } else {
            std::cout << currentEvent.time << " " << static_cast<int>(type) << " " << clientUserName
                      << "\n";
        }

        switch (type) {

        case EventType::IN: {
            if (eventTime < openTime || eventTime >= endTime) {
                printErrorMessage("NotOpenYet", eventTimeStr);
                events_.pop_front();
                continue;
            }

            if (clients_.find(clientUserName) != clients_.end()) {
                printErrorMessage("YouShallNotPass", currentEvent.time);
            } else {
                clients_[clientUserName] = clientInfo{};
            }
            break;
        }
        case EventType::SEATING: {
            if (clients_.find(clientUserName) == clients_.end()) {
                printErrorMessage("ClientUnknown", eventTimeStr);
                events_.pop_front();
                continue;
            }
            auto& client = clients_.at(clientUserName);
            if (tableNumber < 1 || tableNumber > tables_.size()) {
                printErrorMessage("PlaceIsBusy", eventTimeStr);
                events_.pop_front();
                continue;
            }
            if (tables_[tableNumber - 1].has_value()) {
                printErrorMessage("PlaceIsBusy", eventTimeStr);
                events_.pop_front();
                continue;
            }
            size_t freedTable = 0;
            if (client.seated) {
                freedTable = client.table;
                placedTableTime_[client.table - 1] += eventTime - client.seatTime;
                tables_[client.table - 1] = std::nullopt;
                auto minutes = (eventTime - client.seatTime).count();
                summaryCostTables[client.table - 1] += ((minutes + 59) / 60) * cost;
            }

            client.seated = true;
            client.inQueue = false;
            client.table = tableNumber;
            client.seatTime = eventTime;
            tables_[tableNumber - 1] = clientUserName;

            if (freedTable > 0 && !waiting_.empty()) {
                std::string nextClient = waiting_.front();
                waiting_.pop_front();

                auto& infoNextClient = clients_[nextClient];
                infoNextClient.inQueue = false;
                infoNextClient.seated = true;
                infoNextClient.table = freedTable;
                infoNextClient.seatTime = eventTime;
                tables_[freedTable - 1] = nextClient;

                std::cout << eventTimeStr << " 12 " << nextClient << " " << freedTable << "\n";
            }
            break;
        }
        case EventType::WAITING: {

            if (clients_.find(clientUserName) == clients_.end()) {
                printErrorMessage("ClientUnknown", eventTimeStr);
                events_.pop_front();
                continue;
            }
            if (freeTableExists()) {
                printErrorMessage("ICanWaitNoLonger!", eventTimeStr);
                events_.pop_front();
                continue;
            }
            if (waiting_.size() > tables_.size()) {
                std::cout << eventTimeStr << " " << static_cast<int>(EventType::OUTGOING_OUT) << " "
                          << clientUserName << "\n";
                clients_.erase(clientUserName);
            } else {
                waiting_.push_back(clientUserName);
                clients_[clientUserName].inQueue = true;
            }
            break;
        }
        case EventType::OUT: {
            if (clients_.find(clientUserName) == clients_.end()) {
                printErrorMessage("ClientUnknown", eventTimeStr);
                events_.pop_front();
                continue;
            }
            auto& client = clients_[clientUserName];
            size_t freedTable = 0;

            if (client.seated) {
                freedTable = client.table;
                placedTableTime_[freedTable - 1] += eventTime - client.seatTime;
                tables_[freedTable - 1] = std::nullopt;
                auto minutes = (eventTime - client.seatTime).count();
                summaryCostTables[freedTable - 1] += ((minutes + 59) / 60) * cost;
            }

            clients_.erase(clientUserName);

            if (freedTable > 0 && !waiting_.empty()) {
                std::string nextClient = waiting_.front();
                waiting_.pop_front();

                clients_[nextClient].inQueue = false;
                clients_[nextClient].seated = true;
                clients_[nextClient].table = freedTable;
                clients_[nextClient].seatTime = eventTime;
                tables_[freedTable - 1] = nextClient;

                std::cout << eventTimeStr << " " << static_cast<int>(EventType::OUTGOING_SEATING)
                          << " " << nextClient << " " << freedTable << "\n";
            }
            break;
        }
        default:
            break;
        }
        events_.pop_front();
    }
    std::vector<std::string> rest;
    for (auto& [clientName, info] : clients_) {
        if (info.seated) {
            auto minutes = (endTime - info.seatTime).count();
            summaryCostTables[info.table - 1] += ((minutes + 59) / 60) * cost;
            placedTableTime_[info.table - 1] += endTime - info.seatTime;
        }
        rest.push_back(clientName);
    }
    std::sort(rest.begin(), rest.end());
    for (const auto& name : rest) {
        std::cout << metadata_.workingTime.second << " "
                  << static_cast<int>(EventType::OUTGOING_OUT) << " " << name << "\n";
    }
    std::cout << metadata_.workingTime.second << "\n";
    countSummary();
}

void Club::unwrapMetaInfo() {
    openTime = std::chrono::minutes{toMinutes(metadata_.workingTime.first)};
    endTime = std::chrono::minutes{toMinutes(metadata_.workingTime.second)};
    cost = metadata_.cost;
    tableCount = metadata_.tableCount;
    tables_.assign(tableCount, std::nullopt);
    placedTableTime_.assign(tableCount, std::chrono::minutes{0});
    summaryCostTables.assign(tableCount, 0ULL);
}

bool Club::freeTableExists() const {
    return std::any_of(tables_.begin(), tables_.end(), [](auto& c) { return !c.has_value(); });
}

void Club::countSummary() {
    for (size_t i = 0; i < tableCount; ++i) {
        std::cout << i + 1 << " " << summaryCostTables[i] << " " << std::setfill('0')
                  << std::setw(2) << placedTableTime_[i].count() / 60 << ":" << std::setw(2)
                  << placedTableTime_[i].count() % 60 << "\n";
    }
}

void Club::run() { messagesProcesser(); }
