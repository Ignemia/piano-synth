#include <gtest/gtest.h>
#include "../core/utils/note_settings_cli.h"
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace PianoSynth::Utils;

TEST(NoteSettingsCli, PrintsLoadedNotes) {
    std::string file = "test_params.json";
    std::ofstream out(file);
    out << R"({"60": {"inharm":0.1, "partials":2, "decay":1, "hammer":0.2, "volume":0.5, "tension":1.0}})";
    out.close();

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    showNoteSettings(file);
    std::cout.rdbuf(old);

    std::string output = buffer.str();
    EXPECT_NE(output.find("60"), std::string::npos);

    std::filesystem::remove(file);
}

