#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common_rcrtool.h"
#include "qt_rcrtool.h"
#include "bcGen.h"
#include "blackboard.h"

int rcrToolContinue = 1;
typedef struct _parallelRegion{
    void (*func) (void *);
    const char* funcName;
    unsigned int entryNum;
} parallelRegion;

#define LATENCYTEST 10000

#define AMD_OPTERON

/*!
 * Complain with perror, then exit.
 * 
 * \param msg Nature of the problem (don't include newline)
 */
void die(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/*!
 * 
 */
void doWork(int nshepherds, int nworkerspershep) {
    struct timespec interval, remainder;
    interval.tv_sec = 0;
    interval.tv_nsec = 5000000;

    while (rcrToolContinue) {

        struct _RCRBlackboard* bb = (struct _RCRBlackboard*)getShmBlackboardRO();
        if (!bb) {
            //error
            printf("shmdt failed. Can not get BB Shared Memory.\n");
            return;
        } else {
            struct RCRNode*   firstNode   = (struct RCRNode*)  (&bb[1]);
            struct RCRSocket* firstSocket = (struct RCRSocket*)(&firstNode[bb->numOfNodes]);
            struct RCRCore*   firstCore   = (struct RCRCore*)  (&firstSocket[bb->numOfSockets]);
            struct RCRMeter*  firstMeter  = (struct RCRMeter*) (&firstCore[bb->numOfCores]);
            struct RCRMeter*  socketMeter = firstMeter;
            struct RCRMeter*  coreMeter   = &(firstMeter[bb->numOfSockets * S_NUMBER_OF_SOCKET_METERS]);

            int sm, cm;
            for (sm = 0; sm < bb->numOfSockets * S_NUMBER_OF_SOCKET_METERS; sm++) {
                if (socketMeter[sm].enable) {
                    if (socketMeter[sm].current < socketMeter[sm].lBound) {
                        throwTrigger(APPSTATESHMKEY, T_TYPE_SOCKET, T_TYPE_LOW, sm / S_NUMBER_OF_SOCKET_METERS, sm);
                        //printf("Blown socket Trigger. %f %f %f!\n", socketMeter[sm].lBound, socketMeter[sm].current, socketMeter[sm].uBound);
                    }
                    if (socketMeter[sm].current > socketMeter[sm].uBound) {
                        throwTrigger(APPSTATESHMKEY, T_TYPE_SOCKET, T_TYPE_HIGH, sm / S_NUMBER_OF_SOCKET_METERS, sm);
                        //printf("Blown socket Trigger. %f %f %f!\n", socketMeter[sm].lBound, socketMeter[sm].current, socketMeter[sm].uBound);
                    }
                }
            }
            for (cm = 0; cm < bb->numOfCores * C_NUMBER_OF_CORE_METERS; cm++) {
                if (coreMeter[cm].enable) {
                    if (coreMeter[cm].current < coreMeter[cm].lBound) {
                        throwTrigger(APPSTATESHMKEY, T_TYPE_CORE, T_TYPE_LOW, cm / C_NUMBER_OF_CORE_METERS, sm + cm);
                        //printf("Blown core Trigger! %f %f %f\n", coreMeter[cm].lBound, coreMeter[cm].current, coreMeter[cm].uBound);
                    }
                    if (coreMeter[cm].current > coreMeter[cm].uBound) {
                        throwTrigger(APPSTATESHMKEY, T_TYPE_CORE, T_TYPE_HIGH, cm / C_NUMBER_OF_CORE_METERS, sm + cm);
                        //printf("Blown core Trigger! %f %f %f\n", coreMeter[cm].lBound, coreMeter[cm].current, coreMeter[cm].uBound);
                    }
                }
            }

            if(shmdt(bb) == -1){
                printf("shmdt failed\n");
                return;
            }
        }

#ifdef QTHREAD_RCRTOOL
        int dummySocketOrCoreID = 0;
        dumpAppState(APPSTATESHMKEY, T_TYPE_SOCKET, dummySocketOrCoreID);
#endif
        nanosleep(&interval, &remainder);
    }
    clearAppState();
}


