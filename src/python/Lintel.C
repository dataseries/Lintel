#include <iostream>
#include "Stats.H"
#include "StatsQuantile.H"
#include "Clock.H"
extern "C" {
#include <Python.h>
};

extern PyTypeObject PStatsType;

typedef struct {
    PyObject_HEAD
    StatsBase *stats;
    PyMethodDef *methods;
} PStatsObject;

static void Lintel_dealloc(PyObject *o_)
{
    delete reinterpret_cast<PStatsObject*>(o_)->stats;
}

#define METHOD(name)						\
PyObject *PStatsBase_##name(PStatsObject *o, PyObject *args)	\
{								\
    return Py_BuildValue("d", o->stats->name());		\
}

METHOD(min)
METHOD(max)
METHOD(mean)
METHOD(stddev)
METHOD(variance)
METHOD(conf95)
METHOD(relconf95)

static PyMethodDef PStatsBaseMethods[] = {
    {"min", (PyCFunction)PStatsBase_min, METH_VARARGS, ""},
    {"max", (PyCFunction)PStatsBase_max, METH_VARARGS, ""},
    {"mean", (PyCFunction)PStatsBase_mean, METH_VARARGS, ""},
    {"stddev", (PyCFunction)PStatsBase_stddev, METH_VARARGS, ""},
    {"variance", (PyCFunction)PStatsBase_variance, METH_VARARGS, ""},
    {"conf95", (PyCFunction)PStatsBase_conf95, METH_VARARGS, ""},
    {"relconf95", (PyCFunction)PStatsBase_relconf95, METH_VARARGS, ""},
    {NULL, NULL}
};

/* 
   Vanilla "Stats" class. 
*/
static PyMethodDef *PStatsMethods;

PyObject *PStats_add(PStatsObject *o, PyObject *args)
{
    double f;
    if (!PyArg_ParseTuple(args, "d", &f)) {
	return NULL;
    }
    static_cast<Stats*>(o->stats)->add(f);
    return Py_BuildValue("s#", NULL, 0);
}

static PyObject *new_Stats(PyObject *self, PyObject *args)
{
    PStatsObject *o;
    o = PyObject_NEW(PStatsObject, &PStatsType);
    o->stats = new Stats();
    o->methods = PStatsMethods;
    return (PyObject*)o;
}

/* 
   "StatsQuantile" class. 
*/
static PyMethodDef *PStatsQuantileMethods;

PyObject *PStatsQuantile_add(PStatsObject *o, PyObject *args)
{
    double f;
    if (!PyArg_ParseTuple(args, "d", &f)) {
	return NULL;
    }
    static_cast<StatsQuantile*>(o->stats)->add(f);
    return Py_BuildValue("s#", NULL, 0);
}
PyObject *PStatsQuantile_getQuantile(PStatsObject *o, PyObject *args)
{								
    double f;
    if (!PyArg_ParseTuple(args, "d", &f)) {
	return NULL;
    }
    f = static_cast<StatsQuantile*>(o->stats)->getQuantile(f);
    return Py_BuildValue("d", f);
}

static PyObject *new_StatsQuantile(PyObject *self, PyObject *args)
{
    PStatsObject *o;
    o = PyObject_NEW(PStatsObject, &PStatsType);
    o->stats = new StatsQuantile();
    o->methods = PStatsQuantileMethods;
    return (PyObject*)o;
}

static PyObject *Lintel_getattr(PyObject *o_, char *name)
{
    PStatsObject *o = reinterpret_cast<PStatsObject*>(o_);
    return Py_FindMethod(o->methods, o_, name);
}

PyTypeObject PStatsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Stats",
    sizeof(PStatsObject),
    0,
    Lintel_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    Lintel_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    0,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};    


static PyObject *clock_rate(PyObject *self, PyObject *args)
{
    return Py_BuildValue("d", Clock::clock_rate);
}
static PyObject *clock_now(PyObject *self, PyObject *args)
{
    Clock::Tll v = Clock::now();
    return PyLong_FromUnsignedLongLong(v);
}

static PyMethodDef Lintel_methods[] = {
    {"new_Stats", new_Stats, METH_VARARGS, "Create a new Stats object."},
    {"new_StatsQuantile", new_StatsQuantile, METH_VARARGS, "Create a new StatsQuantile object."},
    {"clock_rate", clock_rate, METH_VARARGS, "Get the CPU clock rate."},
    {"clock_now", clock_now, METH_VARARGS, "Get the CPU clock rate."},
    {NULL, NULL, 0, NULL}
};

static PyMethodDef *merge_methods(PyMethodDef *d1, PyMethodDef *d2)
{
    int n = 0;
    for (PyMethodDef *p = d1; p->ml_name; p++) 
	n++;
    for (PyMethodDef *p = d2; p->ml_name; p++) 
	n++;
    PyMethodDef *r = new PyMethodDef[n + 1];
    n = 0;
    for (PyMethodDef *p = d1; p->ml_name; p++) 
	r[n++] = *p;
    for (PyMethodDef *p = d2; p->ml_name; p++) 
	r[n++] = *p;
    memset(&r[n], 0, sizeof r[n]);
    return r;
}

extern "C"
DL_EXPORT(void)
initLintel(void) 
{
    PStatsType.ob_type = &PyType_Type;

    Clock::calibrateClock(true,0.001);

    Py_InitModule("Lintel", Lintel_methods);

    static PyMethodDef stat_methods[] = {
	{"add", (PyCFunction)PStats_add, METH_VARARGS, "Add a value to the stats object"},
	{NULL, NULL},
    };

    PStatsMethods = merge_methods(PStatsBaseMethods, stat_methods);

    static PyMethodDef quantile_methods[] = {
	{"add", (PyCFunction)PStatsQuantile_add, METH_VARARGS, "Add a value to the stats object"},
	{"getQuantile", (PyCFunction)PStatsQuantile_getQuantile, METH_VARARGS, "Get quantile"},
	{NULL, NULL},
    };
    PStatsQuantileMethods = merge_methods(PStatsBaseMethods, quantile_methods);
    Clock c;
}
