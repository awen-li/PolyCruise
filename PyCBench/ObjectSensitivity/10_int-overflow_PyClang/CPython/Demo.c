#include <Python.h>
#include "BinOp.h"

static PyObject *pyBinOp(PyObject *self, PyObject *args)
{
    int Oper;

    if (!PyArg_ParseTuple(args, "i", &Oper))
    {
        return NULL;
    }

    int Ret = BinOp (Oper);

    return Py_BuildValue("i", Ret);
}

static PyMethodDef DemoMethods[] = 
{
    {"pyBinOp", pyBinOp, METH_VARARGS, "pyBinOp"},
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