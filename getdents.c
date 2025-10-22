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



static PyObject* sysopendir(PyObject *self, PyObject *const *args, Py_ssize_t nargs){
return PyLong_FromLong(open(PyUnicode_AsUTF8(args[0]),O_RDONLY | O_DIRECTORY ) );
}

static PyObject* sysgetdents(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {

	int fd = PyLong_AsLong(args[0]);
	int count = PyLong_AsLong(args[1]);
	PyObject* dents_list = PyList_New(0);

	int nread;
	 char *buf;
	   void *d;
	    int bpos;	
	   char d_type;
	   ino64_t  d_ino; 
	   off64_t     d_off;
	   int position;
	nread = syscall(SYS_getdents64, fd, buf, count);
	for (bpos = 0; bpos < nread;bpos +=*((unsigned short *) (d +16) )) {
		PyObject *dent_dict = PyDict_New();
		d =  (buf + bpos);
		d_ino  = *((d_ino*)(buf + bpos ));
		d_off  = *((d_ino*)(buf + bpos + 8 ));	
		d_type = *(buf + bpos +18);
		const char *name = ((char *) (d +19) );

		PyDict_SetItemString(dent_dict, "d_type", PyLong_FromLong(d_type));	
		PyDict_SetItemString(dent_dict, "d_off", PyLong_FromLong(d_off));
	       	PyDict_SetItemString(dent_dict, "name", PyUnicode_FromString(name));	
		PyList_Append(dents_list, dent_dict);	
		

		}


		return dents_list;
}



static PyMethodDef getdentsMethods[] = {
	//List of Methods
    {"open", sysopendir, METH_FASTCALL, "Get ressource usage"},  	
    {"getdents", sysgetdents, METH_FASTCALL, "Get ressource usage"},
    {NULL, NULL, 0, NULL}//Null array is needed to indicate the end
};


static struct PyModuleDef getdents_module = {
    PyModuleDef_HEAD_INIT,
    "rusage",                  
    "getdents for listing a dirctory", 
    -1,                     
    getdentsMethods              
};



PyMODINIT_FUNC PyInit_getdents(void) {
    return PyModule_Create(&getdents_module);
}


