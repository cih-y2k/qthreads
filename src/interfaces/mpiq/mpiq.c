#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "qthread/qthread.h"
#include "qt_envariables.h"

#include "mpiq.h"

static int use_choices           = 0;
static int thread_support_choice = 0;

int mpiq_policy(uint64_t const policy_flags)
{
    int rc = 0;

    use_choices = 1;

    /* Set MPI thread support policy */
    if (FLAG_POLICY_THREAD_SINGLE & policy_flags) {
        thread_support_choice = ID_POLICY_THREAD_SINGLE;
        fprintf(stderr, "Info: using THREAD_SINGLE policy\n");
    } else if (FLAG_POLICY_THREAD_FUNNELED & policy_flags) {
        thread_support_choice = ID_POLICY_THREAD_FUNNELED;
        fprintf(stderr, "Info: using THREAD_FUNNELED policy\n");
    } else if (FLAG_POLICY_THREAD_SERIALIZED & policy_flags) {
        thread_support_choice = ID_POLICY_THREAD_SERIALIZED;
        fprintf(stderr, "Info: using THREAD_SERIALIZED policy\n");
    } else if (FLAG_POLICY_THREAD_MULTIPLE & policy_flags) {
        thread_support_choice = ID_POLICY_THREAD_MULTIPLE;
        fprintf(stderr, "Info: using THREAD_MULTIPLE policy\n");
    }

    return rc;
}

int MPIQ_Init(int * argc, char *** argv)
{
    int rc = 0;
    int required;
    int provided;

    /* Choose thread support level. */
    const char * required_str = qt_internal_get_env_str("MPIQ_THREAD_SUPPORT", NULL);
    if (NULL != required_str) {
        /* Use setting in env. */
        if (0 == strcmp(required_str, "MPI_THREAD_SINGLE")) {
            required = MPI_THREAD_SINGLE;
        } else if (0 == strcmp(required_str, "MPI_THREAD_FUNNELED")) {
            required = MPI_THREAD_FUNNELED;
        } else if (0 == strcmp(required_str, "MPI_THREAD_SERIALIZED")) {
            required = MPI_THREAD_SERIALIZED;
        } else if (0 == strcmp(required_str, "MPI_THREAD_MULTIPLE")) {
            required = MPI_THREAD_MULTIPLE;
        } else {
            fprintf(stderr, "Error: unsupported thread level (MPIQ_THREAD_SUPORT=%s)\n", required_str);
            abort();
        }
    } else if (use_choices) {
        /* Use policy choice if nothing specified in env. */
        required = thread_support_choice;
    } else {
        /* Default to single. */
        required = MPI_THREAD_SINGLE;
    }

    rc = MPIQ_Init_thread(argc, argv, required, &provided);
    if (provided != required) {
        fprintf(stderr, "Error: thread support request not provided.\n", rc);
        abort();
    }

    return rc;
}

int MPIQ_Init_thread(int * argc, char *** argv, int required, int * provided)
{
    int rc = 0;

    if (use_choices && (thread_support_choice != required)) {
        fprintf(stderr, "Error: thread support policy does not match request (%d!=%d)\n", thread_support_choice, required);
        abort();
    }

    rc = MPI_Init_thread(argc, argv, required, provided);
    if (MPI_SUCCESS != rc) {
        fprintf(stderr, "Error: failed to initialize MPI (rc=%d)\n", rc);
        abort();
    }

    if (*provided < required) {
        fprintf(stderr, "Warning: thread support below required level (%d < %d)\n", *provided, required);
    }

    switch (*provided) {
    case MPI_THREAD_SINGLE:
        fprintf(stderr, "Info: using MPI_THREAD_SINGLE\n");
        qthread_init(1);
        if (1 != qthread_readstate(TOTAL_WORKERS)) {
            fprintf(stderr, "Error: Using more than one worker with MPI_THREAD_SINGLE.\n");
            abort();
        }
        break;
    case MPI_THREAD_FUNNELED:
        fprintf(stderr, "Info: using MPI_THREAD_FUNNELED\n");
        // TODO: initialize with 1 comm. resource.
        abort();
        break;
    case MPI_THREAD_SERIALIZED:
        fprintf(stderr, "Info: using MPI_THREAD_SERIALIZED\n");
        break;
        // TODO: initialize with k comm. resources.
        abort();
    case MPI_THREAD_MULTIPLE:
        fprintf(stderr, "Info: using MPI_THREAD_MULTIPLE\n");
        // TODO: initialize with k comm. resources.
        abort();
        break;
    default:
        fprintf(stderr, "Error: unknown MPI thread level (provided=%d)\n", *provided);
        abort();
    }

    return rc;
}

int MPIQ_Finalize(void)
{
    int rc;

    rc = MPI_Finalize();

    return rc;
}

int MPIQ_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    int rc;

    rc = MPI_Comm_dup(comm, newcomm);

    return rc;
}

int MPIQ_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler)
{
    int rc;

    rc = MPI_Errhandler_set(comm, errhandler);

    return rc;
}

int MPIQ_Comm_rank(MPI_Comm comm, int *rank)
{
    int rc;

    rc = MPI_Comm_rank(comm, rank);

    return rc;
}

int MPIQ_Comm_size(MPI_Comm comm, int *size)
{
    int rc;

    rc = MPI_Comm_size(comm, size);

    return rc;
}

int MPIQ_Barrier(MPI_Comm comm)
{
    int rc;

    rc = MPI_Barrier(comm);

    return rc;
}

int MPIQ_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    int rc;

    rc = MPI_Bcast(buffer, count, datatype, root, comm);

    return rc;
}

int MPIQ_Abort(MPI_Comm comm, int errorcode)
{
    int rc;

    rc = MPI_Abort(comm, errorcode);

    return rc;
}

int MPIQ_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    int rc;

    rc = MPI_Isend(buf, count, datatype, dest, tag, comm, request);

    return rc;
}

int MPIQ_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
    int rc;

    rc = MPI_Irecv(buf, count, datatype, source, tag, comm, request);

    return rc;
}

int MPIQ_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    int rc;

    rc = MPI_Send(buf, count, datatype, dest, tag, comm);

    return rc;
}

int MPIQ_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    int rc;

    rc = MPI_Recv(buf, count, datatype, source, tag, comm, status);

    return rc;
}

int MPIQ_Wait(MPI_Request *request, MPI_Status *status)
{
    int rc;

    rc = MPI_Wait(request, status);

    return rc;
}

int MPIQ_Waitany(int count, MPI_Request array_of_requests[], int *indx, MPI_Status *status)
{
    int rc;

    rc = MPI_Waitany(count, array_of_requests, indx, status);
    
    return rc;
}

int MPIQ_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[])
{
    int rc;

    rc = MPI_Waitall(count, array_of_requests, array_of_statuses);

    return rc;
}

int MPIQ_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    int rc;

    rc = MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);

    return rc;
}

int MPIQ_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    int rc;

    rc = MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);

    return rc;
}
