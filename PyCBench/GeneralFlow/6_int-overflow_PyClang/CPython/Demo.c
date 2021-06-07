#include <Python.h>
#include "Incmp.h"

int Overflow (unsigned Value);
static PyObject *PyInCmp(PyObject *self, PyObject *args)
{
    unsigned Value;

    if (!PyArg_ParseTuple(args, "i", &Value))
    {
        return NULL;
    }

    int Ret = Overflow (Value);

    return Py_BuildValue("i", Ret);
}

static PyMethodDef DemoMethods[] = 
{
    {"pyOverflow", PyInCmp, METH_VARARGS, "pyOverflow"},
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