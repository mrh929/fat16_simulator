#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#define TRUE 1
#define FALSE 0
#define ISCMD(t) strcmp(a, t) == 0
#define COMMAND_MAX 100
#define SECTORS_PER_CLUST 1//ÿ��������
#define BYTES_PER_SECTOR 512//ÿ�����ֽ���
#define SECTORS_PER_FAT 128//ÿFAT������
#define RESERVED_SECTORS 1//����������
#define SECTORS_ROOT SECTORS_PER_CLUST//��Ŀ¼��������
#define CLUST_MAX_COUNT (SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1) //�ص��ܸ��� 
#define ROOT (RESERVED_SECTORS + SECTORS_PER_FAT * 2) * BYTES_PER_SECTOR//��Ŀ¼��λ��
#define CLUST_SECOND ROOT + SECTORS_ROOT * BYTES_PER_SECTOR//�ڶ����ص�λ�� 
FILE *fp;

struct clu{//һ���� 
	unsigned char status;
	unsigned char byte[BYTES_PER_SECTOR * SECTORS_PER_CLUST + 10]; //��������� 
}clust[CLUST_MAX_COUNT];

struct fi{//һ���ļ� 
	bool dir;
	char name[2][20];
	int time;//�޸�ʱ�� 
	int date;//�޸����� 
	int clust;//�״غ�
	long long len;//�ļ���С
	fi *next;//��һ���ļ� 
}file[1 + CLUST_MAX_COUNT * SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32];
int FileCnt, Clust_now;//��ǰ�ļ����ܸ����뵱ǰ�Ĵ� 
fi *HEAD;//ͷָ�� 

bool Data_RW(const int);//�ļ�����������д���ر�
void Data_Initialize();//��ʼ�������ļ�
void Data_Create_New();//�����µ���FAT16�����ļ�
void Data_Read();//�����ݶ����ڴ� 
void Data_Save();//�����ļ� 
fi *File_Search(char *);//���Ѿ�������ļ���Ѱ�Ҷ�Ӧ���Ƶ��ļ� 
void File_List_Update(int);//Ѱ�ҵ�ǰĿ¼�µ��ļ�/�ļ���
void File_List_Initialize();//�ļ��б���� 
void File_List_Add(fi *);//���ļ��б��м���һ��Ԫ�� 
void File_List_Delete(fi *p);//���ļ��б���ɾ��һ��Ԫ�� 
void Bubble_Sort();//��ð��������ļ������ֵ�������
void File_Print();//��ӡ�����е��ļ� 
void DBR_Print();//����DBR
void Manu_Print();//��ӡ�����˵�
void Bin_Printf(long long, int);//��С����ʽ��ӡ (���ļ�)
long long Bin_Readf(int);//��ȡn���ֽڵ�С��������
void Bin_Prints(unsigned char *, long long, int);//��С����ʽ��ӡ (���ַ���)
long long Bin_Reads(unsigned char *, int);//��ȡn���ֽڵ�С��������
bool Command_Read();//��ȡ���� 
bool Read_32Bytes(unsigned char *, fi *);//��ȡһ���ļ�/�ļ��е���Ϣ 
bool Is_End(unsigned char); //�жϸô��Ƿ�Ϊ���һ�� 

int main(){
	Data_Initialize();
	Manu_Print();
	bool FLAG = FALSE;
	do{
		printf("\\root:\\>");
		FLAG = Command_Read();
	}while(FLAG);
	return 0;
	
}

bool Command_Read(){
	char str[COMMAND_MAX], a[COMMAND_MAX], b[COMMAND_MAX];
	gets(str);
	sscanf(str, "%s%s", a, b);
	
	if(str[0] == 0) return TRUE;
	
	File_List_Update(Clust_now);
//	Bubble_Sort();
	if(ISCMD("ls") || ISCMD("dir")){
		File_Print();
		
	}else if(ISCMD("cd")){
		fi *t = File_Search(b);
		if(t != NULL && t->dir == TRUE)
			Clust_now = t->clust;
		else
			printf("Invalid directory!\n\n");
			
	}else if(ISCMD("mkdir")){
		
		
		
	}else if(ISCMD("create")){
		
		
		
	}else if(ISCMD("rmdir")){
		
		
	}else if(ISCMD("rm")){
		
		
	}else if(ISCMD("read")){
		
		
	}else if(ISCMD("write")){
		
		
	}else if(ISCMD("format")){
		printf("Are you sure to FORMAT this disk? (y/n): ");
		scanf("%s", a);
		if(ISCMD("y") || ISCMD("Y")){
			printf("Once again. You'll lose your data, continue? (y/n): ");
			scanf("%s", a);
			if(ISCMD("y") || ISCMD("Y")){
				printf("formating...  ");
				Data_RW(2);
				Data_Create_New();
				Data_Read();
				Data_RW(3);
				printf("done.\n");
				getchar();
			}
		}
		
		
	}else if(ISCMD("save")){
		printf("Are you sure to quit with saving? (y/n): ");
		scanf("%s", a);
		if(ISCMD("y") || ISCMD("Y")){
			Data_Save();
			return FALSE;
		}
		
	}else{
		printf("unknown command \"%s\"\n",str);
		
	}
	return TRUE;//����ѭ�� 
}


