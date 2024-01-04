#include "logging.h"
int main() {
    using namespace Common;
    char c = 'd';
    int i = 5;
    unsigned int ui = 10;
    // unsigned long ul=20;
    float f = 1.0F;
    double d = 2.0;
    const char *str = "test c string";
    std::string s = "test std string";
    Logger loggerExample("/home/juzhuo/dev/LowLatencyCpp/logging_example.log");
    loggerExample.log(
        "Logging a char:% an int:% and an unsigned int:%\n", c, i, ui);
    loggerExample.log("Logging a float:% and a double:%", f, d);
    loggerExample.log("Logging a c string:%\n", str);
    loggerExample.log("Logging a std string:%\n", s);
    return 0;
}