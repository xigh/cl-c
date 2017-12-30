// vector.c
//
// Do operations on vectors/matrices
//

// compile with: gcc -Wall -o vector vector.c ../common/clenum.c ../common/clerror.c -lOpenCL

#include <stdio.h>
#include "../common/clutil.h"

#define EXENAME "vector"

int main(int argc, char **argv)
{
    struct device *devices, *d;

    devices = enumCLDevices();
    if (devices == NULL)
    {
        fprintf(stderr, EXENAME ": no opencl device found\n");
        fprintf(stderr, "\t%s\n", getLastCLError());
        return -1;
    }

    for (d = devices; d != NULL; d = d->next)
    {
        char *dtype;

        dtype = "unknown";
        switch (d->type)
        {
        case CL_DEVICE_TYPE_CPU:
            dtype = "cpu";
            break;
        case CL_DEVICE_TYPE_GPU:
            dtype = "gpu";
            break;
        case CL_DEVICE_TYPE_ACCELERATOR:
            dtype = "accel";
            break;
        }

        printf("%d.%d: %s [%s]\n", d->pid, d->did, d->name, dtype);
    }

    freeCLDevices(devices);
    return 0;
}
