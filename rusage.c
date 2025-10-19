/*rusage.c file contains the source code of the ressource usage python module, this module is implemeted in C and usable in python code*/
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
#include <sys/time.h>
#include <sys/resource.h>




static PyObject* sysgetrusage(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
	 struct rusage usage;
	int who = PyLong_AsLong(args[0]); //Parsing the (only) argument
	getrusage(who, &usage);//getrusage system call
	PyObject *dict = PyDict_New();//The dictionary that will contain the data


	PyDict_SetItemString(dict, "ru_maxrss", PyLong_FromLong(usage.ru_maxrss));//memory consumtion
	PyDict_SetItemString(dict, "ru_ixrss", PyLong_FromLong(usage.ru_ixrss))//shared memory size
	PyDict_SetItemString(dict, "ru_idrss", PyLong_FromLong(usage.ru_idrss))//unshared data size 
	PyDict_SetItemString(dict, "ru_isrss", PyLong_FromLong(usage.ru_isrss))//unshared stack size
	PyDict_SetItemString(dict, "ru_minflt", PyLong_FromLong(usage.ru_minflt));//soft page fault
   	PyDict_SetItemString(dict, "ru_majflt", PyLong_FromLong(usage.ru_majflt));//hard page fault
	PyDict_SetItemString(dict, "ru_nswap", PyLong_FromLong(usage.ru_nswap))//number of swaps
	PyDict_SetItemString(dict, "ru_inblock", PyLong_FromLong(usage.ru_inblock));//Input operations
	PyDict_SetItemString(dict, "ru_oublock", PyLong_FromLong(usage.ru_oublock));//Output operations
	PyDict_SetItemString(dict, "ru_msgsnd", PyLong_FromLong(usage.ru_msgsnd))//IPC sent messages  
	PyDict_SetItemString(dict, "ru_msgrcv", PyLong_FromLong(usage.ru_msgrcv))// IPC received messages  
	PyDict_SetItemString(dict, "ru_nsignals", PyLong_FromLong(usage.ru_nsignals))//number of signals
	PyDict_SetItemString(dict, "ru_nvcsw", PyLong_FromLong(usage.ru_nvcsw));//volentary giving-up
	PyDict_SetItemString(dict, "ru_nivcsw", PyLong_FromLong(usage.ru_nivcsw));//non-volentary context switching

	return dict;//Finally the dictionay representing the ressources is returned hear !
}




static PyMethodDef RusageMethods[] = {
	//List of Methods
    {"getrusage", sysgetrusage, METH_FASTCALL, "Get ressource usage"},
    {NULL, NULL, 0, NULL}//Null array is needed to indicate the end
};


static struct PyModuleDef rusage_module = {
    PyModuleDef_HEAD_INIT,
    "rusage",                  
    "TPM NV INDEX interraction", 
    -1,                     
    RusageMethods              
};


PyMODINIT_FUNC PyInit_rusage(void) {
     PyObject *module = PyModule_Create(&rusage_module);
	  PyModule_AddIntConstant(module, "RUSAGE_SELF", RUSAGE_SELF);//Define the constant RUSAGE_SELF which means that resources are evaluated for the same process 
    PyModule_AddIntConstant(module, "RUSAGE_CHILDREN", RUSAGE_CHILDREN);//Define the constant RUSAGE_CHILDREN which means that resources are evaluated for all the children of that process without distiction
	return module;
}
