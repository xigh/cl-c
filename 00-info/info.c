// info.c
//
// Query basic opencl information from the system.
// If you need more, use clinfo 
//

// compile with: gcc -Wall -o info info.c -lOpenCL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

#define EXENAME "info"

char *getPlateformString(cl_platform_id id, cl_platform_info info)
{
    cl_int err;
    size_t sz;
    char *s;

    err = clGetPlatformInfo(id, info, 0, NULL, &sz);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetPlatformInfo(%x, 0) failed with %d\n", info, err);
        return NULL;
    }

    s = (char *)malloc(sz);
    if (s == NULL)
    {
        fprintf(stderr, EXENAME ": Could not allocate memory [name]\n");
        return NULL;
    }

    err = clGetPlatformInfo(id, info, sz, s, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetPlatformInfo(%x, %d) failed with %d\n", info, (int)sz, err);
        return NULL;
    }

    return s;
}

char *getDeviceString(cl_device_id id, cl_device_info info)
{
    cl_int err;
    size_t sz;
    char *s;

    err = clGetDeviceInfo(id, info, 0, NULL, &sz);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetDeviceInfo(%x, 0) failed with %d\n", info, err);
        return NULL;
    }

    s = (char *)malloc(sz);
    if (s == NULL)
    {
        fprintf(stderr, EXENAME ": Could not allocate memory [name]\n");
        return NULL;
    }

    err = clGetDeviceInfo(id, info, sz, s, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetDeviceInfo(%x, %d) failed with %d\n", info, (int)sz, err);
        return NULL;
    }

    return s;
}

int main(int argc, char **argv)
{
    cl_int err;
    cl_uint np, nps, nd, nds;
    cl_platform_id *platforms;
    cl_device_id *devices;

    err = clGetPlatformIDs(0, NULL, &nps);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetPlatformIDs failed with %d\n", err);
        return -1;
    }

    if (nps == 0)
    {
        fprintf(stderr, EXENAME ": OpenCL not found\n");
        return -1;
    }

    platforms = (cl_platform_id *)malloc(nps * sizeof(*platforms));
    if (platforms == NULL)
    {
        fprintf(stderr, EXENAME ": Could not allocate memory [platforms]\n");
        return -1;
    }

    err = clGetPlatformIDs(nps, platforms, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, EXENAME ": clGetPlatformIDs(%d) failed with %d\n", nps, err);
        return -1;
    }

    for (np = 0; np < nps; np += 1)
    {
        char *name, *version;
        
        name = getPlateformString(platforms[np], CL_PLATFORM_NAME);
        if (name == NULL)
        {
            continue;
        }
        printf("%d: %s\n", np, name);
        free(name);

        version = getPlateformString(platforms[np], CL_PLATFORM_VERSION);
        if (version != NULL)
        {
            printf("   version=%s\n", version);
            free(version);
        }

        err = clGetDeviceIDs(platforms[np], CL_DEVICE_TYPE_ALL, 0, NULL, &nds);
        if (err != CL_SUCCESS || nds == 0)
        {
            printf("   no device\n\n");
            continue;
        }

        devices = (cl_device_id *)malloc(nds * sizeof(*devices));
        if (devices == NULL)
        {
            fprintf(stderr, EXENAME ": Could not allocate memory [devices]\n");
            return -1;
        }

        err = clGetDeviceIDs(platforms[np], CL_DEVICE_TYPE_ALL, nds, devices, NULL);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, EXENAME ": clGetDeviceIDs failed with %d\n", err);
            return -1;
        }
        
        for (nd = 0; nd < nds; nd += 1) {
            char *dname, *dstype;
            cl_device_type dtype;
            cl_uint cunits;
            cl_ulong memsize;
            
            dname = getDeviceString(devices[nd], CL_DEVICE_NAME);
            if (dname == NULL)
            {
                continue;
            }

            err = clGetDeviceInfo(devices[nd], CL_DEVICE_TYPE, sizeof(dtype), &dtype, NULL);
            if (err != CL_SUCCESS)
            {
                fprintf(stderr, EXENAME ": clGetDeviceInfo(CL_DEVICE_TYPE, %d) failed with %d\n", nd, err);
                return -1;
            }
            
            dstype = "unknown";
            switch (dtype) {
            case CL_DEVICE_TYPE_CPU:
                dstype = "cpu";
                break;
            case CL_DEVICE_TYPE_GPU:
                dstype = "gpu";
                break;
            case CL_DEVICE_TYPE_ACCELERATOR:
                dstype = "accel";
                break;
            }

            printf("   %d: %s [%s]\n", nd, dname, dstype);
            free(dname);

            err = clGetDeviceInfo(devices[nd], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cunits), &cunits, NULL);
            if (err != CL_SUCCESS)
            {
                fprintf(stderr, EXENAME ": clGetDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS, %d) failed with %d\n", nd, err);
                continue;
            }
            
            printf("      %d compute units\n", cunits);

            err = clGetDeviceInfo(devices[nd], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(memsize), &memsize, NULL);
            if (err != CL_SUCCESS)
            {
                fprintf(stderr, EXENAME ": clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_SIZE, %d) failed with %d\n", nd, err);
                continue;
            }
            
            if (memsize > 1024l*1024l*1024l) {
                printf("      global memory size: %.1fGB\n", ((float) memsize) / (1024.0 * 1024.0 * 1024.0));
            }
            else if (memsize > 1024l*1024l) {
                printf("      global memory size: %.1fMB\n", ((float) memsize) / (1024.0 * 1024.0));
            }
            else if (memsize > 1024) {
                printf("      global memory size: %.1fkB\n", ((float) memsize) / 1024.0);
            }
            else {
                printf("      global memory size: %ld\n", memsize);                
            }            
        }
    }

    return 0;
}
