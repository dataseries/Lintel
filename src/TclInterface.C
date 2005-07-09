/* -*-C++-*-
*******************************************************************************
*
* File:         TclInterface.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/TclInterface.C,v 1.10 2005/02/14 04:36:52 anderse Exp $
* Description:  Tcl/C++ interface functions
* Author:       Alistair Veitch
* Created:      Fri Feb 18 14:42:39 2000
* Modified:     Thu Jan 20 13:28:35 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <list>
#include <tcl.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "streamcompat.H"
#include "TclInterface.H"

// ========================================================================
// Class for storing name/procedure pairs.  These name/procedure pairs
// need to be registered with the Tcl interpreter, once the interpreter
// is created.  The public interface to this class are the two static
// functions add and registerAll.  Because there is no need for users
// of this class to instantiate objects of this class, the constructor
// is private.
class TIFNameProcPair {
private:
    TIFNameProcPair(std::string &name_in, Tcl_ObjCmdProc proc_in);

    // The destructor and copy constructor have to be public (so STL
    // can get to it).  This is not harmful, since nobody will ever
    // have an object of this class to copy.
public:
    virtual ~TIFNameProcPair();

    TIFNameProcPair(const TIFNameProcPair& old);

private:
    // The assignment operator is not implemented.
    const TIFNameProcPair& operator=(const TIFNameProcPair& old);

    // Data members:
    std::string name;
    Tcl_ObjCmdProc *proc;
  
    // The actual list of all entries:
    static std::list<TIFNameProcPair> *all_pairs;

public:
    // Call add to add one name/procedure pair to the list of things 
    // which need to be set up.  This should be done before calling
    // registerAll.
    static void add(std::string &name_in, Tcl_ObjCmdProc proc_in);

    // Call registerAll to tell the indicated Tcl interpreter about
    // all these name / procedure pairs (i.e., commands).  Call this
    // after calling add() multiple times.  After calling this method,
    // the list of pairs will be empty, so calling it multiple times is
    // wasteful, but harmless.
    static void registerAll(Tcl_Interp* interp);
};

std::list<TIFNameProcPair> *TIFNameProcPair::all_pairs;

// Regular constructor (this one is private)
TIFNameProcPair::TIFNameProcPair(std::string &name_in, Tcl_ObjCmdProc proc_in) :
    name(name_in),
    proc(proc_in) {
}

// Copy constructor (this one is public)
TIFNameProcPair::TIFNameProcPair(TIFNameProcPair const& old) :
    name(old.name),
    proc(old.proc) {
}

// Destructor (also public):
TIFNameProcPair::~TIFNameProcPair() { }

// Add a new pair to the list to be processed later:
void TIFNameProcPair::add(std::string &name_in, Tcl_ObjCmdProc* proc_in) {
    if (all_pairs == NULL) {
	all_pairs = new std::list<TIFNameProcPair>;
    }
    all_pairs->push_back(TIFNameProcPair(name_in, proc_in));
}

// Register everything currently in the list with the interpreter:
void TIFNameProcPair::registerAll(Tcl_Interp* interp) {
    if (all_pairs == NULL) {
	all_pairs = new std::list<TIFNameProcPair>;
    }
    for (std::list<TIFNameProcPair>::iterator i = all_pairs->begin();
         i != all_pairs->end(); ++i) {
        Tcl_CreateObjCommand(interp, const_cast<char *>(i->name.c_str()),
			     i->proc, NULL, NULL);
    }
    // Since we have processed everything in the list, we delete it:
    all_pairs->erase(all_pairs->begin(), all_pairs->end());
}

// ========================================================================

std::string TIFusage;	// used for printing usage messages

/*
 * code in this section registers a new Tcl type. It constitutes the
 * following functions:
 *	TIFsetFromAnyProc: Converts a Tcl object into a TIF object, *if*
 *      		   it can be done. generates error if not.
 *	TIFupdateStringProc: updates the std::string representation of a TIF object
 *	TIFdupIntRepProc: duplicates a TIF object
 * These functions are called automatically by the Tcl type system as
 * required. They are registered by TIFinit()
 */

