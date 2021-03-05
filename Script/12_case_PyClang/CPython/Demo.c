#include <Python.h>
#include "Trace.h"

static PyObject *DemoTrace(PyObject *self, PyObject *args)
{
    char* Module = NULL;
	int Value = 0;
	
    if (!PyArg_ParseTuple(args, "si", &Module, &Value))
	{
        return NULL;
	}
	
	Trace (Module, Value);
	
    Py_RETURN_NONE;
}

static PyMethodDef DemoMethods[] = 
{
    {"DemoTrace", DemoTrace, METH_VARARGS, "trace functions"},
    {NULL, NULL, 0, NULL} 
};

static struct PyModuleDef ModPyDemo =
{
    PyModuleDef_HEAD_INIT,
    "PyDemo",    /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    DemoMethods
};

PyMODINIT_FUNC PyInit_PyDemo(void)
{
    return PyModule_Create(&ModPyDemo);
}