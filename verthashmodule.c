#include <Python.h>

#include "h2.h"

static PyObject *verthash_getpowhash(PyObject *self, PyObject *args)
{
    char *output;
    PyObject *value;
#if PY_MAJOR_VERSION >= 3
    PyBytesObject *input;
    PyBytesObject *datfile;
#else
    PyStringObject *input;
    PyStringObject *datfile;
#endif
    if (!PyArg_ParseTuple(args, "SS", &input, &datfile))
        return NULL;
    Py_INCREF(input);
    Py_INCREF(datfile);
    output = PyMem_Malloc(32);

#if PY_MAJOR_VERSION >= 3
    const Py_ssize_t datfile_size = PyBytes_Size((PyObject*) datfile);
    const Py_ssize_t input_size = PyBytes_Size((PyObject*) input);

    verthash_hash(PyBytes_AsString((PyObject*) datfile), datfile_size, PyBytes_AsString((PyObject*) input), input_size, output);
#else
    const Py_ssize_t datfile_size = PyString_Size((PyObject*) datfile);
    const Py_ssize_t input_size = PyString_Size((PyObject*) input);

    verthash_hash(PyString_AsString((PyObject*) datfile), datfile_size, PyString_AsString((PyObject*) input), input_size, output);
#endif
    Py_DECREF(input);
    Py_DECREF(datfile);
#if PY_MAJOR_VERSION >= 3
    value = Py_BuildValue("y#", output, 32);
#else
    value = Py_BuildValue("s#", output, 32);
#endif
    PyMem_Free(output);
    return value;
}

static PyMethodDef VerthashMethods[] = {
    { "getPoWHash", verthash_getpowhash, METH_VARARGS, "Returns the proof of work hash using Verthash" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef VerthashModule = {
    PyModuleDef_HEAD_INIT,
    "verthash",
    "...",
    -1,
    VerthashMethods
};

PyMODINIT_FUNC PyInit_verthash(void) {
    return PyModule_Create(&VerthashModule);
}

#else

PyMODINIT_FUNC initverthash(void) {
    (void) Py_InitModule("verthash", VerthashMethods);
}
#endif