bool Data_RW(const int t){//done
	switch(t){
		case 1: //ֻ�� 
			fp = fopen("fat16_data.bin", "rb");
			return fp != NULL;
		case 2: //��д 
			fp = fopen("fat16_data.bin", "wb");
			return fp != NULL;
		case 3:
			fclose(fp);
			return 1;
	}
}

void Data_Initialize(){//done
	if(Data_RW(1) == FALSE){
		printf("Disk data doesn't exist. \nCreating a new one...\n");
		Data_Create_New();
	}
	Data_RW(3);//�ر��ļ� 
	Data_RW(1);//ֻ��ģʽ 
	
	Data_Read();
	printf("Data loaded successfully.\n\n");
	Clust_now = 1;
	Data_RW(3);
}

void Data_Read(){
	fseek(fp, BYTES_PER_SECTOR, SEEK_CUR);//����DBR���� 
	for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)//ע�⣬�����Ǵ�0��ʼ�� 
		clust[i].status = (char) Bin_Readf(2);//��ȡFAT1 
	fseek(fp, SECTORS_PER_FAT * BYTES_PER_SECTOR, SEEK_CUR);//����FAT2
	fread(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//��ȡ��Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fread(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//��ȡ������ 
}

void Data_Create_New(){//done
	Data_RW(2);
	DBR_Print();
	for(int i = 1; i <= (SECTORS_PER_FAT * 2 + SECTORS_PER_CLUST * CLUST_MAX_COUNT) * BYTES_PER_SECTOR; i++)
		fprintf(fp, "%c", 0);//�ѿ�����ˢΪ0
	Data_RW(3);
}

void Data_Save(){
	Data_RW(2);
	DBR_Print();
	for(int j = 0; j <= 1; j++)
		for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)
			Bin_Printf(clust[i].status, 2);//��ӡFAT����ӡ����
	fwrite(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//�����Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fwrite(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//����������
	Data_RW(3);
}

fi *File_Search(char *name){
	char *a = name, *b;
	while(*name != '.' && *name != 0) name++;
	if(*name != 0){
		*name = 0;
		b = name + 1;
	}
	
	for(fi *p = HEAD; p != NULL; p = p->next)
		if(strcmp(a, p->name[0]) == 0 && strcmp(b, p->name[1]) == 0)
			return p;//�ҵ��� 
	return NULL;//û�ҵ� 
}

void File_Print(){
	for(fi *p = HEAD; p != NULL; p = p->next){
		printf("%s\t\t", p->dir?"<DIR>":"     ");
		printf("%s",p->name[0]);
		if(!p->dir && p->name[1] != 0) printf(".%s", p->name[1]);
		printf("\n");
	}
}

void File_List_Update(int id){
	if(id <= 0) return;
	
	fi t;
	for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(Read_32Bytes(clust[id].byte + 32 * i, &t)){//�����ȡ�ɹ���������һ�����ڴ������ļ� 
			fi *p = (fi *) malloc(sizeof(fi));
			File_List_Add(p);
			FileCnt++;
		}
			
	
	if(!Is_End(clust[id].status))//�������غ��滹���ļ� �ͼ�����ȡ 
		File_List_Update(clust[id].status);
	else return;
}

void File_List_Initialize(){
	FileCnt = 0;
	HEAD = NULL;
	
	
}

void File_List_Add(fi *p){
	p->next = HEAD;//ֱ����ͷ��������½�� 
	HEAD = p;
	FileCnt++;
}

