/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Python.h>
#include "Queue.h"

void TRC_trace (ULONG EventId, const char* Format, ...);

/* void TRC_trace (ULONG EventId, const char* Format, ...) */
static PyObject *PyTrace(PyObject *self, PyObject *args, ...)
{
    const int a, b;
	
    if (!PyArg_ParseTuple(args, "ii", &a, &b))
	{
        return NULL;
	}
	
	int result = 0;
	
    return Py_BuildValue("i", result);
}

static PyMethodDef TraceMethods[] = 
{
    {"PyTrace", PyTrace, METH_VARARGS, "Python tracing method."},
    {NULL, NULL, 0, NULL} 
};

static struct PyModuleDef ModPyTrace =
{
    PyModuleDef_HEAD_INIT,
    "ModPyTrace", /* name of module */
    "",           /* module documentation, may be NULL */
    -1,           /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    TraceMethods
};

PyMODINIT_FUNC PyInit_PyTrace(void)
{
    return PyModule_Create(&ModPyTrace);
}