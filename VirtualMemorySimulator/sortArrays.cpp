#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_CHAR 30
#define THREAD_NUM 4
#define SEED 1000
using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct PageEntry
{
    int pageFrameNum;
    int present;
    int modified;
    int referenced;
    int* g_fifo;
    int* l_fifo;
} PageEntry;

// File Streams
ofstream fd;
ifstream in;


int* physicalMem;

// variables for LRU algorithm
int* lruArr;
int lruCounter=1;
int lruCounterP1 = 1;
int lruCounterP2 = 1;
int lruCounterP3 = 1;
int lruCounterP4 = 1;

// Input size
int physicalMemSize;
int virtualMemSize;
int virtualFrame;
int physicalFrame;
int frameSize;
int pageTableSize;
int totalFrame;

// variable for NRU algorithm
int countNru = 0;
int countNruP1 = 0;
int countNruP2 = 0;
int countNruP3 = 0;
int countNruP4 = 0;

// check input
bool local;
bool fifo = false;
bool lru = false;
bool nru = false;
bool sch = false;

//number for prints statistics
int numberOfReadsF = 0;
int numberOfWritesF = 0;
int numberOfPageMissF = 0;
int numberOfPageRepF = 0;
int numberOfPageWriteF = 0;
int numberOfPageReadsF = 0;

int numberOfReadsB = 0;
int numberOfWritesB = 0;
int numberOfPageMissB = 0;
int numberOfPageRepB = 0;
int numberOfPageWriteB = 0;
int numberOfPageReadsB = 0;

int numberOfReadsQ = 0;
int numberOfWritesQ = 0;
int numberOfPageMissQ = 0;
int numberOfPageRepQ = 0;
int numberOfPageWriteQ = 0;
int numberOfPageReadsQ = 0;

int numberOfReadsM = 0;
int numberOfWritesM = 0;
int numberOfPageMissM = 0;
int numberOfPageRepM = 0;
int numberOfPageWriteM = 0;
int numberOfPageReadsM = 0;

int numberOfReadsI = 0;
int numberOfWritesI = 0;
int numberOfPageMissI = 0;
int numberOfPageRepI = 0;
int numberOfPageWriteI = 0;
int numberOfPageReadsI = 0;

int numberOfReadsC = 0;
int numberOfWritesC = 0;
int numberOfPageMissC = 0;
int numberOfPageRepC = 0;
int numberOfPageWriteC = 0;
int numberOfPageReadsC = 0;

int accessNumPT = 0;

int pageTablePrintInt;
PageEntry* pageTable;

void displayUsage();
void clean_up();
void initVirtualMem(char* diskName);
void initPhysicalMem();
void initPageTable();
int findFifoPage(int memoryframe);
void make_replacement(int diskIndex, int pindex);
void writeDisk(int index, int value);
void diskFToMemoryF(int pindex, int base);
void quickSort( int left, int right);
int partition(int left, int right);
int findNruBackupMem(const char* tName);


void mergeSort(int l, int r);
void merge(int l, int m, int r);
void indexSort(int low_ind, int up_ind);

void set(unsigned int index, int value,const char * tName);
int get(unsigned int index,const char * tName);
void* sortVirtualArray(void* arg);


void printPageTable();