void File_List_Delete(fi *p){
	if(HEAD == p){//���p��ͷ 
		HEAD = p->next;
		free(p);
	}else{//���p����ͷ 
		fi *pre = HEAD;
		while(pre->next != p) pre = pre->next;
		pre->next = p->next;
		free(p);
	}
}

bool Is_End(unsigned char t){
	if(t >= 0x2 && t <= 0xFFEF) return 0;
	else return 1;
}

bool Read_32Bytes(unsigned char *str, fi *a){
	sscanf("%8s %3s", a->name[0], a->name[1]);
	if(a->name[0][0] == 0xE5 || a->name[0][0] == 0){//������ļ��Ѿ���ɾ�����߲����� 
		fseek(fp, 16, SEEK_CUR);//ֱ������ʣ��32 - 16 = 16���ֽ�
		return FALSE;
	}
	str += 16;
	if(*str++ == 10) a->dir = TRUE; 
	else a->dir = FALSE;
	
	fseek(fp, 10, SEEK_CUR);//����10���������ֽ� 
	a->time = Bin_Reads(str, 2);//��ȡʱ��
	str += 2;
	a->date = Bin_Reads(str, 2);//��ȡ����
	str += 2;
	a->clust = Bin_Reads(str, 2);//��ȡ�״غ�
	str += 2;
	a->len = Bin_Reads(str, 4);//��ȡ�ļ���С 
	str += 4;
	
	return TRUE;
}

void Bubble_Sort(){
	for(int i = 0; i < FileCnt; i++)
		for(fi *p = HEAD; p->next != NULL; p = p->next)
			if(strcmp(p->name[0], p->next->name[0]) > 0){
				/*
				
					��������Ҫдһ�� 
				
				*/
			}
}

void DBR_Print(){//done
	fprintf(fp, "%c%c%c",0xEB ,0x3C ,0x90 );
	fprintf(fp, "ByMrh929");
	Bin_Printf(BYTES_PER_SECTOR, 2);
	Bin_Printf(SECTORS_PER_CLUST, 1);
	Bin_Printf(RESERVED_SECTORS, 2);//���������� 
	Bin_Printf(2, 1);//FAT����
	Bin_Printf(512, 2);//��Ŀ¼���� 
	Bin_Printf(SECTORS_PER_FAT, 2);
	Bin_Printf(63, 2);
	Bin_Printf(0, 4);
	fprintf(fp, "FAT16   ");
	for(int i = 1; i <= 475; i++)
		fprintf(fp, "%c", 0x99);
	Bin_Printf(43605, 2);
}

void Bin_Prints(unsigned char *str, long long sum, int n){
	long long m;
	for(int i = 0; i < n; i++){
		m = sum % 256;
		*str++ = m;//���һλ������ָ����� 
		sum /= 256;
	}
}

long long Bin_Reads(unsigned char *str, int n){
	long long sum = 0;
	unsigned char t;
	for(int i = 0; i < n ; i++){
		t = *str++;	//��ȡһλ����ָ����� 
//		sscanf(str, "%c", &t);
		sum += t << (i * 8);
	}
	return sum;
}

void Bin_Printf(long long sum, int n){//done
	long long m;
	for(int i = 0; i < n; i++){
		m = sum % 256;
		fprintf(fp, "%c", m);
		sum /= 256;
	}
}

long long Bin_Readf(int n){//done
	long long sum = 0;
	unsigned char t;
	for(int i = 0; i < n ; i++){ 
		fscanf(fp, "%c", &t);
		sum += t << (i * 8);
	}
	return sum;
}

void Manu_Print(){//done
	printf("This system supports the following commands:\n\n");
	printf("ls/dir\t\t\t\t\tShow the files and folders contained.\n");
	printf("cd [DIR]\t\t\t\tEnter an appinted folder.\n");
	printf("mkdir [DIR]\t\t\t\tCreate a folder.\n");
	printf("create [NAME]\t\t\t\tCreate a file.\n");
	printf("rmdir [DIR]\t\t\t\tDelete a folder.\n");
	printf("rm [NAME]\t\t\t\tDelete a file.\n");
	printf("read [NAME]\t\t\t\tRead a file.\n");
	printf("write [NAME]\t\t\t\tWrite a file.\n");
	printf("format\t\t\t\t\tFormat this disk.(be careful)\n");
	printf("save\t\t\t\t\tSave this disk and quit.(without which you'll lose your data)\n\n\n");
}
