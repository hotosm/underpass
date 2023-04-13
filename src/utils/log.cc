//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
//
// This file is part of Underpass.
//
//     Underpass is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Underpass is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.
//

/// \file log.hh
/// \brief This implements a simple logging system for info and debugging messages

#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif
#include "log.hh"

#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <cctype>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <cstdint>

#include <unistd.h> // for getpid

namespace clocktime {
/// Wall clock timer, returns current POSIX time in milliseconds.
DSOEXPORT std::uint64_t getTicks();

/// Returns the offset between actual clock time and UTC.
/// It relies on the system's time zone settings, so
/// cannot be regarded as reliable.
DSOEXPORT std::int32_t getTimeZoneOffset(double time);

#ifdef HAVE_FTIME
extern "C"
{
#include <sys/timeb.h>
#include <sys/types.h>
}
#endif

#if !defined(HAVE_TM_GMTOFF)
#ifdef HAVE_LONG_TIMEZONE
extern long timezone; // for tzset()/long timezone;
#endif
#endif

/// Win32 implementation for getTicks
#if defined(_WIN32) || defined(WIN32)
#include <mmsystem.h>
#include <windows.h>

std::uint64_t
clocktime::getTicks() {
    // This needs to return milliseconds. Does it?
    return timeGetTime();
}

#else // not _WIN32
#include <sys/time.h>

std::uint64_t
getTicks() {

    struct timeval tv;

    gettimeofday(&tv, nullptr);

    std::uint64_t result = static_cast<std::uint64_t>(tv.tv_sec) * 1000000L;

    // Time Unit: microsecond
    result += tv.tv_usec;

    return static_cast<std::uint64_t>(result / 1000.0);
}

#endif // not WIN32

/// Common non-boost function to return the present time offset.
/// This all seems like a terrible hack.
///
/// If the real mktime() sees isdst == 0 with a DST date, it sets
/// t_isdst and modifies the hour fields, but we need to set the
/// specified hour in the localtime in force at that time.
///
/// To do this we set tm_isdst to the correct value for that moment in time
/// by doing an initial conversion of the time to find out is_dst for that
/// moment without DST, then do the real conversion.
/// This may still get things wrong around the hour when the clocks go back
///
/// It also gets things wrong for very high or low time values, when the
/// localtime implementation fills the gmtoff element with 53 minutes (on
/// at least one machine, anyway).
std::int32_t
getTimeZoneOffset(double time) {

    time_t tt = static_cast<time_t>(time / 1000.0);

    struct tm tm;

#ifdef HAVE_LOCALTIME_R

    // If the requested time exceeds the limits we return 0; otherwise we'll
    // be using uninitialized values
    if (!localtime_r(&tt, &tm)) {
        return 0;
    }
#else
    struct tm *tmp = NULL;
    tmp = localtime(&tt);
    if (!tmp)
        return 0; // We failed.
    memcpy(&tm, tmp, sizeof(struct tm));
#endif

    struct tm tm2 = tm;
    tm2.tm_isdst = 0;

    time_t ttmp = 0;

    ttmp = mktime(&tm2);

#ifdef HAVE_LOCALTIME_R
    // find out whether DST is in force
    if (!localtime_r(&ttmp, &tm2)) {
        return 0;
    }
#else
    struct tm *tmp2 = NULL;
    tmp2 = localtime(&ttmp);
    if (!tmp2)
        return 0; // We failed.
    memcpy(&tm2, tmp2, sizeof(struct tm));
#endif

    // If mktime or localtime fail, tm2.tm_isdst should be unchanged,
    // so 0. That's why we don't make any checks on their success.

    tm.tm_isdst = tm2.tm_isdst;

#ifdef HAVE_TM_GMTOFF

    int offset;

    // tm_gmtoff is in seconds east of GMT; convert to minutes.
    offset = tm.tm_gmtoff / 60;
    // gnash::log_debug("Using tm.tm_gmtoff. Offset is %d", offset);
    return offset;

#else

    // Find the geographical system timezone offset and add an hour if
    // DST applies to the date.
    // To get it really right I guess we should call both gmtime()
    // and localtime() and look at the difference.
    //
    // The range of standard time is GMT-11 to GMT+14.
    // The most extreme with DST is Chatham Island GMT+12:45 +1DST

    int offset;

#if defined(HAVE_TZSET) && defined(HAVE_LONG_TIMEZONE)

    tzset();
    // timezone is seconds west of GMT
    offset = -timezone / 60;
    // gnash::log_debug("Using tzset. Offset is %d", offset);

#elif !defined(WIN32) && defined(HAVE_GETTIMEOFDAY)

    // gettimeofday(3):
    // "The use of the timezone structure is obsolete; the tz argument
    // should normally be specified as NULL. The tz_dsttime field has
    // never been used under Linux; it has not been and will not be
    // supported by libc or glibc."
    //
    // In practice this appears to return the present time offset including dst,
    // so adding the dst of the time specified (we do this a couple of lines
    // down) gives the correct result when it's not presently dst, the wrong
    // one when it is.
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    offset = -tz.tz_minuteswest;
    // gnash::log_debug("Using gettimeofday. Offset is %d", offset);

#elif defined(HAVE_FTIME)
    // ftime(3): "These days the contents of the timezone and dstflag
    // fields are undefined."
    // In practice, timezone is -120 in Italy when it should be -60.
    // The problem here as for gettimeofday: the offset also includes dst.
    struct timeb tb;

    ftime(&tb);

    // tb.timezone is number of minutes west of GMT
    offset = -tb.timezone;

#else

    offset = 0; // No idea.

#endif

    // Adjust by one hour if DST was in force at that time.
    //
    // According to http://www.timeanddate.com/time/, the only place that
    // uses DST != +1 hour is Lord Howe Island with half an hour. Tough.

    if (tm.tm_isdst == 0) {
        // DST exists and is not in effect
    } else if (tm.tm_isdst > 0) {
        // DST exists and was in effect
        offset += 60;
    } else {
        // tm_isdst is negative: cannot get TZ info.
        // Convert and print in UTC instead.
        LOG_ONCE(
            logger::log_error("Cannot get requested timezone information"););
        offset = 0;
    }

    return offset;

#endif // No gmoff
}

} // namespace clocktime

