/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#ifndef LINTEL_PROGRAM_OPTIONS_HPP
#define LINTEL_PROGRAM_OPTIONS_HPP

#include <iostream>
#include <utility>

#include <boost/bind.hpp>
#include <boost/program_options.hpp>

#include <Lintel/AssertBoost.hpp>

/** @file
    
    Program options handling; simplest usage is:
 
    @verbatim

    using namespace std;
    // specify default seed as time() ^ getpid()
    lintel::ProgramOption<int> seed("seed", "specify the random seed", time() ^ getpid());
    lintel::ProgramOption<bool> help("help", "get help");
    lintel::ProgramOption< vector<string> > params("param", "specify string parameters");

    int main(int argc, char *argv[]) {
        vector<string> files = lintel::parseCommandLine(argc, argv, true);
	if (files.empty() || help.get()) {
	    cout << boost::format("Usage: %s [--help] [--seed=#] [--param=parameter] file...")
	        % argv[0];
            exit(0);
        }
	random_number_generator.seed(seed.get());
	if (params.used()) {
	    BOOST_FOREACH(const string &param, params.get()) {
	        handleParamter(param);
            }
        }

	...;
    }

    Slightly more advanced usage would be to specify:

    void skewHandler(const boost::program_options::variable_value &opt) {
       if (opt.empty()) {
           ... do default handling ...
       } else {
           ... do handling with opt.as<T>() ...
       }
    }

    lintel::ProgramOption<double> skew("skew", "time skew to apply", 
                                       boost::bind(&skewHandler, _1));

    Which will result in skewHandler being called when a call to
    lintel::parseCommandLine is made.

    @endverbatim
*/

namespace lintel {
    namespace detail {
	typedef boost::function<void (const boost::program_options::variable_value &)> 
	  ProgramOptionFnT;
	typedef std::pair<std::string, ProgramOptionFnT> ProgramOptionPair;

	// TODO: Use the boost support for multiple option groups rather than what we do here.
	boost::program_options::options_description &programOptionsDesc(bool is_hidden = false);
	
	std::vector<ProgramOptionPair> &programOptionsActions();

        // Helper to "pretty" print the default value in description message.
        template<typename T> std::string defaultValueString(const T &def_val) {
            return str(boost::format("[%1%]") % def_val);
        }
        // overload general helper for char's since they default to value 0
        // but cast to string. This leads to ugly line breaks/missing text in
        // default value message.
        template<> inline std::string defaultValueString<char>(const char &def_val) {
            return str(boost::format("[\'\\%03d\']") % static_cast<int>(def_val));
        }
        // Helper to pretty print default values of vector program options
        template<typename T> std::string defaultValueString(const std::vector<T> &def_vector) {
            std::string vdesc = "[";
            for(uint32_t i = 0; i < def_vector.size(); ++i) {
                vdesc += defaultValueString(def_vector[i]);
            }
            vdesc += "]";
            return vdesc;
        }
    }

    /// add string to help message, should be called before a call to
    /// parseCommandLine.  Will be printed after the 'Usage: argv[0]' bit.
    void programOptionsHelp(const std::string &to_add);

    /// print out the usage information. Uses value set by setArgvZero or by
    /// parseCommandLine(argc,argv) for program name.
    void programOptionsUsage();

    /// print out the usage information. Uses argv0 passed in as program name.
    void programOptionsUsage(const std::string &argv0);

    /// sets program name for progamOptionsUsage
    void setArgvZero(const std::string &argv0);

    /// parse command line options as a standard argc, argv pair; sets the
    /// program name (argv[0]) for ProgramOptionsUsage via setArgvZero.
    ///
    /// @return any un-parsed arguments if allow_unrecognized is true
    std::vector<std::string> parseCommandLine(int argc, char *argv[], 
					      bool allow_unrecognized = false);

    /// parse command line options as vector of strings
    ///
    /// @return any un-parsed arguments if allow_unrecognized is true
    std::vector<std::string> parseCommandLine(const std::vector<std::string> &args,
					      bool allow_unrecognized = false);

    class ProgramArguments {
    public:
	ProgramArguments() { }

	ProgramArguments(const std::string &arg) {
	    args.push_back(arg);
	}

	ProgramArguments(const boost::format &f) {
	    args.push_back(f.str());
	}

