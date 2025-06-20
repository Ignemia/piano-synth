#include <gtest/gtest.h>
#include "../app/cli_arguments.h"

using namespace PianoSynth::App;

TEST(CliArgumentsTest, ParseFlags) {
    const char* argv[] = {"prog", "--record", "--config"};
    CliOptions opts = parseCliArguments(3, const_cast<char**>(argv));
    EXPECT_TRUE(opts.record);
    EXPECT_TRUE(opts.show_config);
}

TEST(CliArgumentsTest, NoFlags) {
    const char* argv[] = {"prog"};
    CliOptions opts = parseCliArguments(1, const_cast<char**>(argv));
    EXPECT_FALSE(opts.record);
    EXPECT_FALSE(opts.show_config);
}