int main(int argc, char** argv){
    int numPhysical, numVirtual ;
    char pageRplcAlgo[MAX_CHAR], allocPolicy[MAX_CHAR], diskName[MAX_CHAR];

    if(argc != 8){
        displayUsage();
    } 

    //Get Input
    frameSize = atoi(argv[1]);
    numPhysical = atoi(argv[2]);
    numVirtual = atoi(argv[3]);
    strcpy(pageRplcAlgo,argv[4]);
    strcpy(allocPolicy,argv[5]);
    pageTablePrintInt = atoi(argv[6]);
    strcpy(diskName,argv[7]);

    // Calculate Size
    totalFrame = pow(2,frameSize);
    physicalFrame = pow(2,numPhysical);
    virtualFrame = pow(2,numVirtual);
    physicalMemSize = totalFrame*physicalFrame;
    virtualMemSize = totalFrame*virtualFrame;
    pageTableSize = virtualFrame;

    //Allocate memory
    pageTable = (PageEntry*) malloc(sizeof(PageEntry)*virtualFrame);
    physicalMem = (int*)  malloc(sizeof(int)*physicalMemSize);
    lruArr = (int*)  malloc(sizeof(int)*physicalFrame);
    fd.open(diskName,ios::binary);
    in.open(diskName, ios::binary );
    srand(SEED);
    if(strcmp(allocPolicy,"local") == 0){
        local = true;
    }
    else{
        local = false;
    }

    if(strcmp(pageRplcAlgo,"FIFO") == 0){
        fifo = true;
    }
    else if(strcmp(pageRplcAlgo,"LRU") == 0){
        lru = true;
    }
    else if(strcmp(pageRplcAlgo,"NRU") == 0){
        nru = true;
    }
    else if(strcmp(pageRplcAlgo,"SC") == 0){
        fifo = true;
    }
    else if(strcmp(pageRplcAlgo,"WSClock") == 0){
        fifo = true;
    }

    //Init pt
    initPageTable();


    // Fill Virtual Memory
    for (int i = 0; i < virtualMemSize; i++)
    {
        int num = rand();
        set(i,num,"fill");
    }

    initPhysicalMem();

    pthread_t threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++)
    {
        int s = pthread_create(&threads[i],NULL,&sortVirtualArray,(void*) i);
        if(s != 0){
            cout << "Create Thread Error" << endl;
            exit(1);
        }
    }

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    bool flag = true;
    int temp = INT_MIN;
    for (int i = 0; i < virtualMemSize/4; i++)
    {
        int val = get(i,"check");
        if(val < temp){
            cout << "Bubble is not sorted" << endl;
            flag = false;
            break;
        }
        temp = val;
    }
     if(flag){
        cout << "bubble is sorted" << endl;
    }  
    flag = true;
    temp = INT_MIN;
    for (int i = virtualMemSize/4; i < virtualMemSize/2; i++)
    {
        int val = get(i,"check");
        if(val < temp){
            cout << "quick is not sorted" << endl;
            flag = false;
            break;
        }
        temp = val;
    }
    if(flag){
        cout << "quick is sorted" << endl;
    }
    flag = true;
    temp = INT_MIN;
    for (int i = 2*(virtualMemSize/4); i < 3*(virtualMemSize/4); i++)
    {
        int val = get(i,"check");
        if(val < temp){
            cout << "merge is not sorted" << endl;
            flag = false;
            break;
        }
        temp = val;
    }
    if(flag){
        cout << "merge is sorted" << endl;
    }
    flag = true;
    temp = INT_MIN;
    for (int i = 3*(virtualMemSize/4); i < virtualMemSize; i++)
    {
        int val = get(i,"check");
        if(val < temp){
            cout << "index is not sorted" << endl;
            flag = false;
            break;
        }
        temp = val;
    }
    if(flag){
        cout << "index is sorted" << endl;
    }

    cout << "######### FILL ###################" << endl;
    cout << " Number of reads: " << numberOfReadsF << endl;
    cout << " Number of writes: " << numberOfWritesF << endl;
    cout << " Number of page misses: " << numberOfPageMissF << endl;
    cout << " Number of page replacment: " << numberOfPageRepF << endl;
    cout << " Number of disk page write: " << numberOfPageWriteF << endl;
    cout << " Number of disk page read: " << numberOfPageReadsF << endl;

    cout << "######### QUICK SORT ###################" << endl;
    cout << " Number of reads: " << numberOfReadsQ << endl;
    cout << " Number of writes: " << numberOfWritesQ << endl;
    cout << " Number of page misses: " << numberOfPageMissQ << endl;
    cout << " Number of page replacment: " << numberOfPageRepQ << endl;
    cout << " Number of disk page write: " << numberOfPageWriteQ << endl;
    cout << " Number of disk page read: " << numberOfPageReadsQ << endl;

    cout << "######### BUBBLE SORT ###################" << endl;
    cout << " Number of reads: " << numberOfReadsB << endl;
    cout << " Number of writes: " << numberOfWritesB << endl;
    cout << " Number of page misses: " << numberOfPageMissB << endl;
    cout << " Number of page replacment: " << numberOfPageRepB << endl;
    cout << " Number of disk page write: " << numberOfPageWriteB << endl;
    cout << " Number of disk page read: " << numberOfPageReadsB << endl;



    cout << "######### MERGE SORT ###################" << endl;
    cout << " Number of reads: " << numberOfReadsM << endl;
    cout << " Number of writes: " << numberOfWritesM << endl;
    cout << " Number of page misses: " << numberOfPageMissM << endl;
    cout << " Number of page replacment: " << numberOfPageRepM << endl;
    cout << " Number of disk page write: " << numberOfPageWriteM << endl;
    cout << " Number of disk page read: " << numberOfPageReadsM << endl;

    cout << "######### INDEX SORT ###################" << endl;
    cout << " Number of reads: " << numberOfReadsI << endl;
    cout << " Number of writes: " << numberOfWritesI << endl;
    cout << " Number of page misses: " << numberOfPageMissI << endl;
    cout << " Number of page replacment: " << numberOfPageRepI << endl;
    cout << " Number of disk page write: " << numberOfPageWriteI << endl;
    cout << " Number of disk page read: " << numberOfPageReadsI << endl;

    cout << "######### CHECK ###################" << endl;
    cout << " Number of reads: " << numberOfReadsC << endl;
    cout << " Number of writes: " << numberOfWritesC << endl;
    cout << " Number of page misses: " << numberOfPageMissC << endl;
    cout << " Number of page replacment: " << numberOfPageRepC << endl;
    cout << " Number of disk page write: " << numberOfPageWriteC << endl;
    cout << " Number of disk page read: " << numberOfPageReadsC << endl;

    return 0;
    
}




