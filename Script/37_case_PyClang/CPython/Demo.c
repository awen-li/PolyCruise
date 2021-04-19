#include <Python.h>
#include "Passwd.h"

static PyObject *PwdInfo(PyObject *self, PyObject *args)
{
    int Value = 0;

    if (!PyArg_ParseTuple(args, "i", &Value))
    {
        return NULL;
    }

    char* pwd  =  Getpasswd (Value);
    
    return Py_BuildValue("s", pwd);
}

static PyMethodDef DemoMethods[] = 
{
    {"PwdInfo", PwdInfo, METH_VARARGS, "pwd information"},
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