#include <Python.h>
#include "Incmp.h"

static PyObject *PyInCmp(PyObject *self, PyObject *args)
{
    char* Cmd;

    if (!PyArg_ParseTuple(args, "s", &Cmd))
    {
        return NULL;
    }

    int Ret = Incmp (Cmd);

    return Py_BuildValue("i", Ret);
}

static PyMethodDef DemoMethods[] = 
{
    {"pyInCmp", PyInCmp, METH_VARARGS, "PyInCmp"},
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