void initPhysicalMem(){
    int tnum = 0, res;

    // init bubble thread
    int size = virtualMemSize/4;
    int psize = physicalMemSize/4;
    int index = size*tnum;
    int frameCount = 0;
    int pcount = 0;
    for (int i = index; i < index + psize; i++)
    {
        numberOfWritesF++;
        numberOfPageRepF++;
        in.seekg(sizeof(int)*i);
        in.read((char*)&res,sizeof(res)); 
               
        if((pcount+1)%totalFrame == 0){
             int pageIndx = i/totalFrame; // page table daki entry indexi
            pageTable[pageIndx].pageFrameNum = frameCount;
            frameCount++;
        }


        physicalMem[pcount] = res;
        pcount++;
    }

     //init quick thread
    tnum++;
    index = size*tnum;
   
    for (int i = index; i < index + psize; i++)
    {
        numberOfWritesF++;
        numberOfPageRepF++;
        in.seekg(sizeof(int)*i);
        in.read((char*)&res,sizeof(res));        
        if((pcount+1)%totalFrame == 0){
             int pageIndx = i/totalFrame; // page table daki entry indexi
            pageTable[pageIndx].pageFrameNum = frameCount;
            frameCount++;
        }
        physicalMem[pcount] = res;
        pcount++;
    }

    //init merge thread
    tnum++;
    index = size*tnum;
    for (int i = index; i < index + psize; i++)
    {
        numberOfWritesF++;
       numberOfPageRepF++;
        in.seekg(sizeof(int)*i);
        in.read((char*)&res,sizeof(res));        
        if((pcount+1)%totalFrame == 0){
              int pageIndx = i/totalFrame; // page table daki entry indexi
            pageTable[pageIndx].pageFrameNum = frameCount;
            frameCount++;
        }
        physicalMem[pcount] = res;
        pcount++;
    }
        
    //init index thread
    tnum++;
    index = size*tnum;
    for (int i = index; i < index + psize; i++)
    {
        numberOfWritesF++;
        numberOfPageRepF++;
        in.seekg(sizeof(int)*i);
        in.read((char*)&res,sizeof(res));        
        if((pcount+1)%totalFrame == 0){
              int pageIndx = i/totalFrame; // page table daki entry indexi
            pageTable[pageIndx].pageFrameNum = frameCount;
            frameCount++;
        }
        physicalMem[pcount] = res;
        pcount++;
    }


    // lru icin arrayi init ediyorum.
    int piecePhy = physicalFrame/4;
    for (int i = 0; i < physicalFrame; i++)
    {
        if(local){
            if(i < piecePhy*1){
                lruArr[i] = lruCounterP1;
                lruCounterP1++;
            }
            else if(i < piecePhy*2){
                lruArr[i] = lruCounterP2;
                lruCounterP2++;
            }
            else if(i < piecePhy*3){
                lruArr[i] = lruCounterP3;
                lruCounterP3++;                
            }
            else if(i < piecePhy*4){
                lruArr[i] = lruCounterP4;
                lruCounterP4++;                  
            }
        }
        else{
            lruArr[i] = lruCounter;
            lruCounter++;
        }
    }


    
    

}


// Memorydeki frame i gosteern page entry i bulur.
int findFifoPage(int memoryframe){

    for (int i = 0; i < pageTableSize; i++)
    {
        if(pageTable[i].pageFrameNum == memoryframe){
            return i;
        }
    }        
    
    
    return -1;
}

// memory den diske yazar.
void make_replacement(int diskIndex, int pindex){
    fd.seekp(sizeof(int)*diskIndex);
    for (int j = pindex; j < pindex + totalFrame; j++)
    {
        int temp_value = physicalMem[j];
        fd.write((char*)&temp_value,sizeof(temp_value));                    
    }
}

void writeDisk(int index, int value){
    fd.seekp(sizeof(value)*index);
    fd.write((char*)&value,sizeof(value));    
}

// Diskten memory e frame yazar.
void diskFToMemoryF(int pindex, int base){
    int diskValue;
    int memi = pindex;
    in.seekg(sizeof(int)*base);
    for (int j = base; j < base + totalFrame; j++)
    {
        in.read((char*)&diskValue,sizeof(diskValue));     
        physicalMem[memi] = diskValue;
        memi++;             
    }
}