static const char *TIFname = "TclInterface";
static const int TIFnameLen = strlen(TIFname);
static Tcl_ObjType *TIFtype;
static Tcl_Interp *TIFinterp = NULL;

// utility function to set Tcl error string
static void setError(Tcl_Interp *interp, const char *msg) {
    if (interp != NULL) {
	Tcl_SetResult(interp, const_cast<char *>(msg), TCL_STATIC);
    }
}

int TIFsetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr) {
    Tcl_ObjType *oldTypePtr = objPtr->typePtr;
    if (oldTypePtr == TIFtype) {
	return TCL_OK;
    }
    char *string = Tcl_GetStringFromObj(objPtr, NULL);
    
    if (strncmp(string, TIFname, TIFnameLen) != 0) {
	setError(interp, "not a TclInterface object");
	return TCL_ERROR;
    }
    char *address = strchr(string, '@');
    if (address == NULL) {
	setError(interp, "no @ in object name");
        return TCL_ERROR;
    }
    address++;
    if ((oldTypePtr != NULL) && (oldTypePtr->freeIntRepProc != NULL)) {
        oldTypePtr->freeIntRepProc(objPtr);
    }
    objPtr->typePtr = TIFtype;
    objPtr->internalRep.otherValuePtr = (char *)strtoul(address, NULL, 16);
    return TCL_OK;
}

static void TIFupdateStringProc(Tcl_Obj *objPtr) {
    char result[100];
    sprintf(result, "%s@%x", TIFname,
	    (unsigned)(long)objPtr->internalRep.otherValuePtr);
    objPtr->length = strlen(result);
    objPtr->bytes = Tcl_Alloc(objPtr->length + 1);
    strcpy(objPtr->bytes, result);
}

static void TIFdupIntRepProc(Tcl_Obj *srcPtr, Tcl_Obj *dupPtr) {
    dupPtr->internalRep.otherValuePtr = srcPtr->internalRep.otherValuePtr;
    dupPtr->typePtr = TIFtype;
}

// Do initialization
Tcl_Interp *TIFinit(Tcl_Interp *ti) {
    // if we've already been called, just return interpreter
    if (TIFinterp != NULL) {
	return TIFinterp;
    }

    // create/initialize interpreter
    TIFinterp = (ti != NULL) ? ti : Tcl_CreateInterp();
    
    // create the TclInterface object type
    TIFtype = new Tcl_ObjType();
    TIFtype->name = strdup("TclInterface");
    TIFtype->freeIntRepProc = NULL;
    TIFtype->dupIntRepProc = TIFdupIntRepProc;
    TIFtype->updateStringProc = TIFupdateStringProc;
    TIFtype->setFromAnyProc = TIFsetFromAnyProc;
    Tcl_RegisterObjType(TIFtype);

    // register all the base TIF constructors and functions
    TIFNameProcPair::registerAll(TIFinterp);

    // register one name, to ensure that TIFenumTable is initialised
    TclEnum throwAway("ATclEnumNameThatWontBeUsed", 0);
    
    return TIFinterp;
}

Tcl_Interp *getTIFinterpreter()
{
    return TIFinterp;
}

TclInterface::TclInterface(std::string name, Tcl_ObjCmdProc proc) {
    TIFNameProcPair::add(name, proc);
};

//==========================================================================

/*
 * enum values get registered in a table, so we can look them up as needed
 * (i.e. when Tcl passes a string instead of a value). This code handles
 * that (happens as a byproduct of creating a TclEnum object).
 */
bool TclEnum::init = false;

// TIFenumTable is the repository for all the "real" translations
Tcl_HashTable TIFenumTable;

TclEnum::TclEnum(std::string name, int value) {
    if (!init) {
	Tcl_InitHashTable(&TIFenumTable, TCL_STRING_KEYS);
	init = true;
    }
    
    int newFlag;
    Tcl_HashEntry *entryPtr = Tcl_CreateHashEntry(&TIFenumTable, name.c_str(),
						  &newFlag);
    if (newFlag == 0) {
	std::cerr << "duplicate enum name " << name << std::endl;
	exit(0);
    }
    Tcl_SetHashValue(entryPtr, value);
};

