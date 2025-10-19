#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syscall.h>
#include <sched.h>
#include <signal.h>


static PyObject* sysclone(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
 return PyLong_FromLong(syscall(SYS_clone,   CLONE_FILES | SIGCHLD | CLONE_FS,NULL));

}




static PyMethodDef CloneMethods[] = {
	//List of Methods
    {"clone", sysclone, METH_FASTCALL, "Create new thread"},
    {NULL, NULL, 0, NULL}//Null array is needed to indicate the end
};


static struct PyModuleDef clone_module = {
    PyModuleDef_HEAD_INIT,
    "clone",                  
    "Threading ", 
    -1,                     
    CloneMethods              
};


PyMODINIT_FUNC PyInit_clone(void) {
    return PyModule_Create(&clone_module);
}
