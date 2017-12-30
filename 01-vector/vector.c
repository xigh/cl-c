// vector.c
//
// Do operations on vectors/matrices
//

// compile with: gcc -Wall -o vector vector.c ../common/clenum.c ../common/clerror.c -lOpenCL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../common/clutil.h"

#define EXENAME     "vector"
#define VEC_SIZE    (100 * 1024 * 1024)

const char *kernel_add = "__kernel void vAdd(__global const float* a, __global const float* b,"
                         "                   __global float* c, const unsigned int n)"
                         "{"
                         "   int i = get_global_id(0);"
                         "   if (i < n)"
                         "   {"
                         "       c[i] = a[i] + b[i];"
                         "   }"
                         "}";

struct data
{
    unsigned int size;

    float *buf0;
    float *buf1;
    float *buf2;

    cl_mem mem0;
    cl_mem mem1;
    cl_mem mem2;

    cl_context ctx;
    cl_program prog;
    cl_kernel kern;
    cl_event evt;
    cl_command_queue queue;
};

int testVectorStep3(struct device *d, struct data *x)
{
    size_t len;
    cl_int err;
    size_t size;
    cl_ulong start, end;
    int ok;

    ok = 0;
    len = strlen(kernel_add);

    x->prog = clCreateProgramWithSource(x->ctx, 1, &kernel_add, &len, &err);
    if (x->prog == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateProgramWithSource failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clBuildProgram(x->prog, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clBuildProgram failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    x->kern = clCreateKernel(x->prog, "vAdd", &err);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clCreateKernel failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clSetKernelArg(x->kern, 0, sizeof(cl_mem), &x->mem0);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clSetKernelArg[0] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clSetKernelArg(x->kern, 1, sizeof(cl_mem), &x->mem1);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clSetKernelArg[1] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clSetKernelArg(x->kern, 2, sizeof(cl_mem), &x->mem2);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clSetKernelArg[2] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clSetKernelArg(x->kern, 3, sizeof(unsigned int), &x->size);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clSetKernelArg[3] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    x->queue = clCreateCommandQueue(x->ctx, d->device, CL_QUEUE_PROFILING_ENABLE, &err);
    if (x->queue == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateCommandQueue failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    // async write
    err = clEnqueueWriteBuffer(x->queue, x->mem0, CL_FALSE, 0,
                               sizeof(cl_float) * x->size, x->buf0, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clEnqueueWriteBuffer[mem0] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem0 uploaded\n", d->pid, d->did);

    // async write
    err = clEnqueueWriteBuffer(x->queue, x->mem1, CL_FALSE, 0,
                               sizeof(cl_float) * x->size, x->buf1, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clEnqueueWriteBuffer[mem1] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem1 uploaded\n", d->pid, d->did);

    size = (size_t)x->size;

    err = clEnqueueNDRangeKernel(x->queue, x->kern, 1, NULL, &size, NULL, 0, NULL, &x->evt);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clEnqueueNDRangeKernel failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: prog kernel queued\n", d->pid, d->did);

    err = clFinish(x->queue);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clEnqueueNDRangeKernel failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: prog kernel finished\n", d->pid, d->did);

    // block read
    err = clEnqueueReadBuffer(x->queue, x->mem2, CL_TRUE, 0,
                              sizeof(cl_float) * x->size, x->buf2, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clEnqueueReadBuffer[mem2] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem2 downloaded\n", d->pid, d->did);

    err = clWaitForEvents(1, &x->evt);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clWaitForEvents failed with %d\n", d->pid, d->did, err);
        goto error;
    }
    
    err = clGetEventProfilingInfo(x->evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clGetEventProfilingInfo[start] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    err = clGetEventProfilingInfo(x->evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    if (err != CL_SUCCESS)
    {
        fprintf(stderr, "%d.%d: clGetEventProfilingInfo[end] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: duration: %g seconds\n", d->pid, d->did, (float)(end - start) / 1e9f);

    ok = 1;

error:
    if (x->evt) {
        err = clReleaseEvent(x->evt);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseEvent failed with %d\n", d->pid, d->did, err);
        }
    }

    if (x->queue)
    {
        err = clReleaseCommandQueue(x->queue);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseCommandQueue failed with %d\n", d->pid, d->did, err);
            goto error;
        }
    }

    if (x->kern)
    {
        err = clReleaseKernel(x->kern);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseKernel failed with %d\n", d->pid, d->did, err);
            goto error;
        }
    }

    if (x->prog)
    {
        err = clReleaseProgram(x->prog);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseProgram failed with %d\n", d->pid, d->did, err);
            goto error;
        }
    }

    return ok;
}

int testVectorStep2(struct device *d, struct data *x)
{
    cl_int err;
    int ok;

    ok = 0;

    x->mem0 = clCreateBuffer(x->ctx, CL_MEM_READ_ONLY, sizeof(cl_float) * x->size, NULL, &err);
    if (x->mem0 == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateBuffer[mem0] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem0 buffer allocated\n", d->pid, d->did);

    x->mem1 = clCreateBuffer(x->ctx, CL_MEM_READ_ONLY, sizeof(cl_float) * x->size, NULL, &err);
    if (x->mem1 == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateBuffer[mem1] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem1 buffer allocated\n", d->pid, d->did);

    x->mem2 = clCreateBuffer(x->ctx, CL_MEM_READ_ONLY, sizeof(cl_float) * x->size, NULL, &err);
    if (x->mem2 == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateBuffer[mem2] failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: mem2 buffer allocated\n", d->pid, d->did);

    ok = testVectorStep3(d, x);

error:
    if (x->mem2 != NULL)
    {
        err = clReleaseMemObject(x->mem2);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseMemObject(mem2) failed with %d\n", d->pid, d->did, err);
        }
    }

    if (x->mem1 != NULL)
    {
        err = clReleaseMemObject(x->mem1);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseMemObject(mem1) failed with %d\n", d->pid, d->did, err);
        }
    }

    if (x->mem0 != NULL)
    {
        err = clReleaseMemObject(x->mem0);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseMemObject(mem0) failed with %d\n", d->pid, d->did, err);
        }
    }

    return ok;
}

void testVectorStep1(struct device *d)
{
    struct data x;
    cl_int err;
    struct timespec start, end;
    float dur;
    int i;

    memset(&x, 0, sizeof(x));

    x.size = VEC_SIZE;

    x.buf0 = (float *)malloc(x.size * sizeof(float));
    if (x.buf0 == NULL)
    {
        fprintf(stderr, "Could not allocate memory [x.buf0]\n");
        goto error;
    }

    x.buf1 = (float *)malloc(x.size * sizeof(float));
    if (x.buf1 == NULL)
    {
        fprintf(stderr, "Could not allocate memory [x.buf1]\n");
        goto error;
    }

    printf("%d:%d: generating source buffer (rand)\n", d->pid, d->did);

    for (i = 0; i < x.size; i += 1)
    {
        x.buf0[i] = (float)rand() / (float)RAND_MAX;
        x.buf1[i] = (float)rand() / (float)RAND_MAX;
    }

    x.buf2 = (float *)malloc(x.size * sizeof(float));
    if (x.buf2 == NULL)
    {
        fprintf(stderr, "Could not allocate memory [x.buf2]\n");
        goto error;
    }

    x.ctx = clCreateContext(NULL, 1, &d->device, NULL, NULL, &err);
    if (x.ctx == NULL)
    {
        fprintf(stderr, "%d.%d: clCreateContext failed with %d\n", d->pid, d->did, err);
        goto error;
    }

    printf("%d.%d: context created\n", d->pid, d->did);
    if (testVectorStep2(d, &x) != 0)
    {
        float *buf;

        buf = (float *) malloc(sizeof(float) * x.size);
        if (buf == 0) {
            fprintf(stderr, "Could not allocate memory [buf]\n");
            goto error;
        }

        printf("%d.%d: computing vector addition with CPU\n", d->pid, d->did);

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

        for (i = 0; i < x.size; i += 1)
        {
            buf[i] = x.buf0[i] + x.buf1[i];
        }

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	    if ((end.tv_nsec-start.tv_nsec)<0) {
		    dur = (float) (end.tv_sec-start.tv_sec-1) + 
                (float) (1000000000+end.tv_nsec-start.tv_nsec) / 1e9;
	    } else {
		    dur = (float) (end.tv_sec-start.tv_sec) +
		        (float) (end.tv_nsec-start.tv_nsec) / 1e9;
	    }

        printf("%d.%d: vectors added in %g seconds\n", d->pid, d->did, dur);

        printf("%d.%d: comparing results ...\n", d->pid, d->did);

        for (i = 0; i < x.size; i += 1)
        {
            if (x.buf2[i] != buf[i])
            {
                printf("%d.%d: check error at %d: %f + %f != %f\n",
                       d->pid, d->did, i, x.buf0[i], x.buf1[i], x.buf2[i]);
                goto error;
            }
        }

        printf("%d.%d: check ok: added %d floats\n", d->pid, d->did, x.size);
    }

error:
    if (x.ctx)
    {
        err = clReleaseContext(x.ctx);
        if (err != CL_SUCCESS)
        {
            fprintf(stderr, "%d.%d: clReleaseContext failed with %d\n", d->pid, d->did, err);
        }
    }

    if (x.buf2)
    {
        free(x.buf2);
    }

    if (x.buf1)
    {
        free(x.buf1);
    }

    if (x.buf0)
    {
        free(x.buf0);
    }
}

int main(int argc, char **argv)
{
    struct device *devices, *d;

    srand(1);

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

        testVectorStep1(d);
    }

    freeCLDevices(devices);
    return 0;
}
