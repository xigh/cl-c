#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clutil.h"

void freeCLDevices(struct device *d)
{
    struct device *n;

    while (d != NULL)
    {
        n = d->next;
        // free d->XXX
        free(d);
        d = n;
    }
}

void setLastCLError(char *fmt, ...);
char *getCLPlateformString(cl_platform_id id, cl_platform_info info);
char *getCLDeviceString(cl_device_id id, cl_device_info info);

struct device *enumCLDevices()
{
    struct device *devices;
    cl_uint np, nps, nd, nds;
    cl_platform_id *pids;
    cl_device_id *dids;
    cl_int err;

    err = clGetPlatformIDs(0, NULL, &nps);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetPlatformIDs failed with %d\n", err);
        return NULL;
    }

    if (nps == 0)
    {
        setLastCLError("OpenCL not found\n");
        return NULL;
    }

    pids = (cl_platform_id *)malloc(nps * sizeof(*pids));
    if (pids == NULL)
    {
        setLastCLError("Could not allocate memory [platform ids]\n");
        return NULL;
    }

    err = clGetPlatformIDs(nps, pids, NULL);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetPlatformIDs failed with %d\n", err);
        return NULL;
    }

    devices = NULL;
    for (np = 0; np < nps; np += 1)
    {
        err = clGetDeviceIDs(pids[np], CL_DEVICE_TYPE_ALL, 0, NULL, &nds);
        if (err != CL_SUCCESS || nds == 0)
        {
            // no device
            continue;
        }

        dids = (cl_device_id *)malloc(nds * sizeof(*dids));
        if (dids == NULL)
        {
            setLastCLError("Could not allocate memory [device ids]\n");
            return NULL;
        }

        err = clGetDeviceIDs(pids[np], CL_DEVICE_TYPE_ALL, nds, dids, NULL);
        if (err != CL_SUCCESS)
        {
            setLastCLError("clGetDeviceIDs failed with %d\n", err);
            return NULL;
        }

        for (nd = 0; nd < nds; nd += 1)
        {
            struct device *d;

            d = (struct device *)malloc(sizeof(*d));
            if (d == NULL)
            {
                setLastCLError("Could not allocate memory [device entry]\n");
                continue;
            }

            d->next = devices;
            d->pid = np;
            d->did = nd;
            d->platform = pids[np];
            d->device = dids[nd];

            // TODO: get more information (name, type, memory, extensions, ...)

            err = clGetDeviceInfo(d->device, CL_DEVICE_TYPE, sizeof(d->type), &d->type, NULL);
            if (err != CL_SUCCESS)
            {
                setLastCLError("clGetDeviceInfo(CL_DEVICE_TYPE, %d) failed with %d\n", nd, err);
                free(d);
                continue;
            }

            d->name = getCLDeviceString(d->device, CL_DEVICE_NAME);
            if (d->name == NULL)
            {
                free(d);
                continue;
            }

            devices = d;
        }
    }

    return devices;
}

char *getCLPlateformString(cl_platform_id id, cl_platform_info info)
{
    cl_int err;
    size_t sz;
    char *s;

    err = clGetPlatformInfo(id, info, 0, NULL, &sz);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetPlatformInfo(%x, 0) failed with %d\n", info, err);
        return NULL;
    }

    s = (char *)malloc(sz);
    if (s == NULL)
    {
        setLastCLError("Could not allocate memory [name]\n");
        return NULL;
    }

    err = clGetPlatformInfo(id, info, sz, s, NULL);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetPlatformInfo(%x, %d) failed with %d\n", info, (int)sz, err);
        return NULL;
    }

    return s;
}

char *getCLDeviceString(cl_device_id id, cl_device_info info)
{
    cl_int err;
    size_t sz;
    char *s;

    err = clGetDeviceInfo(id, info, 0, NULL, &sz);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetDeviceInfo(%x, 0) failed with %d\n", info, err);
        return NULL;
    }

    s = (char *)malloc(sz);
    if (s == NULL)
    {
        setLastCLError("Could not allocate memory [name]\n");
        return NULL;
    }

    err = clGetDeviceInfo(id, info, sz, s, NULL);
    if (err != CL_SUCCESS)
    {
        setLastCLError("clGetDeviceInfo(%x, %d) failed with %d\n", info, (int)sz, err);
        return NULL;
    }

    return s;
}
