/* tpm.c file contains the source code of the tpm python module, this module is implemeted in C and usable in python code
IMPORTANT :: For the time beging the data that is read from/wirtten into TPM has a fixed size of 128bit (16 bytes)
In the future the size may be changed !

*/
#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>




static PyObject* tpm_nv_read(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
//tpm_nv_read() function reads data from TPM




	int tpm_fd=open("/dev/tpm0", O_RDWR); 
		if(tpm_fd<0) return PyLong_FromLong(errno);//If the device file fail to open, then the errno is returned as PyLong

    const char* password;//Password of the NV_INDEX
   uint32_t nv_index;//The NV_INDEX itself

	//Parsing the arguments
  nv_index = PyLong_AsLong(args[0]);
	password = PyUnicode_AsUTF8(args[1]);
	
	
        
        char tpm_response_buffer[2048];//the TPM responce will be stored hear
        int k;
	
	

	

     const char tpm_request [] =  {0x80,0x02,
                                0x00,0x00,0x00,0x00,
                                0x00,0x00,0x01,0x4e,//Command = NV_READ = 0x0000014E
				  0x00,0x00,0x00,0x00,
			        0x00,0x00,0x00,0x00,     

       				 0x00, 0x00,0x00, 0x09,  
        0x40, 0x00, 0x00, 0x09,    
        0x00, 0x00, 0x01, 0x00, 0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,


        };
 char *nvread_payload;

	//NV_INDEX should be rotated before being used in TPM
	nv_index = (nv_index & 0xff) << 24 |  (nv_index & 0xff00) << 8 |  (nv_index & 0xff0000) >> 8 |  (nv_index & 0xff000000) >> 24 ;
	*((uint32_t*)(tpm_request + 10)) = nv_index;
	*((uint32_t*)(tpm_request + 14)) = nv_index;

	unsigned int password_size = strlen(password);
	 uint32_t  auth_size = password_size + 9;//Authentification size is always nine more than password size
	//Rotation should be done to authentification size
        auth_size = (auth_size & 0xff) << 24 |  (auth_size & 0xff00) << 8 |  (auth_size & 0xff0000) >> 8 |  (auth_size & 0xff000000) >> 24 ;
	//the password size should be converted from 32bit to 16 bit and mirrored before being used in TPM
         uint16_t password_length = password_size;
         password_length =  (password_length & 0xff) << 8 |(password_length & 0xff00) >> 8; 

		
		//Auth Size and Pasword Size should be set
		*((uint32_t*)(tpm_request + 18)) = auth_size;
        *((uint16_t*)(tpm_request + 29)) = password_length;


	//For the time being the datasize is fixed to 128 bit (it may be changed later)
	strcpy(tpm_request+31,password);
	*((uint8_t*)(tpm_request + (31+password_size)))=0x00;
        *((uint8_t*)(tpm_request + (32+password_size)))=0x10;
        *((uint8_t*)(tpm_request + (33+password_size)))=0x00;
        *((uint8_t*)(tpm_request + (34+password_size)))=0x00;


	//Command size is the size of the entire TPM command it is 35+password size (for NV_READ) of course it should be rotated before being used in TPM
	uint32_t command_size = 35+password_size;
	command_size =  (command_size & 0xff) << 24 |  (command_size & 0xff00) << 8 |  (command_size & 0xff0000) >> 8 |  (command_size & 0xff000000) >> 24 ;
	*((uint32_t*)(tpm_request + 2)) = command_size;


  int tpm_request_size=write(tpm_fd,tpm_request,password_size + 35); //Sending data to TPM
   int tpm_response_size=read(tpm_fd,tpm_response_buffer,4096);// Receiving response from it
  uint32_t code = *( (uint32_t*) (tpm_response_buffer + 6) );// the response code is the bytes [6:9] of the response

if(code!=0) {//the code should be zero for successful operation
        printf("ERROR  : %x \n",code); 
	return PyLong_FromLong(code);}





	//the size of the data which are read are on the two bytes 14 and 15, this number should be mirrored before being interpreted
  unsigned int payload_size = (((unsigned short  int)tpm_response_buffer[14]) << 8) + ((unsigned short  int) tpm_response_buffer[15]);
nvread_payload = ((char*)(tpm_response_buffer + 16 )); //The data itself startfs from the byte 16 till (16+ payload_size)
    
	if(payload_size > 1024) payload_size = 1024;
	close(tpm_fd);

    
	return PyBytes_FromStringAndSize(nvread_payload, (Py_ssize_t)payload_size);
}