//Page Entry Güncellenir.
// pageIndex ten hangi programın oldugunu anlıyorum.
void updatePageTable(int pageIndex, int backupMemoryFrame, int memoryPieceSize){
    int virtualQ = pageTableSize/4;
    int quarter = -1;
    //Hangi program anlamak icin.
    if(pageIndex < virtualQ*1){
        quarter = 0;
    }
    else if(pageIndex < virtualQ*2){
        quarter = 1;
    }
    else if(pageIndex < virtualQ*3){
         quarter = 2;
    }
    else if(pageIndex < virtualQ*4){
         quarter = 3;
    }
    pageTable[pageIndex].pageFrameNum = backupMemoryFrame;
    pageTable[pageIndex].modified = 1;
    pageTable[pageIndex].referenced = 0;
    // Fifo kuyrugu guncellemeleri
    if(local && fifo){
        if(backupMemoryFrame == memoryPieceSize*(quarter+1) -1){
            *(pageTable[pageIndex].l_fifo) = memoryPieceSize*quarter;
            
        }
        else
            *(pageTable[pageIndex].l_fifo) = backupMemoryFrame+1;  
    }
    else if (fifo){
        if(backupMemoryFrame == physicalFrame - 1){
            *(pageTable[pageIndex].g_fifo) = 0;
            
        }
        else
            *(pageTable[pageIndex].g_fifo) = backupMemoryFrame+1;         
    }
  
}

//LRU ya gore memory frame bulunur.
int findBackupMemLru(const char* tName,int i){
    int res;
     int piecePhy = physicalFrame/4;
     // Local oldugu zaman parca parca bakıyorum.
    if(i < pageTableSize/4){
        int min = lruArr[0];
        int minIndex = 0;
        for (int i = 1; i < piecePhy; i++)
        {
            if(min > lruArr[i]){
                min = lruArr[i];
                minIndex = i;
            }
        }
        res = minIndex;
        lruArr[minIndex] = lruCounterP1;
        lruCounterP1++;                          
    }     
    else if(i < 2*(pageTableSize/4)){
        
        int min = lruArr[1*piecePhy];
        int minIndex = 1*piecePhy;
       
        for (int i = 1*piecePhy+1; i < 1*piecePhy + piecePhy; i++)
        {
            if(min > lruArr[i]){
                min = lruArr[i];
                minIndex = i;
            }
        }
        res = minIndex;
        lruArr[minIndex] = lruCounterP2;
        lruCounterP2++;   
       
    }  
    else if(i < 3*(pageTableSize/4)){
        int min = lruArr[2*piecePhy];
        int minIndex = 2*piecePhy;
        for (int i = 2*piecePhy+1; i < 2*piecePhy + piecePhy; i++)
        {
            if(min > lruArr[i]){
                min = lruArr[i];
                minIndex = i;
            }
        }
        res = minIndex;
        lruArr[minIndex] = lruCounterP3;
        lruCounterP3++; 
    }    
    else if(i < 4*(pageTableSize/4)){
        int min = lruArr[3*piecePhy];
        int minIndex = 3*piecePhy;
        for (int i = 3*piecePhy+1; i < 3*piecePhy + piecePhy; i++)
        {
            if(min > lruArr[i]){
                min = lruArr[i];
                minIndex = i;
            }
        }
        res = minIndex;
        lruArr[minIndex] = lruCounterP4;
        lruCounterP4++; 
    }    
    else{
        cout << "hataaaa" << endl;
        exit(1);
    }
    //TO-DO
   
    return res;
}

// Referenced bitleri sıfır yapıyorum.
//Eger local ise o programın memoryler referencları sıfırlıyorum.
void resetNru(int start){
    if(!local){
        for (int i = 0; i < pageTableSize; i++)
        {
            pageTable[i].referenced = 0;
        }
        countNru = 0;
    }
    else{
        for (int i = start; i < start + pageTableSize/4; i++)
        {
            pageTable[i].referenced = 0;
        }
        
    }

}



