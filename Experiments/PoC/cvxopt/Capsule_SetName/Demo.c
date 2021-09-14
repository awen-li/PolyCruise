#include <Python.h>

static PyObject *Capsule_SetName(PyObject *self, PyObject *args)
{
    char* Name = NULL;
    PyObject *Obj = NULL;

    if (!PyArg_ParseTuple(args, "Os", &Obj, &Name))
    {
        return NULL;
    }

    printf ("Gonna set new name [%s].\r\n", Name);
    PyCapsule_SetName (Obj, Name);
    printf ("Set result: [%s].\r\n",  PyCapsule_GetName(Obj));

    Py_RETURN_NONE;
}

void Destory(PyObject* null) 
{
    ;
}

static PyObject *Capsule_New(PyObject *self, PyObject *args)
{
    char* Name = NULL;
    PyObject *Obj = NULL;

    if (!PyArg_ParseTuple(args, "s", &Name))
    {
        return NULL;
    }

    char BUF[2];
    memset (BUF, sizeof (BUF), 0xff);
    
    Obj  = PyCapsule_New(BUF, Name, Destory);

    return Obj;
}

static PyMethodDef DemoMethods[] = 
{
    {"Capsule_SetName", Capsule_SetName, METH_VARARGS, "set name"},
    {"Capsule_New", Capsule_New, METH_VARARGS, "new object"},
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