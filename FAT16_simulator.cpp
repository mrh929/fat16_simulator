#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define TRUE 1
#define FALSE 0
#define SECTORS_PER_CLUST 1//ÿ��������
#define BYTES_PER_SECTOR 512//ÿ�����ֽ���
#define SECTORS_PER_FAT 128//ÿFAT������
#define RESERVED_SECTORS 1//����������
#define SECTORS_ROOT 32//��Ŀ¼��������
#define CLUST_MAX_COUNT (SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1) //�ص��ܸ��� 
#define ROOT (RESERVED_SECTORS + SECTORS_PER_FAT * 2) * BYTES_PER_SECTOR//��Ŀ¼��λ��
#define CLUST_SECOND ROOT + SECTORS_ROOT * BYTES_PER_SECTOR//�ڶ����ص�λ�� 
FILE *fp;

struct clu{//һ���� 
	unsigned char status;
	unsigned char byte[BYTES_PER_SECTOR * SECTORS_PER_CLUST + 10]; //��������� 
}clust[CLUST_MAX_COUNT];

bool File_RW(const int);//�ļ�����������д���ر�
void File_Initialize();//��ʼ�������ļ� 
void File_Create_New();//�����µ���FAT16�����ļ�
void File_Save();//�����ļ� 
void DBR_Print();//����DBR
void Manu_Print();//��ӡ�����˵�
void Bin_Print(long long, int);//��С����ʽ��ӡ 
long long Bin_Read(int);//��ȡn���ֽڵ�С��������

int main(){
	File_Initialize();
	Manu_Print();
	printf("\\root:\\>");
	getchar();
//	File_Save();
	
	
}

bool File_RW(const int t){//done
	switch(t){
		case 1: 
			fp = fopen("fat16_data.bin", "rb");
			return fp != NULL;
		case 2: 
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
	
	fseek(fp, BYTES_PER_SECTOR, SEEK_CUR);//����DBR���� 
	for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)//ע�⣬�����Ǵ�0��ʼ�� 
		clust[i].status = (char) Bin_Read(2);//��ȡFAT1 
	fseek(fp, SECTORS_PER_FAT * BYTES_PER_SECTOR, SEEK_CUR);//����FAT2
	fread(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//��ȡ��Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fread(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//��ȡ������ 
	printf("Loaded data successfully.\n\n");
}

void File_Create_New(){//done
	File_RW(2);
	DBR_Print();
	for(int i = 1; i <= (SECTORS_PER_FAT * 2 + SECTORS_PER_CLUST * CLUST_MAX_COUNT) * BYTES_PER_SECTOR; i++)
		fprintf(fp, "%c", 0);//�ѿ�����ˢΪ0 
}

void File_Save(){
	File_RW(2);
	DBR_Print();
	for(int j = 0; j <= 1; j++)
		for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)
			Bin_Print(clust[i].status, 2);//��ӡFAT����ӡ����
	fwrite(clust[1].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//�����Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fwrite(clust[i].byte, sizeof(unsigned char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//����������
	File_RW(3);
}

void DBR_Print(){//done
	fprintf(fp, "%c%c%c",0xEB ,0x3C ,0x90 );
	fprintf(fp, "ByMrh929");
	Bin_Print(BYTES_PER_SECTOR, 2);
	Bin_Print(SECTORS_PER_CLUST, 1);
	Bin_Print(RESERVED_SECTORS, 2);//���������� 
	Bin_Print(2, 1);//FAT����
	Bin_Print(512, 2);//��Ŀ¼���� 
	Bin_Print(SECTORS_PER_FAT, 2);
	Bin_Print(63, 2);
	Bin_Print(0, 4);
	fprintf(fp, "FAT16   ");
	for(int i = 1; i <= 475; i++)
		fprintf(fp, "%c", 0x99);
	Bin_Print(43605, 2);
}

void Bin_Print(long long sum, int n){//done
	long long m;
	for(int i = 1; i <= n; i++){
		m = sum % 256;
		fprintf(fp, "%c", m);
		sum /= 256;
	}
}

long long Bin_Read(int n){//done
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
	printf("format\t\t\t\t\tFormat this disk.\n");
	printf("save\t\t\t\t\tSave this disk.(or you'll lose data)\n");
}