void set(unsigned int index, int value,const char * tName){

    int memoryPieceSize = physicalFrame/4;
    // fill oldugu zaman dosyaya yazıyorum direkt. 
    if(strcmp(tName,"fill") == 0){
        numberOfWritesF++;
        numberOfPageMissF++;
        numberOfPageWriteF++;
         fd.seekp(sizeof(value)*index);
        fd.write((char*)&value,sizeof(value));
    }
    else{
        //increment Page Table Interval
        accessNumPT++;
        if(accessNumPT == pageTablePrintInt){
            accessNumPT = 0;
            printPageTable();
        }

        // Reference bit sıfırlamak icin
        countNru++;
        if(countNru == 20){
            resetNru(0);
        }
        

        int i = index/totalFrame; // 5500/4096 1. frame
        int base = i*totalFrame; // virtual memory de frame baslangici 4096
        int offset = index - base; // 5500 - 4096

        // Page Fault durumu oldugunda veya
        if(pageTable[i].pageFrameNum == -1){ 
     
          
           
            // ilk once diskteki veri guncellenir.
           
            writeDisk(index, value);
            
            // ---------- REPLACEMENT ------------ //
            //Memory to Disk 
             int backupMemoryFrame;
            if(local){
                if(fifo)
                    backupMemoryFrame = *(pageTable[i].l_fifo); // Fifoya gore memory frame indexi
                else if(lru){
                    backupMemoryFrame = findBackupMemLru(tName,i);
                }
                else if(nru){
                    backupMemoryFrame = findNruBackupMem(tName);
                   
                }
            }
            else{
                if(fifo){
                     backupMemoryFrame = *(pageTable[i].g_fifo); // Fifoya gore memory frame indexi
                }
                else if(lru){
                    int min = lruArr[0];
                    int minIndex = 0;
                    for (int i = 1; i < physicalFrame; i++)
                    {
                        if(min > lruArr[i]){
                            min = lruArr[i];
                            minIndex = i;
                        }
                    }
                    backupMemoryFrame = minIndex;
                    lruArr[minIndex] = lruCounter;
                    lruCounter++;
                }
                else if(nru){
                    backupMemoryFrame = findNruBackupMem(tName);
                }

            }
                
            

            int fpindex = findFifoPage(backupMemoryFrame); // Memory frame indexinin bulundugu page entry

            if(fpindex == -1){
                cout << "HATAAAAA -1" << endl;
                exit(1);
            }           

            int pindex = backupMemoryFrame*totalFrame; // Degistirilecek memory frame adresinin baslangici
            int diskIndex = fpindex*totalFrame; // Virtual memory de yazilacak index
            make_replacement(diskIndex,pindex);

            //Update Page Table to X (-1)
            pageTable[fpindex].pageFrameNum = -1;
            
            
            //Disk to memory
            diskFToMemoryF(pindex,base);

            //Update Page Table
            if(strcmp(tName,"bubble") == 0){
                numberOfPageMissB++;
                numberOfPageRepB++;
                numberOfPageWriteB++;
                numberOfWritesB++;
            }
            else if(strcmp(tName,"quick") == 0){
                numberOfPageMissQ++;
                numberOfPageRepQ++;
                numberOfPageWriteQ++;
                numberOfWritesQ++;  
            }
            else if(strcmp(tName,"merge") == 0){
                numberOfPageMissM++;
                numberOfPageRepM++;
                numberOfPageWriteM++;
                numberOfWritesM++;
            }
            else if(strcmp(tName,"index") == 0){
                numberOfPageMissI++;
                numberOfPageRepI++;
                numberOfPageWriteI++;
                numberOfWritesI++;   
            }
            updatePageTable(i,backupMemoryFrame,memoryPieceSize);
            
        }
        else{
            /*----------------- PAGE TABLE DAN FIZIKSEL HAFIZAYA ULASIM -----------------*/
           
            int pframenum = pageTable[i].pageFrameNum; // fiziksel memoryde hangi memoryde oldugu
            //cout << "framenum: " << pframenum << endl;
            int pbase = pframenum*totalFrame; //frame adresinin baslangici
            int pindex = pbase + offset; // offset eklenerek fiziksel memorydeki direkt adres bulunur
           // cout << "pindex: " << pindex << endl;
            physicalMem[pindex] = value;
            pageTable[i].referenced = 1;

            // LRU degiskenlerini artiriyorum.
            if(i < pageTableSize/4){
                lruArr[pframenum] = lruCounterP1;
                lruCounterP1++;
                countNruP1++;
            }
            else if(i < (pageTableSize/4)*2){
                lruArr[pframenum] = lruCounterP2;
                lruCounterP2++;
                countNruP2++;
            }
            else if(i < (pageTableSize/4)*3){
                lruArr[pframenum] = lruCounterP3;
                lruCounterP3++;
                countNruP3++;               
            }
            else if(i < pageTableSize){
                lruArr[pframenum] = lruCounterP4;
                lruCounterP4++;
                countNruP4++;
            }

            //increment statistic variables
            if(strcmp(tName,"bubble") == 0){
                numberOfWritesB++;
            }
            else if(strcmp(tName,"quick") == 0){
                numberOfWritesQ++;
            }
            else if(strcmp(tName,"merge") == 0){
                numberOfWritesM++;
            }
            else if(strcmp(tName,"index") == 0){
                numberOfWritesI++;
            }     
            else if(strcmp(tName,"check") == 0){
                numberOfWritesC++;
            }
        }
    }

}

