/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <inttypes.h>
#include <Lintel/ProgramOptions.hpp>
#include <Lintel/TestUtil.hpp>
#include <fstream>
using namespace std;

void first(int argc, char *argv[]) {
    // --mode=first --test --multi=1 --multi=2
    lintel::ProgramOption<string> mode("mode", "...");
    lintel::ProgramOption<bool> test("test", "...");
    lintel::ProgramOption< vector<int> > multi("multi", "...");
    lintel::ProgramOption<bool> unused("unused", "...");
    lintel::ProgramOption<char> unused2("unused2", "...");

    lintel::parseCommandLine(argc, argv);

    SINVARIANT(mode.used() && test.used() && multi.used() && !unused.used() && !unused2.used());
    SINVARIANT(mode.get() == "first" && test.get() == true && multi.get().size() == 2 &&
	       multi.get()[0] == 1 && multi.get()[1] == 2);
    exit(0);
}

void second(int argc, char *argv[]) {
    // --sample=a --unknown foo

    lintel::ProgramOption<char> sample("sample", "...");
    vector<string> unparsed = lintel::parseCommandLine(argc, argv, true);

    SINVARIANT(sample.used() && sample.get() == 'a');
    SINVARIANT(unparsed.size() == 2 && unparsed[0] == "--unknown" && unparsed[1] == "foo");
    exit(0);
}

void third(int argc, char *argv[]) {
    // --help

    lintel::programOptionsHelp("test-extra-help-bit");
    lintel::ProgramOption<char> arg1("arg1", "argument 1 description");
    lintel::ProgramOption<bool> arg2("arg2", "argument 2 description");
    lintel::parseCommandLine(argc, argv);
    FATAL_ERROR("should not have gotten here");
}

void fifth(int argc, char *argv[]) {
    //  --cmdline1 --cmdline2=3 --cmdline3=8 --cmdline3=8 --cmdline3=8 --cmdline3=8
    lintel::ProgramOption<bool> cmdline1("cmdline1", "test");
    lintel::ProgramOption<bool> inline1("inline1", "test");
    lintel::ProgramOption<int> cmdline2("cmdline2", "test");
    lintel::ProgramOption<int> inline2("inline2", "test", 4);
    lintel::ProgramOption< vector<int> > cmdline3("cmdline3", "test");
    lintel::ProgramOption< vector<int> > inline3("inline3", "test", vector<int>(3,7));

    lintel::parseCommandLine(argc, argv);
    SINVARIANT(cmdline1.get() && cmdline2.used() && cmdline3.used());
    SINVARIANT(cmdline2.get() == 3);
    SINVARIANT(cmdline3.get() == vector<int>(4,8));
    SINVARIANT(!inline1.get() && !inline2.used() && !inline3.used());
    SINVARIANT(inline2.get() == 4); // Test default value.
    SINVARIANT(inline3.get() == vector<int>(3,7)); // Test default value.

    // Set program option programmatically (note it was set to 3 on command line)
    cmdline2.set(77);
    SINVARIANT(cmdline2.get() == 77);

    lintel::ProgramArguments pargs;
    
    pargs << "--inline1" << "--inline2=5";

    // Process additional program options.
    lintel::parseCommandLine(pargs << "--inline3=9" << "--inline3=9");

    // Make sure the "more_args" program options are set correctly, and confirm
    // previously set program options are still the same.
    SINVARIANT(cmdline1.get() && cmdline2.used());
    SINVARIANT(inline1.get() && inline2.used());
    SINVARIANT(cmdline2.get() == 77);
    SINVARIANT(cmdline3.get() == vector<int>(4,8));
    SINVARIANT(inline2.get() == 5);
    SINVARIANT(inline3.get() == vector<int>(2,9)); 
    exit(0);
}

