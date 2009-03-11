/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/ProgramOptions.hpp>
#include <Lintel/TestUtil.hpp>
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
    //  --cmdline1 --cmdline2=3 
    lintel::ProgramOption<bool> cmdline1("cmdline1", "test");
    lintel::ProgramOption<bool> inline1("inline1", "test");
    lintel::ProgramOption<int> cmdline2("cmdline2", "test");
    lintel::ProgramOption<int> inline2("inline2", "test");

    lintel::parseCommandLine(argc, argv);
    SINVARIANT(cmdline1.get() && cmdline2.used());
    SINVARIANT(cmdline2.get() == 3);
    SINVARIANT(!inline1.get() && !inline2.used());

    // Set program option programmatically (note it was set to 3 on command line)
    cmdline2.set(77);
    SINVARIANT(cmdline2.get() == 77);

    // Process additional program options.
    vector<string> more_args;
    more_args.push_back("--inline1");
    more_args.push_back("--inline2=5");
    lintel::parseCommandLine(more_args, "internal-parse");

    // Make sure all progam options still set correctly.
    SINVARIANT(cmdline1.get() && cmdline2.used());
    SINVARIANT(inline1.get() && inline2.used());
    SINVARIANT(cmdline2.get() == 77);
    SINVARIANT(inline2.get() == 5);
    exit(0);
}

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
    }

    FATAL_ERROR("?");
}
