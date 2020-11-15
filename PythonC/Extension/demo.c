#include <Python.h>

static char *gMalloc = NULL;

int add (int a, int b)
{
	int result = a + b;

	return result;
}

static PyObject *demo_add(PyObject *self, PyObject *args)
{
    const int a, b;
	
    if (!PyArg_ParseTuple(args, "ii", &a, &b))
	{
        return NULL;
	}
	
	int result = add (a,b);
	
    return Py_BuildValue("i", result);
}

static PyMethodDef DemoMethods[] = 
{
    {"add", demo_add, METH_VARARGS, "Add two integers"},
    {NULL, NULL, 0, NULL} 
};

static struct PyModuleDef ModPyDemo =
{
    PyModuleDef_HEAD_INIT,
    "ModPyDemo", /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    DemoMethods
};

PyMODINIT_FUNC PyInit_demo(void)
{
    return PyModule_Create(&ModPyDemo);
}