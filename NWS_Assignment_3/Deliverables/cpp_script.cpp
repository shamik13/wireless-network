#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;

#define uint unsigned int
#define uplink 0
#define downlink 1
#define withRtsCts 0
#define woRtsCts 1

#define RNGRUNS 5
#define TOTALFLOWS 5

/* Structure for maintaining Data-set for particular characteristics */
typedef struct data {
    uint flow;
    float ulrc, ulwrc, dlrc, dlwrc;
} dataset;

/* Merge Characteristic Data collected into it's specific Data set */
void mergeInPlt(FILE *fp, char *fileName, dataset *data, bool uldlflag, bool rtsctsflag)
{
    uint i,j,k, flows[] = {1,5,10,20,30};
    float arr[RNGRUNS*TOTALFLOWS], sum;
    char fl[32], str[32];

    freopen(fileName, "r", stdin);

    // Read Input
    for(i=0;i<TOTALFLOWS*RNGRUNS;i++)
    {
        scanf("%s %s\n", fl, str);
        arr[i] = atof(str);
    }

    // Averaging
    for(i=0,j=0; i<TOTALFLOWS;i++)
    {
        for(sum=0, k=0; k<RNGRUNS; k++,j++)
            sum += arr[j];
    
        printf("Avg: %u -> |%f|\n", flows[i], (float)sum/RNGRUNS);

        //Uplink Traffic
        if(uldlflag == uplink)
        {
            if(rtsctsflag == withRtsCts)
                data[i].ulrc = sum/RNGRUNS;
            else if(rtsctsflag == woRtsCts)
                data[i].ulwrc = sum/RNGRUNS;
        }
        // Downlink Traffic
        else if(uldlflag == downlink)
        {
            if(rtsctsflag == withRtsCts)
                data[i].dlrc = sum/RNGRUNS;
            else if(rtsctsflag == woRtsCts)
                data[i].dlwrc = sum/RNGRUNS;
        }        
    }
}

void mergePlt(char *dataSetName, char* file1, char* file2, char* file3, char* file4)
{
    FILE *fp;
    dataset data[TOTALFLOWS];
    uint flows[] = {1,5,10,20,30};
    uint i,j;

    for(i=0;i<TOTALFLOWS;i++)
        data[i].flow = flows[i];

    if((fp=fopen(dataSetName, "w"))==NULL)
    {
        printf("Cannot open collision_dataSet.txt. Exiting!\n");
        exit(0);
    }

    // Merge the data-files into one-dataSet
    mergeInPlt(fp, file1, data, uplink,   woRtsCts);
    mergeInPlt(fp, file2, data, uplink,   withRtsCts);
    mergeInPlt(fp, file3, data, downlink, woRtsCts);
    mergeInPlt(fp, file4, data, downlink, withRtsCts);

    for(i=0,j=1;i<TOTALFLOWS;i++)
        fprintf(fp, "%u %f %f %f %f\n", data[i].flow, data[i].ulwrc, data[i].ulrc, data[i].dlwrc, data[i].dlrc);

    fclose(fp);
}

int main()
{
    mergePlt((char*)"throughput_dataSet.txt", (char*)"all_sim_th_ul_wo_rts_cts.txt", (char*)"all_sim_th_ul_rts_cts.txt", (char*)"all_sim_th_dl_wo_rts_cts.txt", (char*)"all_sim_th_dl_rts_cts.txt");
    mergePlt((char*)"collision_dataSet.txt", (char*)"all_sim_col_ul_wo_rts_cts.txt", (char*)"all_sim_col_ul_rts_cts.txt", (char*)"all_sim_col_dl_wo_rts_cts.txt", (char*)"all_sim_col_dl_rts_cts.txt");
    mergePlt((char*)"drop_dataSet.txt", (char*)"all_sim_drp_ul_wo_rts_cts.txt", (char*)"all_sim_drp_ul_rts_cts.txt", (char*)"all_sim_drp_dl_wo_rts_cts.txt", (char*)"all_sim_drp_dl_rts_cts.txt");

    return 0;
}