void sixth(int argc, char *argv[]) {
    // --po-1 --test-opt-1 --po-2=5 --test-opt-4=5 
    lintel::ProgramOption<bool> po_1("po-1", "Thing");
    lintel::TestingOption<bool> test_opt_1("test-opt-1", "SHOULD NOT SEE");
    lintel::TestingOption<bool> test_opt_2("test-opt-2", "SHOULD NOT SEE");
    
    lintel::ProgramOption<int> po_2("po-2", "Another thing", 4);
    lintel::TestingOption<int> test_opt_3("test-opt-3", "SHOULD NOT SEE", 5);
    lintel::TestingOption<int> test_opt_4("test-opt-4", "SHOULD NOT SEE", 4);
    
    lintel::parseCommandLine(argc, argv);

    SINVARIANT(po_1.get() && test_opt_1.get() && (!test_opt_2.get()));
    SINVARIANT(po_2.get()==5);
    SINVARIANT(test_opt_3.get()==5);
    SINVARIANT(test_opt_4.get()==5);
    exit(0);
}

void read_config_file_test(int argc, char *argv[]) {
    lintel::ProgramOption<string> po_mode("mode", "...");
    lintel::ProgramOption<string> po_file_path("file-path", "Configuration file path");

    lintel::parseCommandLine(argc, argv);

    lintel::ProgramOption<int> po_int("po-int", "Read integer", 1);
    lintel::ProgramOption<std::string> po_str("po-str", "Read string", "test");
    lintel::ProgramOption<int> po_s1_int("s1.po-int", "Read section1", 2);
    lintel::ProgramOption<std::string> po_s2_str("s2.po-str", "Read section2", "test");

    SINVARIANT(po_int.get() == 1);
    SINVARIANT(po_str.get() == "test");
    SINVARIANT(po_s1_int.get() == 2);
    SINVARIANT(po_s2_str.get() == "test");

    if (po_mode.get() == "stream_read") {
        std::fstream file(po_file_path.get().c_str(), fstream::in);
	lintel::parseConfigFile(file);
    } else if (po_mode.get() == "file_read") {
	lintel::parseConfigFile(po_file_path.get());
    } else {
	FATAL_ERROR("should not have gotten here");
    }	

    SINVARIANT(po_int.get() == 10);
    INVARIANT(po_str.get() == "test string", boost::format("'%s'") % po_str.get());
    SINVARIANT(po_s1_int.get() == 20);
    INVARIANT(po_s2_str.get() == "test section", boost::format("'%s'") % po_s2_str.get());
    exit(0);
}

void helpWidthTest(int argc, char *argv[]) {
    lintel::ProgramOption<string> po_program_option_help_width_test
        ("program-option-help-width-test", "This is a program option test. This program option is"
         " used to verify, while displaying program option help, width of program option "
         "description is appropriately set according to LINTEL_PO_HELP_WIDTH environment variable."
         " If LINTEL_PO_HELP_WIDTH environment variable is not set then default value of 80 is "
         "used as program option help width. Description of this program option is big, "
         "so that program option help description width can be verfied for larger range.");
    lintel::parseCommandLine(argc, argv, true);
    FATAL_ERROR("This test is to verify help description for program option.");
}

// TODO: with options like --foo bar -- baz; if you call parseCommandLine,
// you will not get the baz option back in the array.

int main(int argc, char *argv[]) {
    SINVARIANT(getenv("LINTEL_PO_TEST") != NULL);
    
    string mode(getenv("LINTEL_PO_TEST"));
    if (mode == "first") {
	first(argc, argv);
    } else if (mode == "second") {
	second(argc, argv);
    } else if (mode == "third") {
	third(argc, argv);
    } else if (mode == "fourth") {
	TEST_INVARIANTMSG(first(argc, argv), 
			  "Unexpected option '-x'; try ./program_options -h for help");
	exit(0);
    } else if (mode == "fifth") {
        fifth(argc, argv);
    } else if (mode == "sixth") {
	sixth(argc, argv);
    } else if (mode == "stream_read" || mode == "file_read") {
	read_config_file_test(argc, argv);
    } else if (mode == "help-width-test") {
        helpWidthTest(argc, argv);
    }
    // TODO: add test for duplicate program option, should probably abort
    // if the user tries to do that.

    FATAL_ERROR("?");
}