int readDisk(int index){
    int ret;
    in.seekg(sizeof(int)*index);
    in.read((char*)&ret,sizeof(ret));  
    return ret;  
}

int findNruBackupMem(const char* tName){
    int case0 = -1, case1 = -1, case2 = -1, case3 = -1, res = -1;

    for (int i = 0; i < pageTableSize; i++)
    {
        if(pageTable[i].pageFrameNum != -1){
            if(pageTable[i].referenced == 0 && pageTable[i].modified == 0){
                case0 = i;
                break;
            }
            else if(pageTable[i].referenced == 0 && pageTable[i].modified == 1){
                case1 = i;
            }
            else if(pageTable[i].referenced == 1 && pageTable[i].modified == 0){
                case2 = i;
            }
            else if(pageTable[i].referenced == 1 && pageTable[i].modified == 1){
                case3 = i;
            }
        }
    }


    if(case0 != -1){
        res = pageTable[case0].pageFrameNum;
    }
    else if(case1 != -1){
        res = pageTable[case1].pageFrameNum;
    }
    else if(case2 != -1){
        res = pageTable[case2].pageFrameNum;
    }
    else if(case3 != -1){
        res = pageTable[case3].pageFrameNum;
    }
    return res;
}

int get(unsigned int index,const  char * tName){
    int res = 0;
    int memoryPieceSize = physicalFrame/4;
    if(tName == NULL){
        in.seekg(sizeof(int)*index);
        in.read((char*)&res,sizeof(res));
    }
    else{
        //Page table print i icin
        accessNumPT++;
        if(accessNumPT == pageTablePrintInt){
            accessNumPT = 0;
            printPageTable();
        }
        countNru++;
        if(countNru == 20){
            resetNru(0);
        }
        
        int i = index/totalFrame; // 5500/4096 1. frame
        int base = i*totalFrame; // virtual memory de frame baslangici 4096
        int offset = index - base; // 5500 - 4096
        if(pageTable[i].pageFrameNum == -1){
 
          
           
            res = readDisk(index);
            //ardindan uygun memory alani bulunur.
             int backupMemoryFrame;
            if(local){
                if(fifo)
                    backupMemoryFrame = *(pageTable[i].l_fifo); // Fifoya gore memory frame indexi
                else if(lru){
                    backupMemoryFrame = findBackupMemLru(tName,i);
                }
                else if(nru){
                    backupMemoryFrame = findNruBackupMem(tName);
                  
                }
            }
            else{
                if(fifo){
                     backupMemoryFrame = *(pageTable[i].g_fifo); // Fifoya gore memory frame indexi
                }
                else if(lru){
                    int min = lruArr[0];
                    int minIndex = 0;
                    for (int i = 1; i < physicalFrame; i++)
                    {
                        if(min > lruArr[i]){
                            min = lruArr[i];
                            minIndex = i;
                        }
                    }
                    backupMemoryFrame = minIndex;
                    lruArr[minIndex] = lruCounter;
                    lruCounter++;
                }
                else if(nru){
                    backupMemoryFrame = findNruBackupMem(tName);
                }
            }
           

            int fpindex = findFifoPage(backupMemoryFrame); // Memory frame indexinin bulundugu page entry

            if(fpindex == -1){
                cout << "HATAAAAA -1" << endl;
                exit(1);
            }
            int pindex = backupMemoryFrame*totalFrame; // degistirilecek memory frameinin baslangic adresi
            int diskIndex = fpindex*totalFrame; // virtual memory deki yeri doner
            // ---------- REPLACEMENT ------------ //
            //Memory to Disk
            make_replacement(diskIndex,pindex);

            //Update Page Table to X (-1)
            pageTable[fpindex].pageFrameNum = -1;
            
            //Disk to memory
            diskFToMemoryF(pindex,base);

            //Update Page Table
            if(strcmp(tName,"bubble") == 0){
                numberOfPageMissB++;
                numberOfPageRepB++;
                numberOfPageReadsB++;
                numberOfReadsB++;
            }
            else if(strcmp(tName,"quick") == 0){
                numberOfPageMissQ++;
                numberOfPageRepQ++;
                numberOfPageReadsQ++;
                numberOfReadsQ++;               
            }
            else if(strcmp(tName,"merge") == 0){   
                numberOfPageMissM++;
                numberOfPageRepM++;
                numberOfPageReadsM++;
                numberOfReadsM++;
            }
            else if(strcmp(tName,"index") == 0){
                numberOfPageMissI++;
                numberOfPageRepI++;
                numberOfPageReadsI++;
                numberOfReadsI++;
            }
            else if(strcmp(tName,"check") == 0){
                numberOfPageMissC++;
                numberOfPageRepC++;
                numberOfPageReadsC++;
                numberOfReadsC++;                
            }
            updatePageTable(i,backupMemoryFrame,memoryPieceSize); 
        }
        else{
            /*----------------- PAGE TABLE DAN FIZIKSEL HAFIZAYA ULASIM -----------------*/
            int pframenum = pageTable[i].pageFrameNum; // fiziksel memoryde hangi memoryde oldugu
            //cout << "pframenum: " << pframenum << endl;
            int pbase = pframenum*totalFrame; //frame adresinin baslangici
            int pindex = pbase + offset; // offset eklenerek fiziksel memorydeki direkt adres bulunur
            res = physicalMem[pindex];
            pageTable[i].referenced = 1;
            int virtualQ = pageTableSize/4;
            if(i < virtualQ*1){
                lruArr[pframenum] = lruCounterP1;
                lruCounterP1++;
            }
            else if(i < virtualQ*2){
                lruArr[pframenum] = lruCounterP2;
                lruCounterP2++;
            }
            else if(i < virtualQ*3){
                lruArr[pframenum] = lruCounterP3;
                lruCounterP3++;
            }
            else if(i <  virtualQ*4){
                lruArr[pframenum] = lruCounterP4;
                lruCounterP4++;
            }

            if(strcmp(tName,"bubble") == 0){
                numberOfReadsB++;
            }
            else if(strcmp(tName,"quick") == 0){
                 numberOfReadsQ++; 
            }
            else if(strcmp(tName,"merge") == 0){
                  numberOfReadsM++;
            }
            else if(strcmp(tName,"index") == 0){
                numberOfReadsI++;
            }
            else if(strcmp(tName,"check") == 0){
                numberOfReadsC++;
            }

        }
    }

    return res;
}

