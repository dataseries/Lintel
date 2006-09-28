/* -*-C++-*- */
/*
   (c) Copyright 1999-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    TclInterface object definition.
*/


#ifndef _TCL_INTERFACE_H
#define _TCL_INTERFACE_H

#include <string>
#include <tcl.h>

// Call this function once, to initialize all the Tcl Interface functions.
// You can specify an already existing Tcl interpreter.  If you don't
// specify one, a new one will be created.
Tcl_Interp *TIFinit(Tcl_Interp *ti = NULL);

// converts a tcl object into a TIF object, *if* ir can be done. Generates
// and error if not
int TIFsetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr);

// Access the interpreter which is being used by TIF.  A program can
// have multiple Tcl interpreters, but only one is being used by TIF
// (by having been supplied to TIFinit or made by it).
// This function might return NULL, if it is called before TIFinit.
// You can call this function inside a TIFfunction or TIFmethod, if
// you have to access the interpreter.
Tcl_Interp *getTIFinterpreter();

extern std::string TIFusage;

class TclInterface {
private:
public:
    TclInterface(std::string name, Tcl_ObjCmdProc proc);
    TclInterface() {};
    virtual ~TclInterface() { };
};


/*
 * TIFenum - this class is used for registering enum name/value pairs, for
 * use by the Tcl code
 */

class TclEnum {
private:
    static bool init;
public:
    TclEnum(std::string name, int value);	// registers name
    virtual ~TclEnum() {};
};

// TIFenumTable is the repository for all the "real" translations
extern Tcl_HashTable TIFenumTable;

#endif // _TCL_INTERFACE_H
