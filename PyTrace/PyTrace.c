/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Python.h>
#include "Event.h"
#include "Queue.h"

void TRC_trace (ULONG EventId, const char* Format, ...);

/* unsigned long PyEventTy (unsigned FuncId, unsigned InstId, 
                            unsigned EventType, unsigned SSFlg) 
    Event Id definition:
    |4b language|4b type|2b soure/sink|18b FunctionId|12b Blockid|24b Instructionid|
*/
static  PyObject *PyEventTy(PyObject *self, PyObject *args)
{
    unsigned long FuncId;
    unsigned long InstId;
    unsigned long EvType;
    unsigned long SSFlg;
	
    if (!PyArg_ParseTuple(args, "kkkk", &FuncId, &InstId, &EvType, &SSFlg))
	{
        return NULL;
	}

	unsigned long EventId = F_LANG2EID (PYLANG_TY) | F_ETY2EID (EvType) | F_SSD2EID (SSFlg) |
                            F_FID2EID (FuncId) | F_BID2EID (0) | F_IID2EID (InstId);
    printf ("EventId = (%lu, %lu, %lu, %lu) -> %lx \r\n", FuncId, InstId, EvType, SSFlg, EventId);
	
    return Py_BuildValue("k", EventId);
}

/* void TRC_trace (ULONG EventId, const char* Format, ...) */
static PyObject *PyTrace(PyObject *self, PyObject *args)
{
    const int a, b;
	
    if (!PyArg_ParseTuple(args, "ii", &a, &b))
	{
        return NULL;
	}
	
	//int result = 0;
	
    return NULL;
}

static PyMethodDef TraceMethods[] = 
{
    {"PyTrace",   PyTrace,   METH_VARARGS, "Python tracing method."},
    {"PyEventTy", PyEventTy, METH_VARARGS, "Python Event type."},
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