static PyObject* tpm_nv_write(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
//tpm_nv_write() function writes data into TPM


	




	//the arguments are read the same as tpm_nv_read()
    const char* password;
     uint32_t nv_index;	
    const char* data;//Except, the data which should be written must be indicated as argument
    Py_ssize_t data_size;

   nv_index = PyLong_AsLong(args[0]); 
	password = PyUnicode_AsUTF8(args[1]); 
	data = PyBytes_AsString(args[2]);

	int tpm_fd=open("/dev/tpm0", O_RDWR); 
		if(tpm_fd<0) return PyLong_FromLong(errno);//also the same
	
    

        
        unsigned char tpm_response_buffer[2048];
        int tpm_response_size;
        int tpm_request_size;
        int k;

  unsigned char tpm_request [] =  {0x80,0x02,
                                0x00,0x00,0x00,0x00,
                                0x00,0x00,0x01,0x37,//Command = NV_WRITE = 0x00000137
                                0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,
                          0x00, 0x00,0x00, 0x09, 	
     		   0x40, 0x00, 0x00, 0x09,   
    		    0x00, 0x00, 0x01, 0x00, 0x00,


	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};


	
	data_size=16;





	//same as NV_READ
	nv_index = (nv_index & 0xff) << 24 |  (nv_index & 0xff00) << 8 |  (nv_index & 0xff0000) >> 8 |  (nv_index & 0xff000000) >> 24 ;
	*((uint32_t*)(tpm_request + 10)) = nv_index;
	*((uint32_t*)(tpm_request + 14)) = nv_index;
	
    



	//Also authentification and password are managed the same as NV_READ
 	unsigned int password_size = strlen(password);
	 uint32_t  auth_size = password_size + 9;
        auth_size = (auth_size & 0xff) << 24 |  (auth_size & 0xff00) << 8 |  (auth_size & 0xff0000) >> 8 |  (auth_size & 0xff000000) >> 24 ;
         uint16_t password_length = password_size;
         password_length =  (password_length & 0xff) << 8 |(password_length & 0xff00) >> 8; 


		*((uint32_t*)(tpm_request + 18)) = auth_size;
        *((uint16_t*)(tpm_request + 29)) = password_length;


	strcpy(tpm_request+31,password);
	char * tpm_write_buffer = tpm_request + (33+password_size);

			*((uint8_t*)(tpm_request + (31+password_size)))=0x00;
			*((uint8_t*)(tpm_request + (32+password_size)))=0x10;
			memcpy(tpm_write_buffer,data,data_size); 



	//In the commend size the size of the data should be addeed, and most importently the commend size should be rotated before being sent to TPM
 	uint32_t command_size = password_size + data_size + 35;
	command_size =  (command_size & 0xff) << 24 |  (command_size & 0xff00) << 8 |  (command_size & 0xff0000) >> 8 |  (command_size & 0xff000000) >> 24 ;

  *((uint32_t*)(tpm_request + 2)) = (command_size);
	


	
	//The data are sent and received from TPM the same as NV_READ 
  tpm_request_size=write(tpm_fd,tpm_request,35+password_size+data_size);//except the size of the data that should be writen must be considered when sending commands to TPM.
        tpm_response_size=read(tpm_fd,tpm_response_buffer,4096);
	

int code = *( (uint32_t*) (tpm_response_buffer + 6) );
 if(code!=0) {
	printf("ERROR  : %x \n",code);
	return PyLong_FromLong(code); }




close(tpm_fd);


   return PyLong_FromLong(code); }


