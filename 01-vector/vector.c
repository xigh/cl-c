// vector.c
//
// Do operations on vectors/matrices
//

// compile with: gcc -Wall -o vector vector.c ../common/clenum.c ../common/clerror.c -lOpenCL

#include <stdio.h>
#include "../common/clutil.h"

#define EXENAME "vector"

void CL_CALLBACK notify(const char *err_info, const void *private_info, size_t cb, void *user_data)
{
    struct device *d;

    d = (struct device *)user_data;
    fprintf(stderr, "%d.%d: received notification: %s\n", d->pid, d->did, err_info);
}

void testVector(struct device *d)
{
    cl_context ctx;
    cl_int err;

    ctx = clCreateContext(NULL, 1, &d->device, notify, d, &err);
    if (ctx == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateContext failed with %d\n", d->pid, d->did, err);
        return;
    }

    printf("%d.%d: context created\n", d->pid, d->did);
    // TODO: continue here

    err = clReleaseContext(ctx);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clReleaseContext failed with %d\n", d->pid, d->did, err);
        return;
    }
}

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

        testVector(d);
    }

    freeCLDevices(devices);
    return 0;
}
