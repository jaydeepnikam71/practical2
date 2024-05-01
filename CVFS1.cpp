#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string.h>

using namespace std;

#define MAXINODE 50
#define MAXFILESIZE 1024

#define REGULAR 1
#define READ 1
#define WRITE 2

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
} SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;
    struct inode *next;
} INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;
    PINODE ptrinode;
} FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
} UFDT;

UFDT UFDTArr[MAXINODE];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

void CreateDILB()
{
    int i=1;
    PINODE newn = NULL;
    PINODE temp=head;

    while(i<=MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount=0;
        newn->ReferenceCount=0;
        newn->FileType=0;
        newn->FileSize=0;

        newn->Buffer=NULL;
        newn->next=NULL;

        newn->InodeNumber = i;

        if(temp==NULL)
        {
            head=newn;
            temp=head;
        }
        else
        {
            temp->next=newn;
            temp =temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

void InitialiseSuperBlock()
{
    int i=0;
    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable !=NULL)
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
                break;
        i++;
    }

    if(i == MAXINODE)
        return -1;
    else
        return i;
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;

    if(name == NULL)
        return NULL;

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

int CreateFile(char *name, int permission)
{
    int i=0;
    PINODE temp = head;

    if((name == NULL) || (permission == 0) || (permission > 3))
        return -1;

    if(SUPERBLOCKobj.FreeInode == 0)
        return -2;

    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name) != NULL)
        return -3;

    while(temp != NULL)
    {
        if(temp->FileType == 0)
            break;
        temp = temp->next;
    }

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFILESIZE);

    return i;
}

int WriteFile(int fd, char *arr, int isize)
{
    if(fd < 0 || fd >= MAXINODE)
        return -1;

    if(UFDTArr[fd].ptrfiletable == NULL)
        return -1;

    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
        return -1;

    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ + WRITE))
        return -1;

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
        return -2;

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
        return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);
    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;
    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize;
}

int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[1024];

    InitialiseSuperBlock();
    CreateDILB();

    while(1)
    {
        fflush(stdin);
        strcpy(str, "");

        printf("\nMarvellous VFS: >");
        fgets(str, 80, stdin);

        count = sscanf(str, "%s%s%s%s", command[0], command[1], command[2], command[3]);

        if(count == 1)
        {
            if(strcmp(command[0], "ls") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "closeall") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "clear") == 0)
            {
                system("clear");
                continue;
            }
            else if(strcmp(command[0], "exit") == 0)
            {
                printf("Terminating the Marvellous Virtual File System\n");
                break;
            }
            else
            {
                printf("\nError: Command not found!!!\n");
                continue;
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0], "stat") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "fstat") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "close") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "rm") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "man") == 0)
            {
                continue;
            }
            else if(strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);

                if(fd == -1)
                {
                    printf("Error: Incorrect parameters\n");
                    continue;
                }

                printf("Enter the data: \n");
                scanf("%[^\n]",arr);

                ret = strlen(arr);
                if(ret == 0)
                {
                    printf("Error: Incorrect parameters\n");
                    continue;
                }

                ret = WriteFile(fd, arr, ret);
                if(ret == -1)
                    printf("Error: Permission denied\n");
                if(ret == -2)
                    printf("Error: No memory to write\n");
                if(ret == -3)
                    printf("Error: It is not regular file\n");
            }
            else if(strcmp(command[0], "truncate") == 0)
            {
                continue;
            }
            else
            {
                printf("\nError: Command not found!!!\n");
                continue;
            }
        }
        else if(count == 3)
        {
            if(strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if(ret >= 0)
                    printf("File is successfully created with file descriptor: %d\n", ret);
                if(ret == -1)
                    printf("Error: Incorrect parameters\n");
                if(ret == -2)
                    printf("Error: There is no inodes\n");
                if(ret == -3)
                    printf("Error: File already exists\n");
                if(ret == -4)
                    printf("Error: Memory allocation failure\n");
                continue;
            }
            else
            {
                printf("\nError: Command not found!!!\n");
                continue;
            }
        }
        else
        {
            printf("\nError: Command not found!!!\n");
            continue;
        }
    }

    return 0;
}
