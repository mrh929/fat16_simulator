#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
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
}file[1 + CLUST_MAX_COUNT * SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32];
int FileCnt, Clust_now;//��ǰ�ļ����ܸ����뵱ǰ�Ĵ� 

bool File_RW(const int);//�ļ�����������д���ر�
void File_Initialize();//��ʼ�������ļ�
void File_Create_New();//�����µ���FAT16�����ļ�
void File_Read();//�����ݶ����ڴ� 
void File_Save();//�����ļ� 
void DBR_Print();//����DBR
void Manu_Print();//��ӡ�����˵�
void Bin_Printf(long long, int);//��С����ʽ��ӡ (���ļ�)
long long Bin_Readf(int);//��ȡn���ֽڵ�С��������
void Bin_Prints(unsigned char *, long long, int);//��С����ʽ��ӡ (���ַ���)
long long Bin_Reads(unsigned char *, int);//��ȡn���ֽڵ�С��������
bool Command_Read();//��ȡ����
void Bubble_Sort();//��ð��������ļ������ֵ������� 
void File_Search(int);//Ѱ�ҵ�ǰĿ¼�µ��ļ�/�ļ���
bool Read_32Bytes(unsigned char *, fi *);

int main(){
	File_Initialize();
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
	
	if(ISCMD("ls") || ISCMD("dir")){
		
		
	}else if(ISCMD("cd")){
		
		
		
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
				File_RW(2);
				File_Create_New();
				File_Read();
				File_RW(3);
				printf("done.\n");
				getchar();
			}
		}
		
		
	}else if(ISCMD("save")){
		printf("Are you sure to quit with saving? (y/n): ");
		scanf("%s", a);
		if(ISCMD("y") || ISCMD("Y")){
			File_Save();
			return FALSE;
		}
		
	}else{
		printf("unknown command \"%s\"\n",str);
		
	}
	return TRUE;//����ѭ�� 
}


bool File_RW(const int t){//done
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

void File_Initialize(){//done
	if(File_RW(1) == FALSE){
		printf("Disk data doesn't exist. \nCreating a new one...\n");
		File_Create_New();
	}
	File_RW(3);//�ر��ļ� 
	File_RW(1);//ֻ��ģʽ 
	
	File_Read();
	printf("Data loaded successfully.\n\n");
	File_RW(3);
}

void File_Read(){
	fseek(fp, BYTES_PER_SECTOR, SEEK_CUR);//����DBR���� 
	for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)//ע�⣬�����Ǵ�0��ʼ�� 
		clust[i].status = (char) Bin_Readf(2);//��ȡFAT1 
	fseek(fp, SECTORS_PER_FAT * BYTES_PER_SECTOR, SEEK_CUR);//����FAT2
	fread(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//��ȡ��Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fread(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//��ȡ������ 
}

void File_Create_New(){//done
	File_RW(2);
	DBR_Print();
	for(int i = 1; i <= (SECTORS_PER_FAT * 2 + SECTORS_PER_CLUST * CLUST_MAX_COUNT) * BYTES_PER_SECTOR; i++)
		fprintf(fp, "%c", 0);//�ѿ�����ˢΪ0
	File_RW(3);
}

void File_Save(){
	File_RW(2);
	DBR_Print();
	for(int j = 0; j <= 1; j++)
		for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)
			Bin_Printf(clust[i].status, 2);//��ӡFAT����ӡ����
	fwrite(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//�����Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fwrite(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//����������
	File_RW(3);
}

void File_Search(int id){
	if(id <= 0) return;
	for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(Read_32Bytes(clust[id].byte + 32 * i, &file[FileCnt + 1]))
			FileCnt++;//����ע��������ǰ�������Ϊ�� 
	
	
	
	
	
	
	
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
