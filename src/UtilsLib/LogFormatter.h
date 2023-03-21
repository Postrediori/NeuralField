#pragma once
#include <plog/Record.h>
#include <plog/Util.h>

namespace plog {
    class LogFormatter {
    public:
        static util::nstring header();

        static util::nstring format(const Record& record);
    };
}