static PyObject* tpm_nv_define(PyObject *self, PyObject *const *args, Py_ssize_t nargs){
//tpm_nv_define() function defines new NV INDEX in the TPM



	//the arguments are read the same as tpm_nv_read()
    const char* password;
     uint32_t nv_index;	
    const char* data;//Except, the data which should be written must be indicated as argument
    Py_ssize_t data_size;

   nv_index = PyLong_AsLong(args[0]); 
	password = PyUnicode_AsUTF8(args[1]); 



	int tpm_fd=open("/dev/tpm0", O_RDWR); 
		if(tpm_fd<0) return PyLong_FromLong(errno);//also the same

    

        
        unsigned char tpm_response_buffer[2048];
        int tpm_response_size;
        int tpm_request_size;
        int k;



	unsigned char tpm_request [] = { 
		 0x80, 0x02,
		 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x2A,//ND_DefineSpace
		0x40, 0x00, 0x00, 0x01,//TPM_RH_OWNER


		 0x00, 0x00, 0x00, 0x09,//Auth_size
		0x40, 0x00, 0x00, 0x09,// TPM_RS_PW
		0x00, 0x00, 0x01, 0x00, 0x00,//SESSION Attribute

	
		 0x00, 0x00,//Password size of the NV_INDEX


	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,



  };

	//same as NV_READ and NV_WRITE	 (except it is stored elswhere)
	nv_index = (nv_index & 0xff) << 24 |  (nv_index & 0xff00) << 8 |  (nv_index & 0xff0000) >> 8 |  (nv_index & 0xff000000) >> 24 ;


	//The Password of the new NV_INDEX
	unsigned int password_size = strlen(password);
	
         uint16_t password_length = password_size;
         password_length =  (password_length & 0xff) << 8 |(password_length & 0xff00) >> 8; 



        *((uint16_t*)(tpm_request + 27)) = password_length;
	strcpy(tpm_request+29,password);
	//29+password_size

	//TPM2B_NV_PUBLIC
	*((uint16_t*)(tpm_request + 29 + password_size)) = 0x0e00;//The size if the TPM2B_NV_PUBLIC
	*((uint32_t*)(tpm_request + 31 + password_size)) = nv_index;//The NV index it is stored into the TPM2B_NV_PUBLIC structure
	*((uint16_t*)(tpm_request + 35 + password_size)) = 0x0b00;//Hash Algorithm SHA256
	*((uint32_t*)(tpm_request + 37 + password_size)) =  0x04000400; //(0x00040004 & 0xff) << 24 |  (0x00040004 & 0xff00) << 8 |  
	*((uint16_t*)(tpm_request + 41 + password_size)) = 0x0000;//Zero Policy
	*((uint16_t*)(tpm_request + 43 + password_size)) = 0x2000;//Data size (fixed to 16 bytes for the moment)



	


	//The command size is little bit longer than NV_READ since authentification and TPM2B_NV_PUBLIC shuld be stored
	uint32_t command_size = 45+password_size;
	command_size =  (command_size & 0xff) << 24 |  (command_size & 0xff00) << 8 |  (command_size & 0xff0000) >> 8 |  (command_size & 0xff000000) >> 24 ;
	*((uint32_t*)(tpm_request + 2)) = command_size;


	
	   //The data are sent and received from TPM the same as NV_READ and NV_WRITE
	   tpm_request_size=write(tpm_fd,tpm_request,password_size + 45); 
    tpm_response_size=read(tpm_fd,tpm_response_buffer,4096);

  int code = *( (uint32_t*) (tpm_response_buffer + 6) );






	close(tpm_fd);
   return PyLong_FromLong((long)code);


}


static PyMethodDef TpmMethods[] = {
	//List of Methods
    {"tpm_nv_read", tpm_nv_read, METH_FASTCALL, "Read data from TPM NV INDEX"},
    {"tpm_nv_write", tpm_nv_write, METH_FASTCALL, "Write data to TPM NV INDEX"},
    {"tpm_nv_define", tpm_nv_define, METH_FASTCALL, "Define TPM NV INDEX"},	
    {NULL, NULL, 0, NULL}//Null array is needed to indicate the end
};


static struct PyModuleDef tpm_module = {
    PyModuleDef_HEAD_INIT,
    "tpm",                  
    "TPM NV INDEX interraction", 
    -1,                     
    TpmMethods              
};


PyMODINIT_FUNC PyInit_tpm(void) {
    return PyModule_Create(&tpm_module);
}
