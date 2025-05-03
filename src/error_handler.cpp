#include "error_handler.h"
#include "parser.h"
#include <vector>
#include <iterator>

static const std::regex hhmm(R"(^(?:[01]\d|2[0-3]):[0-5]\d$)");
static const std::regex client(R"(^[a-z0-9_-]+$)");
static const std::regex positive(R"(^[1-9][0-9]*$)");



ErrorHandler::ErrorHandler(int argc, char *argv[])
{
    if (argc != 2)
        throw std::invalid_argument("Exactly one argument (path to file) is required.");
}

std::optional<std::error_code> ErrorHandler::tryOpenFile(const std::ifstream &file) const
{
    if (!file.is_open())
        return std::make_error_code(std::errc::no_such_file_or_directory);
    return std::nullopt;
}

std::optional<std::error_code> ErrorHandler::workingTimeChecker(const std::string &line) const
{
    std::istringstream iss(line);
    std::string start, finish;
    if (!(iss >> start >> finish))
        return std::make_error_code(std::errc::invalid_argument);

    if (!std::regex_match(start, hhmm) || !std::regex_match(finish, hhmm))
        return std::make_error_code(std::errc::invalid_argument);

    int s = toMinutes(start);
    int f = toMinutes(finish);

    if (s >= f || f > 24 * 60)
        return std::make_error_code(std::errc::invalid_argument);

    return std::nullopt;
}

std::optional<std::error_code> ErrorHandler::eventChecker(const std::string &eventLine) const
{
    std::istringstream iss(eventLine);
    std::vector<std::string> tok{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};

    if (tok.size() < 3)
        return std::make_error_code(std::errc::invalid_argument);

    if (!std::regex_match(tok[0], hhmm))
        return std::make_error_code(std::errc::invalid_argument);

    if (!std::regex_match(tok[1], positive))
        return std::make_error_code(std::errc::invalid_argument);

    int id = std::stoi(tok[1]);
    if (id < 1 || id > 4)
        return std::make_error_code(std::errc::invalid_argument);

    if (!std::regex_match(tok[2], client))
        return std::make_error_code(std::errc::invalid_argument);

    if (id == 2 && tok.size() != 4)
        return std::make_error_code(std::errc::invalid_argument);
    if (id != 2 && tok.size() != 3)
        return std::make_error_code(std::errc::invalid_argument);

    if (id == 2 && !std::regex_match(tok[3], positive))
        return std::make_error_code(std::errc::invalid_argument);

    return std::nullopt;
}