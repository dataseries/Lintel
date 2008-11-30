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
    // time() ^ getpid() is the default seed.
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

	boost::program_options::options_description &programOptionsDesc();
	std::vector<ProgramOptionPair> &programOptionsActions();
    }

    /// parse command line options as a standard arc, argv pair;
    ///
    /// @return any un-parsed arguments if allow_unrecognized is true
    std::vector<std::string> parseCommandLine(int argc, char *argv[], 
					      bool allow_unrecognized = false);

    /// Generic template program option, you can use ProgramOption<
    /// vector<int> > to allow for multiple values to be specified, or
    /// just ProgramOption<int> for one value.
    template<typename T> class ProgramOption {
    public:
	/// Program option without an action function; expected to be
	/// used via that used() and get() functions.  If you specify
	/// a default value, or the default initialization of T is a
	/// sane default, you can skip used().  
	/// 
	/// @param name option name, will match with --name; should be a static string
	/// @param desc option description, will be printed when --help/-h is used; 
	///             should be a static string
	/// @param default_val default value for the option
	ProgramOption(const std::string &name, const std::string &desc, 
		      const T &_default_val = T())
	    : default_val(_default_val)
	{
	    init(name, desc, boost::bind(&ProgramOption<T>::save, this, _1));
	}
	
	/// Traditional ProgramOptionsFn will look like:
        ProgramOption(const std::string &name, const std::string &desc, 
		      detail::ProgramOptionFnT f)
	    : default_val(T()) 
	{
	    INVARIANT(!!f, boost::format("option %s needs a function") % name);
	    init(name, desc, f);
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
    private:
	void init(const std::string &name, const std::string &desc, 
		  detail::ProgramOptionFnT f) {
	    detail::programOptionsDesc().add_options()
		(name.c_str(), boost::program_options::value<T>(), desc.c_str());
	    detail::programOptionsActions().push_back(std::make_pair(name, f));
	}

	void save(const boost::program_options::variable_value &opt) {
	    saved = opt;
	}
	
	boost::program_options::variable_value saved;
	const T default_val;
    };

    /// Special case for program options without values, e.g.,
    /// "--help", which doesn't carry a value like "--seed=100"
    // TODO: allow for --no-<opt> also.
    template<> class ProgramOption<bool> {
    public:
	ProgramOption(const std::string &name, const std::string &desc) 
	    : set(false)
	{
	    init(name, desc, boost::bind(&ProgramOption<bool>::save, this, _1));
	}

	ProgramOption(const std::string &name, const std::string &desc, detail::ProgramOptionFnT f) 
	    : set(false)
	{
	    INVARIANT(!!f, boost::format("option %s needs a function") % name);
	    init(name, desc, f);
	}
	    
	bool used() {
	    return set;
	}
	
	bool get() {
	    return set;
	}
	
    private:
	void init(const std::string &name, const std::string &desc, detail::ProgramOptionFnT f) {
	    detail::programOptionsDesc().add_options() (name.c_str(), desc.c_str());
	    detail::programOptionsActions().push_back(std::make_pair(name, f));
	}

	void save(const boost::program_options::variable_value &opt) {
	    if (!opt.empty()) {
		set = true;
	    }
	}
	
	bool set;
    };
}

#endif