void printPageTable(){

    cout << "################# PAGE TABLE #############################" <<  endl;
    for (int i = 0; i < pageTableSize; i++)
    {
        cout << "Index: " << i << endl;
        cout << "PageFrame: " << pageTable[i].pageFrameNum << endl;
        cout << "Modified: " << pageTable[i].modified << endl;
        cout << "Reference: " << pageTable[i].referenced << endl;
        cout << "Local fifo: " << *(pageTable[i].l_fifo) << endl;
        cout << "Global fifo: " << *(pageTable[i].g_fifo) << endl;
        cout << "---------------------" << endl;
    }
    cout << "----------------------------------------------------------------------" << endl;

}

void initPageTable(){
    int pieceSize = physicalFrame/4;
    int virtualpieceSize = virtualFrame/4;
    // Fifo algoritmasi ilk giren frame i tutan degisken. her entry de ayni anda degismesi icin referans
    // olarak tutuyorum.
    int* gfifo = (int*) malloc(sizeof(int)); 
    int* p1fifo = (int*) malloc(sizeof(int)); 
    int* p2fifo = (int*) malloc(sizeof(int)); 
    int* p3fifo = (int*) malloc(sizeof(int)); 
    int* p4fifo = (int*) malloc(sizeof(int)); 
    *gfifo = 0;
    *p1fifo = pieceSize*0;
    *p2fifo = pieceSize*1;
    *p3fifo = pieceSize*2;
    *p4fifo = pieceSize*3;
   
    for (int i = 0; i < pageTableSize; i++)
    {
        pageTable[i].modified = 0;
        pageTable[i].referenced = 0;
        pageTable[i].pageFrameNum = -1;
        pageTable[i].g_fifo = gfifo;
        if(i < virtualpieceSize*1){
            pageTable[i].l_fifo = p1fifo;
        }
        else if(i < virtualpieceSize*2){
            pageTable[i].l_fifo = p2fifo;
        }
        else if(i < virtualpieceSize*3){
            pageTable[i].l_fifo = p3fifo;
        }
        else{
            pageTable[i].l_fifo = p4fifo;
        }
    }
    

}



void* sortVirtualArray(void* arg){
     int num = (int) arg;
    int size = virtualMemSize/4;
    int index = size*(num);
  	

    //Bubble
    if(num == 0){
        int temp;
        for (int i = index; i < index + size-1; ++i)
        {
            for (int j = index; j < index + size-i-1; ++j)
            {   
                 pthread_mutex_lock(&mutex);
                int prev = get(j,"bubble");
                int next = get(j+1,"bubble");
                pthread_mutex_unlock(&mutex);
                if(prev > next){
                    temp = next;
                     pthread_mutex_lock(&mutex);
                    set(j+1,prev,"bubble");
                    set(j,temp,"bubble");
                     pthread_mutex_unlock(&mutex);
                }
            }
        }    
    }
    else if(num ==1){
       
       quickSort(index,index+size-1);
      
    }
    else if(num == 2){
         mergeSort(index, index + size - 1); 
    }
    else if(num == 3){
        indexSort(index, index + size - 1);
    }

    return NULL;
}

