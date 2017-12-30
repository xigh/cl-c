#include <CL/cl.h>

struct device
{
    struct device *next;
    cl_uint pid;
    cl_uint did;
    cl_platform_id platform;
    cl_device_id device;
    cl_device_type type;
    char *name;
};

// clerror.c
char *getLastCLError();

// clenum.c
void freeCLDevices(struct device *d);
struct device *enumCLDevices();