namespace logger {

LogFile &
LogFile::getDefaultInstance() {
    static LogFile o;
    return o;
}

namespace {

LogFile &dbglogfile = LogFile::getDefaultInstance();

struct Timestamp {
    std::uint64_t startTicks;
    std::map<std::thread::id, int> threadMap;
    Timestamp() : startTicks(clocktime::getTicks()) {}
};

std::ostream &
operator<<(std::ostream &o, Timestamp &t) {
    std::thread::id tid = std::this_thread::get_id();
    int &htid = t.threadMap[tid];
    if (!htid) {
        htid = t.threadMap.size();
        // TODO: notify actual thread id for index
    }

    std::uint64_t diff = clocktime::getTicks() - t.startTicks;
    // should we split in seconds/ms ?
    o << "[" << getpid() << ":" << htid << "] " << diff;

    return o;
}

Timestamp timestamp;
} // namespace

// boost format functions to process the objects
// created by our hundreds of templates

void
processLog_trace(const boost::format &fmt) {
    dbglogfile.log("TRACE", fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "Underpass", fmt.str().c_str());
#endif
}

void
processLog_debug(const boost::format &fmt) {
    if (dbglogfile.getVerbosity() < LogFile::LOG_DEBUG)
        return;
    dbglogfile.log("DEBUG", fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "Underpass", fmt.str().c_str());
#endif
}

void
processLog_info(const boost::format &fmt) {
    dbglogfile.log(fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_VERBOSE, "Underpass", fmt.str().c_str());
#endif
}

void
processLog_network(const boost::format &fmt) {
    dbglogfile.log("NETWORK", fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "Underpass", fmt.str().c_str());
#endif
}

void
processLog_error(const boost::format &fmt) {
    dbglogfile.log("ERROR", fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_ERROR, "Underpass", fmt.str().c_str());
#endif
}

void
processLog_unimpl(const boost::format &fmt) {
    dbglogfile.log("UNIMPLEMENTED", fmt.str());
    // Print messages to the Android log, where they can be retrieved with
    // logcat.
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "Underpass", fmt.str().c_str());
#endif
}

void
LogFile::log(const std::string &msg) {
    std::lock_guard<std::mutex> lock(_ioMutex);

    if (!_verbose)
        return; // nothing to do if not verbose

    if (openLogIfNeeded()) {
        if (_stamp) {
            _outstream << timestamp << ": " << msg << "\n";
        } else {
            _outstream << msg << "\n";
        }
        _outstream.flush();
    } else {
        // log to stdout
        if (_stamp) {
            std::cout << timestamp << " " << msg << std::endl;
        } else {
            std::cout << msg << std::endl;
        }
    }

    if (_listener) {
        (*_listener)(msg);
    }
}

inline void
LogFile::log(const std::string &label, const std::string &msg) {
    log(label + ": " + msg);
}

void
LogFile::setLogFilename(const std::string &fname) {
    closeLog();
    _logFilename = fname;
}

void
LogFile::setWriteDisk(bool use) {
    if (!use)
        closeLog();
    _write = use;
}

// Default constructor
LogFile::LogFile()
    : _verbose(0), _network(false), _state(CLOSED), _stamp(true), _write(false),
      _listener(nullptr) {}

LogFile::~LogFile() {
    if (_state == OPEN)
        closeLog();
}

bool
LogFile::openLogIfNeeded() {
    if (_state != CLOSED)
        return true;
    if (!_write)
        return false;

    if (_logFilename.empty())
        _logFilename = DEFAULT_LOGFILE;

    // TODO: expand ~ to getenv("HOME") !!

    return openLog(_logFilename);
}

bool
LogFile::openLog(const std::string &filespec) {

    // NOTE:
    // don't need to lock the mutex here, as this method
    // is intended to be called only by openLogIfNeeded,
    // which in turn is called by operator<< which is called
    // by the public log_xxx functions that log themselves

    if (_state != CLOSED) {
        std::cout << "Closing previously opened stream" << std::endl;
        _outstream.close();
        _state = CLOSED;
    }

    if (boost::filesystem::exists(filespec)) {
        boost::filesystem::resize_file(filespec, 0);
    }
    // FIXME: Append, don't truncate, the log file
    _outstream.open(filespec, std::ios::app | std::ios::out); // ios::out
    if (_outstream.fail()) {
        // Can't use log_error here...
        std::cout << "ERROR: can't open debug log file " << filespec
                  << " for appending." << std::endl;
        return false;
    }

    _filespec = filespec;
    _state = OPEN;

    return true;
}

bool
LogFile::closeLog() {
    std::lock_guard<std::mutex> lock(_ioMutex);

    if (_state == OPEN) {
        _outstream.flush();
        _outstream.close();
    }
    _state = CLOSED;

    return true;
}

bool
LogFile::removeLog() {
    if (_state == OPEN) {
        _outstream.close();
    }

    // Ignore the error, we don't care
    unlink(_filespec.c_str());
    _filespec.clear();

    return true;
}

} // namespace logger

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
