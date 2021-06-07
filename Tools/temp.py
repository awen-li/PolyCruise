

def test_zeros_obj(self):
    ......
    with open('test_zeros', 'r') as reader:
        shape = reader.read ()
    d = np.zeros(shape, dtype=int)
    ......      PyMethodDef {"zeros", (PyCFunction)array_zeros, ...},

static PyObject *
array_zeros(PyObject *NPY_UNUSED(ignored),
        PyObject *const *args, Py_ssize_t len_args, PyObject *kwnames)
{
    PyArray_Dims shape = {NULL, 0};
    ......
    npy_parse_arguments("zeros", args, len_args, kwnames,
                        "shape", &PyArray_IntpConverter, &shape,...);
    ......
    PyArray_Zeros(shape.len, shape.ptr, typecode, (int) is_f_order);
    ......
}

NPY_NO_EXPORT PyObject *
PyArray_Zeros(int nd, npy_intp const *dims, PyArray_Descr *type, int is_f_order)
{
    ......
    PyArray_NewFromDescr_int(&PyArray_Type, type, nd, dims, NULL, NULL,
                             is_f_order, NULL, NULL, 1, 0);
    ......
}

NPY_NO_EXPORT PyObject *
PyArray_NewFromDescr_int(
        PyTypeObject *subtype, PyArray_Descr *descr, int nd,
        npy_intp const *dims, npy_intp const *strides, void *data,
        int flags, PyObject *obj, PyObject *base, int zeroed,
        int allow_emptystring)
{
    ......
    if (descr->subarray) {
        npy_intp newdims[2*NPY_MAXDIMS];
        npy_intp *newstrides = NULL;
        memcpy(newdims, dims, nd*sizeof(npy_intp));
        if (strides) {
            newstrides = newdims + NPY_MAXDIMS;
            memcpy(newstrides, strides, nd*sizeof(npy_intp));
        }
    }
    ......
}
    