void indexSort(int low_ind, int up_ind){

	int counter1 = 0;
	int counter2 = 0;

	while(low_ind < up_ind){
	
		
		for (int j = low_ind+1; j < up_ind; ++j)
		{
            pthread_mutex_lock(&mutex);
			if(get(low_ind,"index") > get(j,"index"))
				counter1++;
            pthread_mutex_unlock(&mutex);
		}
		if(counter1 > 0){
             pthread_mutex_lock(&mutex);
			int t = get(low_ind,"index");
            int sv = get(counter1+low_ind,"index");
            set(low_ind,sv,"index");
            set(counter1 + low_ind,t,"index");
            pthread_mutex_unlock(&mutex);
			counter1 = 0;
		
		}
		else{
			low_ind = low_ind+1;
		}
		int j = up_ind-1;
		while(j >=low_ind){
            pthread_mutex_lock(&mutex);
			if(get(up_ind,"index") < get(j,"index"))
				counter2++;
            pthread_mutex_unlock(&mutex);
			j = j-1;
		}
		if(counter2 >0){
            pthread_mutex_lock(&mutex);
			int t=get(up_ind,"index");
            int sv = get(up_ind- counter2,"index");
            set(up_ind,sv,"index");
			set(up_ind-counter2,t,"index");
            pthread_mutex_unlock(&mutex);
			counter2=0;
	
		}
		else{
			up_ind =up_ind-1;
		}
	}

}

void merge(int left, int mid, int right) 
{ 
   
    int n1 = mid - left + 1; 
    int n2 = right - mid; 
    int i, j, k; 
    int* Lft = new int[n1];
    int* Rght = new int[n2]; 

     pthread_mutex_lock(&mutex);
    for (i = 0; i < n1; i++) 
        Lft[i] = get(left+i,"merge");
    for (j = 0; j < n2; j++) 
        Rght[j] =  get(mid + 1 + j,"merge");
     pthread_mutex_unlock(&mutex);
   
    i = 0; 
    j = 0;
    k = left; 
    while (i < n1 && j < n2) { 
        if (Lft[i] <= Rght[j]) { 
             pthread_mutex_lock(&mutex);
            set(k,Lft[i],"merge");
             pthread_mutex_unlock(&mutex);
            i++; 
        } 
        else { 
             pthread_mutex_lock(&mutex);
            set(k,Rght[j],"merge");
             pthread_mutex_unlock(&mutex);
            j++; 
        } 
        k++; 
    } 

    while (i < n1) { 
         pthread_mutex_lock(&mutex);
        set(k,Lft[i],"merge");
         pthread_mutex_unlock(&mutex);
        i++; 
        k++; 
    } 
  

    while (j < n2) { 
         pthread_mutex_lock(&mutex);
        set(k,Rght[j],"merge");
         pthread_mutex_unlock(&mutex);
        j++; 
        k++; 
    } 
} 

void  mergeSort(int left, int right){

    if (left < right) { 

        int mid = left + (right - left) / 2; 
  
       
        mergeSort(left, mid); 
        mergeSort(mid + 1, right); 
  
        merge(left, mid, right); 
    } 

}

void quickSort( int left, int right){

    if (left < right) 
    { 

        int pi = partition(left, right); 
  

        quickSort(left, pi - 1); 
        quickSort(pi + 1, right); 
    } 
}

int partition(int left, int right){
     pthread_mutex_lock(&mutex);
    int pivot = get(right,"quick");   
     pthread_mutex_unlock(&mutex);
    int i = (left - 1);  
  
    for (int j = left; j <= right- 1; j++) 
    { 

         pthread_mutex_lock(&mutex);
        if (get(j,"quick") <= pivot) 
        { 
            i++;    
            int temp = get(i,"quick");
            int jv = get(j,"quick");
            set(i,jv,"quick");
            set(j,temp,"quick");
        } 
        pthread_mutex_unlock(&mutex);
    } 
     pthread_mutex_lock(&mutex);
    int temp2 = get(i+1,"quick");
    int jvv = get(right,"quick");
    set(i+1,jvv,"quick");
    set(right,temp2,"quick");
     pthread_mutex_unlock(&mutex);
    return (i + 1); 
}


void clean_up(){
    int pieceSize = pageTableSize/4;
    free(physicalMem);
    free(pageTable[0].g_fifo);
    free(pageTable[0].l_fifo);
    free(pageTable[pieceSize*1].l_fifo);
    free(pageTable[pieceSize*2].l_fifo);
    free(pageTable[pieceSize*3].l_fifo);
    free(pageTable);
    fd.close();
    in.close();
}

void displayUsage(){
    printf("sortArrays frameSize numPhysical numVirtual pageReplacement allocPolicy pageTablePrintInt diskFileName.dat\n");
    exit(1);
}
