#include <Python.h>
#include "Inject.h"

static PyObject *PyRunExec(PyObject *self, PyObject *args)
{
    char* Cmd;

    if (!PyArg_ParseTuple(args, "s", &Cmd))
    {
        return NULL;
    }

    int Ret = RunExec (Cmd);

    return Py_BuildValue("i", Ret);
}

static PyObject *PyRunSystem(PyObject *self, PyObject *args)
{
    char* Cmd;

    if (!PyArg_ParseTuple(args, "s", &Cmd))
    {
        return NULL;
    }

    int Ret = RunSystem (Cmd);

    return Py_BuildValue("i", Ret);
}

static PyMethodDef DemoMethods[] = 
{
    {"pyEXEC", PyRunExec, METH_VARARGS, "RunExec"},
    {"pySystem", PyRunSystem, METH_VARARGS, "RunSystem"},
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