#include <Python.h>

extern void Overflow (char *Module, int Value);
static PyObject *PwdInfo(PyObject *self, PyObject *args)
{
    char* Module;
	int Value = 0;
	
    if (!PyArg_ParseTuple(args, "si", &Module, &Value))
	{
        return NULL;
	}
	
	Overflow (Module, Value);
	
    Py_RETURN_NONE;
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