	ProgramArguments &operator <<(const std::string &arg) {
	    args.push_back(arg);
	    return *this;
	}

	ProgramArguments &operator <<(const boost::format &f) {
	    args.push_back(f.str());
	    return *this;
	}

	std::vector<std::string> args;
    };

    inline void parseCommandLine(const ProgramArguments &pa) {
	parseCommandLine(pa.args);
    }
    
    /// Generic template program option, you can use ProgramOption<
    /// vector<int> > to allow for multiple values to be specified, or
    /// just ProgramOption<int> for one value.
    template<typename T> class ProgramOption {
    public:
	/// Program option without an action function; expected to be
	/// accessed via the used() and get() functions.  If you specify
	/// a default value, or the default initialization of T is a
	/// sane default, you can skip used().  
	/// 
	/// @param name option name, will match with --name; should be a static string
	/// @param desc option description, will be printed when --help/-h is used; 
	///             should be a static string
	/// @param in_default_val default value for the option
	ProgramOption(const std::string &name, const std::string &desc, 
		      const T &in_default_val = T())
	    : default_val(in_default_val)
	{
	    init(name, desc);
	}
	
	bool used() {
	    return !saved.empty();
	}
	
	T get() {
	    if (used()) {
		return saved.as<T>();
	    } else {
		return default_val;
	    }
	}

	void set(const T &val) {
            saved = boost::program_options::variable_value(val, false);
	}

    protected:
	ProgramOption(const std::string &name, const std::string &desc,
		      const T &in_default_val, bool do_description)
	    : default_val(in_default_val)
	{
	    init(name, desc, do_description);	    
	}

    private:
	void init(const std::string &name, const std::string &desc, 
		  bool do_help=true) {
	    std::string def_desc = str(boost::format("%s (Default value %s.)")
		    % desc % detail::defaultValueString(default_val));
	    detail::programOptionsDesc(!do_help).add_options()
		(name.c_str(), boost::program_options::value<T>(), def_desc.c_str());
	    detail::ProgramOptionFnT f = boost::bind(&ProgramOption<T>::save, this, _1);
	    detail::programOptionsActions().push_back(std::make_pair(name, f));
	}

	void save(const boost::program_options::variable_value &opt) {
	    if (!opt.empty()) { // Do not overwrite saved opts with empty values.
		saved = opt;
	    }
	}
	
	boost::program_options::variable_value saved;
	const T default_val;
    };


    /// Program option which won't appear in the usage unless in debug mode
    template<typename T> class TestingOption : public ProgramOption<T> {
    public:	
#if LINTEL_ASSERT_BOOST_DEBUG
	static const bool show_description = true;
#else
	static const bool show_description = false;
#endif
	
	TestingOption(const std::string &name, const std::string &desc, 
		      const T &in_default_val = T())
	    : ProgramOption<T>(name, desc, in_default_val, show_description) { }
    };

    /// Special case for program options without values, e.g.,
    /// "--help", which doesn't carry a value like "--seed=100"
    template<> class ProgramOption<bool> {
    public:
	ProgramOption(const std::string &name, const std::string &desc) 
	    : val(false)
	{
	    init(name, desc);
	}

	bool used() {
	    return val;
	}
	
	bool get() {
	    return val;
	}
	
	void set(const bool &_val) {
            val = _val;
	}

    protected:
	ProgramOption(const std::string &name, const std::string &desc, bool do_description)
	    : val(false)
	{
	    init(name, desc, do_description);
	}
	
    private:
	void init(const std::string &name, const std::string &desc, 
		  bool do_help=true) {
	    detail::programOptionsDesc(!do_help).add_options() (name.c_str(), desc.c_str());
	    detail::ProgramOptionFnT f = boost::bind(&ProgramOption<bool>::save, this, _1);
	    detail::programOptionsActions().push_back(std::make_pair(name, f));
	}

	void save(const boost::program_options::variable_value &opt) {
	    if (!opt.empty()) {
		val = true;
	    }
	}

	bool val;
    };

    /// Hidden option which also doesn't take an argument
    template<> class TestingOption<bool> : public ProgramOption<bool> {
    public:	
	static const bool show_description = TestingOption<int>::show_description;

	TestingOption(const std::string &name, const std::string &desc)
	    : ProgramOption<bool>(name, desc, show_description) { }
    };

}

#endif
