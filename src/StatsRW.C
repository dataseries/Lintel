/* -*-C++-*-
*******************************************************************************
*
* File:         StatsRW.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StatsRW.C,v 1.12 2003/07/30 00:17:18 anderse Exp $
* Description:  
* Author:       Eric Anderson
* Created:      Wed Aug 22 12:52:54 2001
* Modified:     Thu Nov 21 18:09:53 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <Stats.H>
#include <StatsMaker.H>
#include <StatsSeriesGroup.H>

StatsRW::StatsRW(const std::string &name) 
{
    std::string tmp_name;

    tmp_name = name + "-read";
    read = StatsMaker::make(tmp_name);
    tmp_name = name + "-write";
    write = StatsMaker::make(tmp_name);
    tmp_name = name + "-all";
    all = StatsMaker::make(tmp_name);
    finish_init();
}

StatsRW::StatsRW(Stats *_read, Stats *_write, Stats *_all)
{
  read = _read;
  write = _write;
  all = _all;
  finish_init();
}

void 
StatsRW::finish_init()
{
    if (dynamic_cast<StatsSeriesGroup *>(read) != NULL &&
	dynamic_cast<StatsSeriesGroup *>(write) != NULL &&
	dynamic_cast<StatsSeriesGroup *>(all) != NULL) {
	allStatsSeriesGroup = true;
    } else {
	allStatsSeriesGroup = false;
    }
    autoAll = true;
}

void
StatsRW::add(const double value, modeT mode, const double timeSeq)
{
    if(allStatsSeriesGroup && read->count() == 0 && write->count() == 0) {
	dynamic_cast<StatsSeriesGroup *>(read)->setSeriesStart(timeSeq);
	dynamic_cast<StatsSeriesGroup *>(write)->setSeriesStart(timeSeq);
	dynamic_cast<StatsSeriesGroup *>(all)->setSeriesStart(timeSeq);
    }

    switch(mode) 
	{
	case Read:
	    if (read) read->addTimeSeq(value, timeSeq); 
	    break;
	case Write:
	    if (write) write->addTimeSeq(value, timeSeq);
	    break;
	case All:
	    if (autoAll) { 
		if (all) all->reset();
		autoAll = false; 
	    }
	    if (all) all->addTimeSeq(value, timeSeq);
	    break;
	default:
	    AssertFatal(("Unknown mode %d for StatsRW::add\n",mode));
	}
    if (autoAll) {
	if (all) all->addTimeSeq(value, timeSeq);
    }
}

void
StatsRW::setSequenceInfo(double interval_width, StatsSequence::mode mode)
{
    StatsSequence *s;

    s = dynamic_cast<StatsSequence *>(read);
    if (s) {
	s->setIntervalWidth(interval_width);
	s->setMergeMode(mode);
    }
    s = dynamic_cast<StatsSequence *>(write);
    if (s) {
	s->setIntervalWidth(interval_width);
	s->setMergeMode(mode);
    }

    s = dynamic_cast<StatsSequence *>(all);
    if (s) {
	s->setIntervalWidth(interval_width);
	s->setMergeMode(mode);
    }
}

void
StatsRW::printRome(int depth, std::ostream &out) const
{
  std::string spaces;
  for(int i = 0; i < depth; i++) {
    spaces += " ";
  }

  if (read->count() == 0 && write->count() == 0 &&
      all->count() > 0) { 
      // Behave as if we were just the underlying all stats object.
      all->printRome(depth,out);
  } else {
      out << spaces << "{\n";
      if (read) {
	  out << spaces << "  { read {\n";
	  read->printRome(depth+4,out);
	  out << spaces << "  }}\n";
      }

      if (write) {
	  out << spaces << "  { write {\n";
	  write->printRome(depth+4,out);
	  out << spaces << "  }}\n";
      }

      if (all) {
	  out << spaces << "  { all {\n";
	  all->printRome(depth+4,out);
	  out << spaces << "  }}\n";
      }
      out << spaces << "}\n";
  }
}


void StatsRW::printTabular(int depth, std::ostream& out) const
{  
  // Note, that for now I ignore the depth.  It's easier to feed 
  // un-indented data to gnuplot/perl; however, we may change our 
  // minds in the future.

 std::string spaces("");
 /*
  for(int i = 0; i < depth; i++) {
    spaces += " ";
  }
  */

  if (read->count() == 0 && write->count() == 0 &&
      all->count() > 0) { 
      // Behave as if we were just the underlying all stats object.
      all->printTabular(depth,out);
  } else {
      out << spaces << "StatsRW begin\n";
      if (read) {
	  out << spaces << "read begin\n";
	  read->printTabular(depth+4,out);
	  out << spaces << "read end\n";
      }

      if (write) {
	  out << spaces << "write begin\n";
	  write->printTabular(depth+4,out);
	  out << spaces << "write end\n";
      }

      if (all) {
	  out << spaces << "all begin\n";
	  all->printTabular(depth+4,out);
	  out << spaces << "all end\n";
      }
      out << spaces << "StatsRW end\n";
  }
}
  
