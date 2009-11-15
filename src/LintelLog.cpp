/* -*-C++-*- */
/*
   (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <iostream>
#include <queue>

#ifdef SYS_POSIX
#include <sys/time.h>
#endif

#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#include <Lintel/LintelLog.hpp>
#include <Lintel/SimpleMutex.hpp>
#include <Lintel/StringUtil.hpp>

using namespace std;
using boost::format;

LintelLog::instance_data *LintelLog::instance;

static unsigned slow_woulddebug_calls;

bool LintelLog::wouldDebug(const string &category, unsigned level)
{
    ++slow_woulddebug_calls;
    maybeInitInstance();
    uint32_t id = categoryToId(category);
    return instance->debug_levels[id] >= level;
}

void LintelLog::reserveCategories(unsigned max_categories)
{
    maybeInitInstance();
    instance->mutex.lock();
    SINVARIANT(max_categories >= instance->debug_levels.size());
    instance->debug_levels.reserve(max_categories);
    instance->mutex.unlock();
}

void LintelLog::parseEnv()
{
    char *env = getenv("LINTEL_LOG_DEBUG");
    if (env == NULL) {
	return;
    }
    parseDebugString(env);
}

void LintelLog::parseDebugString(const string &debug_options)
{
    vector<string> parts;
    split(debug_options, ",", parts);
    for(vector<string>::iterator i = parts.begin(); 
	i != parts.end(); ++i) {
	vector<string> subparts;
	split(*i,"=",subparts);
	INVARIANT(subparts.size() == 1 || subparts.size() == 2, "bad");
	if (subparts.size() == 1) {
	    setDebugLevel(*i, 1);
	} else {
	    setDebugLevel(subparts[0], stringToInteger<uint32_t>(subparts[1]));
	}
    }
}

void LintelLog::setKnownCategories(const vector<string> &categories)
{
    for(vector<string>::const_iterator i = categories.begin(); 
	i != categories.end(); ++i) {
	categoryToId(*i);
    }
}

void LintelLog::setDebugLevel(const string &category, uint8_t level)
{
    maybeInitInstance();

    // Assuming that readers will either see the old or the new value,
    // and hence we won't have to lock the read path.
    instance->mutex.lock();
    uint32_t id = lockedCategoryToId(category);
    SINVARIANT(instance->debug_levels.size() > id);
    instance->debug_levels[id] = level;
    instance->mutex.unlock();
    maybeCategoryLimitWarning(id);
}

void LintelLog::debugMessagesInitial()
{
    static Category help("help");
    if (wouldDebug(help)) {
	vector<string> debug_names;

	instance->mutex.lock();
	for(HashMap<string, uint32_t>::iterator i 
		= instance->category2id.begin(); 
	    i != instance->category2id.end(); ++i) {
	    debug_names.push_back(i->first);
	}
	instance->mutex.unlock();
	sort(debug_names.begin(), debug_names.end());

	LintelLogDebugLevelVariable
	    (help, 1, format("known debugging options: %s")
	     % join(", ", debug_names));
    }
}

void LintelLog::debugMessagesFinal()
{
    LintelLogDebug("LintelLog::stats", 
		   format("%d calls to slow wouldDebug path")
		   % slow_woulddebug_calls);
}

void LintelLog::addAppender(appender_fn fn)
{
    maybeInitInstance();
    instance->mutex.lock();
    instance->appenders.push_back(fn);
    instance->mutex.unlock();
}

static string str_empty("");
static string str_DEBUG("DEBUG");
static string str_INFO("INFO");
static string str_WARN("WARN");
static string str_ERROR("ERROR");
static string str_sep(": ");

const string &LintelLog::logTypeToString(const LogType logtype)
{
    switch(logtype)
	{
	case Report: return str_empty;
	case Debug: return str_DEBUG;
	case Info: return str_INFO;
	case Warn: return str_WARN;
	case Error: return str_ERROR;
	default: FATAL_ERROR("unknown log type");
	}
}

void LintelLog::consoleAppender(const std::string &msg, LogType logtype)
{
    switch(logtype) 
	{
	case Report: cout << msg << "\n"; break;
	case Debug: case Info:
	    cout << logTypeToString(logtype) << str_sep << msg << "\n";
	    break;
	case Warn: case Error:
	    cout.flush();
	    // use endl to force flushing.
	    cerr << logTypeToString(logtype) << str_sep << msg << endl;
	    break;
	default: FATAL_ERROR("unknown log type");
	}
}

void LintelLog::consoleTimeAppender(const std::string &msg, LogType logtype)
{
    struct timeval t;
    CHECKED(gettimeofday(&t,NULL)==0, "how did gettimeofday fail?");
    
    switch(logtype) 
	{
	case Report: cout << msg << "\n"; break;
	case Debug: case Info:
	    cout << format("%s @%d.%d: %s\n") % logTypeToString(logtype) 
		% t.tv_sec % t.tv_usec % msg;
	    break;
	case Warn: case Error:
	    cout.flush();
	    cerr << format("%s @%d.%d: %s") % logTypeToString(logtype) 
		% t.tv_sec % t.tv_usec % msg << endl;
	    break;
	default: FATAL_ERROR("unknown log type");
	}
}

void LintelLog::log(const string &msg, const LogType log_type)
{
    maybeInitInstance();
    // TODO: the next lock statement can deadlock because we were called
    // recursively.  Unfortunately, one (large) case where this can happen
    // is people trying to use logging to output error from segmentation
    // faults that happen while logging.  The obvious thing to do is to use
    // timedLock, but that doesn't work becuase the instance mutex is a 
    // simple mutex so that it works without being linked with libpthread.
    // Not clear what the right solution is.
    instance->mutex.lock();
    if (instance->appenders.empty()) {
	consoleAppender(msg, log_type);
    } else {
	for(vector<appender_fn>::iterator i = instance->appenders.begin();
	    i != instance->appenders.end(); ++i) {
	    (*i)(msg, log_type);
	}
    }
    instance->mutex.unlock();
}

uint32_t LintelLog::categoryToId(const string &category)
{
    maybeInitInstance();
    instance->mutex.lock();
    uint32_t ret = lockedCategoryToId(category);
    instance->mutex.unlock();

    maybeCategoryLimitWarning(ret);
    return ret;
}

void LintelLog::maybeCategoryLimitWarning(uint32_t id)
{
    if (((id * 5) / 4) >= instance->debug_levels.capacity()) {
	static bool generated_warning = false;
	if (!generated_warning) {
	    generated_warning = true;
	    warn(format("close to running out of reserved categories %d/%d used; recommend calling LintelLog::reserveCategories(%d) or greater") 
		 % id % instance->debug_levels.capacity()
		 % (1.5 * instance->debug_levels.capacity()));
	}
    }
}

uint32_t LintelLog::lockedCategoryToId(const string &category)
{
    uint32_t ret;
    uint32_t *t = instance->category2id.lookup(category);
    if (t != NULL) {
	ret = *t;
    } else {
	ret = instance->debug_levels.size();
	INVARIANT(ret < instance->debug_levels.capacity(),
		  format("thread safety would be sacrificed by letting the vector resize; you need to call LintelLog::reserveCategories(%d) or greater")
		  % (ret * 1.5));
	instance->category2id[category] = ret;
	instance->debug_levels.resize(ret+1);
	SINVARIANT(instance->debug_levels[ret] == 0);
    }
    return ret;
}

void LintelLog::maybeInitInstance()
{
    static SimpleMutex init_mutex;

    // is this actually safe?  Really don't want to have to take the
    // lock when we almost never need to.
    if (instance == NULL) {
	init_mutex.lock();
	if (instance == NULL) {
	    instance_data *tmp = new instance_data();
	    tmp->mutex.lock(); // try to force the above to be 
	    tmp->mutex.unlock();
	    instance = tmp;
	}
	init_mutex.unlock();
    }
}
