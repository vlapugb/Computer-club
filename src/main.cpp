#include <iostream>

#include "club.h"
#include "error_handler.h"
#include "parser.h"
int main(int argc, char** argv) {
    try {
        ErrorHandler errHandler(argc, argv);

        Parser parser(errHandler, argv[1]);
        parser.parseFile();
        Club club(parser);
        club.run();
        return 0;
    } catch (...) {
        return 1;
    }
}
