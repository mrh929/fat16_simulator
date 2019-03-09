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
	int status;
	char byte[BYTES_PER_SECTOR * SECTORS_PER_CLUST + 10]; //��������� 
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
char dir[10000][6];//��������ļ������ֵ�����
int dir_now; 

void Manu_Print();//��ӡ�����˵�
bool Command_Read();//��ȡ���� 

bool Data_ReadOrWrite(const int);//�ļ�����������д���ر�
void Data_Initialize();//��ʼ�������ļ�
void Data_Create_New();//�����µ���FAT16�����ļ�
void Data_Read();//�����ݶ����ڴ� 
void Data_Save();//�����ļ� 
void Data_DBR_Print();//����DBR
void Data_Add_A_File(fi *, int, int);//��clust��ָ��λ�õ��ַ������Ϊ���ļ�����Ϣ 

fi *Filelist_Search_File(char *, bool);//���Ѿ�������ļ���Ѱ�Ҷ�Ӧ���Ƶ��ļ� 
void Filelist_Update(int);//�����ļ��б� 
void Filelist_Destroy();//�ļ��б���� 
void Filelist_Add_File(fi *);//���ļ��б��м���һ��Ԫ�� 
void Filelist_Bubble_Sort();//��ð��������ļ��б����ֵ�������
void Filelist_Print();//��ӡ�����е��ļ� 

void File_Printf_Number(long long, int);//��С����ʽ��ӡ (���ļ�)
long long File_Scanf_Number(int);//��ȡn���ֽڵ�С��������
void File_Printf_String(char *,int);//���ַ������ļ������Ϣ 
void File_Printf_32Bytes(fi *);//���ļ�������ļ�/�ļ��е���Ϣ

void Str_Printf_Number(char *, long long, int);//��С����ʽ��ӡ (���ַ���)
long long Str_Scanf_Number(char *, int);//��ȡn���ֽڵ�С��������
void Str_Scanf_String(char *, int, char *);//�Զ����Ƹ�ʽ��ȡ�ַ��������� 
bool Str_Read_32Bytes(char *, fi *);//��ȡһ���ļ�/�ļ��е���Ϣ
void Str_Printf_32Bytes(char *, fi *);//���ַ���������ļ�/�ļ��е���Ϣ
void Str_Cut_Name_Into_Two(char *, char*, char*);//��һ���ļ����ָ�Ϊ������ 

int Clust_Judge_Status(int); //�жϸôص�״̬ 
void Time_Print(int t);//���ʱ��
void Date_Print(int t);//������� 
int Time_Get();//��ȡ��ǰʱ�� 
int Date_Get();//��ȡ��ǰ���� 

void Dir_Change(char *, bool);//����Ŀ¼λ�� 
void Dir_Print();//��ӡ��ǰĿ¼
int Dir_Find_Empty(int);//��һ��������Ѱ�ҿյ�λ�� 
void Make_a_New_File(fi *, bool);//����һ�������õĿ��ļ� 
void Clust_Clear_File(int);//����ĳ�����ڵ��ļ���������Ϣ 
void Clust_Clear_Dir(int);//����ĳ�����ڵ������ļ��� 

int main(){
	Data_Initialize();
	Manu_Print();
	bool FLAG = FALSE;
	char rt[6] = "root:";
	dir_now = 0;
	strcpy(dir[dir_now], rt);
	do{
		Dir_Print();
		printf(">");
		FLAG = Command_Read();
	}while(FLAG);
	return 0;
	
	
}

bool Command_Read(){
	char str[COMMAND_MAX], a[COMMAND_MAX], b[COMMAND_MAX];
	gets(str);
	sscanf(str, "%s%s", a, b);
	
	if(str[0] == 0) return TRUE;
	
	Filelist_Destroy();
	Filelist_Update(Clust_now);
	
	Filelist_Bubble_Sort();
	if(ISCMD("ls") || ISCMD("dir")){
		printf("\n\n");
		Dir_Print();
		printf(" contains the following files or directories:\n");
		Filelist_Print();
		
	}else if(ISCMD("cd")){
		char temp[20];
		strcpy(temp, b);
		fi *t = Filelist_Search_File(temp, TRUE);
		if(t != NULL && t->dir == TRUE){
			Clust_now = t->clust;
			if(strcmp(b, "..") == 0){
				if(dir_now != 0)
					Dir_Change(b, FALSE);//����
			}
			else if(strcmp(b, ".") != 0)
				Dir_Change(b, TRUE);
		}else
			printf("Invalid directory!\n\n");
			
	}else if(ISCMD("mkdir")){
		fi *t = Filelist_Search_File(b, TRUE);
		if(t != NULL){//�������ļ����Ѿ����ڣ��ͷ��ش���
			printf("Directory already exists!\n");
			return TRUE;
		}else if(strlen(b) >= 8){
			printf("Name too long!\n");
			return TRUE;
		}
		
		int Clust_temp = Clust_now;
		int k = Dir_Find_Empty(Clust_temp);
		while(k == -1){
			if(Clust_Judge_Status(clust[Clust_temp].status) == 0)
				Clust_temp = clust[Clust_temp].status;//����������������ػ��п�λ 
			else{
				int Clust_new = 2;//����������Ѿ������ڿ�λ�ˣ�Ҫ��һ���մأ��ӵڶ����ؿ�ʼ�� 
				while(clust[Clust_new].status != 0) Clust_new++;
				clust[Clust_temp].status = Clust_new;
				clust[Clust_new].status = 0xFFFF;
				Clust_temp = Clust_new;
			}
			k = Dir_Find_Empty(Clust_temp);//����Ѱ�ҿ�λ 
		}
			
		fi *F = (fi*) malloc(sizeof(fi));//��������Ϣ 
		Make_a_New_File(F, TRUE);
		clust[Clust_temp].status = 0xFFFF;
		strcpy(F->name[0], b);//�޸��ļ������� 
		Data_Add_A_File(F, Clust_temp, k);//���ļ�д��� 
		
		fi *temp = (fi*) malloc(sizeof(fi));//����һ����ʱ����
		Make_a_New_File(temp, TRUE);
		
		char *p = clust[F->clust].byte;
		strcpy(temp->name[0], ".");
		temp->clust = F->clust; // .�ļ������ڴ�ű��ļ��е���Ϣ
		Str_Printf_32Bytes(p, temp);
		
		strcpy(temp->name[0], "..");
		temp->clust = Clust_now; // ..�ļ������ڴ����һ���ļ��е���Ϣ
		Str_Printf_32Bytes(p + 32, temp);
		
		free(F);
		free(temp);
		
		
	}else if(ISCMD("create")){
		//�������½��ļ��У����ǲ����½� .��.. 
		fi *t = Filelist_Search_File(b, FALSE);
		if(t != NULL){//�������ļ����Ѿ����ڣ��ͷ��ش���
			printf("Directory already exists!\n");
			return TRUE;
		}
		
		char temp[20];//����һ����ʱ�ַ��� 
		strcpy(temp, b);
		Str_Cut_Name_Into_Two(temp, a, b);
		
		if(strlen(a) >= 8 || strlen(b) >=3){
			printf("Name too long!\n");
			return TRUE;
		}
		
		int Clust_temp = Clust_now;
		int k = Dir_Find_Empty(Clust_temp);
		while(k == -1){
			if(Clust_Judge_Status(clust[Clust_temp].status) == 0)
				Clust_temp = clust[Clust_temp].status;//����������������ػ��п�λ 
			else{
				int Clust_new = 2;//����������Ѿ������ڿ�λ�ˣ�Ҫ��һ���մأ��ӵڶ����ؿ�ʼ�� 
				while(clust[Clust_new].status != 0) Clust_new++;
				clust[Clust_temp].status = Clust_new;
				clust[Clust_new].status = 0xFFFF;
				Clust_temp = Clust_new;
			}
			k = Dir_Find_Empty(Clust_temp);//����Ѱ�ҿ�λ 
		}
			
		fi *F = (fi*) malloc(sizeof(fi));//��������Ϣ 
		Make_a_New_File(F, FALSE);
		clust[Clust_temp].status = 0xFFFF;
		
		
		strcpy(F->name[0], a);//�޸��ļ����� 
		strcpy(F->name[1], b);//�޸��ļ���׺ 
		Data_Add_A_File(F, Clust_temp, k);//���ļ�д���
		
	}else if(ISCMD("rmdir")){
		
		fi *t = Filelist_Search_File(b, TRUE);
		if(t == NULL){
			printf("no such a directory!\n");
			return TRUE;
		}
		
		Clust_Clear_Dir(t->clust);
		
		
		char temp[20];
		strcpy(temp, b);
		Str_Cut_Name_Into_Two(temp, a, b);//���ļ����ָ�һ�� 
		char *st = clust[Clust_now].byte;
		for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++){//Ѱ�ҵ����е��Ǹ��ļ������Ҵ���0xe5��� 
			fi *p = (fi*) malloc(sizeof(fi));
			
			Str_Read_32Bytes(st + i * 32, p);
			if(strcmp(p->name[0], a) == 0 && strcmp(p->name[1], b) == 0 && p->dir == TRUE){
				*(st + i * 32) = 0xE5;
				break;
			}
		}
		
		/*
			ֱ���ҵ��Ǹ��ļ������ȵݹ�ɾ�����ļ����µ���������
			Ȼ���ٰѱ��� ���Ϊɾ�� 
		*/ 
		
	}else if(ISCMD("rm")){
		
		fi *t = Filelist_Search_File(b, FALSE);
		if(t == NULL){
			printf("no such a file!\n");
			return TRUE;
		}
		
		Clust_Clear_File(t->clust);
		
		char *st = clust[Clust_now].byte;
		char temp[20];
		strcpy(temp, b);
		Str_Cut_Name_Into_Two(temp, a, b);
		
		for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++){
			fi *p = (fi*) malloc(sizeof(fi));
			Str_Read_32Bytes(st + i * 32, p);
			
			if(strcmp(p->name[0], a) == 0 && strcmp(p->name[1], b) == 0 && p->dir == FALSE){
				*(st + i * 32) = 0xE5;
				break;
			}
		}		
		
		//������ɾ���ļ��У������õݹ�ɾ�� 
		
	}else if(ISCMD("read")){
		fi *t = Filelist_Search_File(b, FALSE);
		if(t == NULL){
			printf("no such a file!\n");
			return TRUE;
		}
		
		int nxtClust = t->clust;
		bool flag = 1;
		printf("\nThis is what %s contains:", b);
		printf("\n----------------------------\n");
		while(flag == 1){
			for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR; i++){
				if(clust[nxtClust].byte[i] == 0){
					flag = 0;
					break;
				}
				printf("%c",clust[nxtClust].byte[i]);
			}
			if(Clust_Judge_Status(clust[nxtClust].status) == 1)
				flag = 0;
			else
				nxtClust = clust[nxtClust].status;
		}
		printf("\n----------------------------\n\n");
		
		
		/*
			���ݴغ�ֱ����������Ӧ���ļ����Ҷ��� 
		
		*/
		
	}else if(ISCMD("write")){
		
		fi *t = Filelist_Search_File(b, FALSE);
		if(t == NULL){
			printf("no such a file!\n");
			return TRUE;
		}
		
		
		int nxtClust = t->clust;
		bool flag = 1;
		char c;
		while(flag == 1){
			for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR; i++){
				c = getchar();
				if(c == -1){
					flag = 0;
					clust[nxtClust].byte[i] = 0;
					break;
				}else
					clust[nxtClust].byte[i] = c;
			}
			clust[nxtClust].status = 0xFFFF;
			if(flag == 0) break;
			if(Clust_Judge_Status(clust[nxtClust].status) == 0)
				nxtClust = clust[nxtClust].status;
			else{
				int tempClust = 2;
				while(Clust_Judge_Status(clust[tempClust].status) != 2) 
					tempClust++;
				clust[nxtClust].status = tempClust;
				nxtClust = tempClust;
			}
		}
		/*
			��д�ļ��߿��µĴ� 
		
		
		*/
		
	}else if(ISCMD("format")){
		printf("Are you sure to FORMAT this disk? (y/n): ");
		gets(a);
		if(ISCMD("y") || ISCMD("Y")){
			printf("Once again. You'll lose your data, continue? (y/n): ");
			gets(a);
			if(ISCMD("y") || ISCMD("Y")){
				printf("formating...  ");
				Data_ReadOrWrite(2);
				Data_Create_New();
				Data_ReadOrWrite(3);
				Data_ReadOrWrite(1);
				Data_Read();
				Data_ReadOrWrite(3);
				printf("done.\n\n");
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


bool Data_ReadOrWrite(const int t){//done
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
	if(Data_ReadOrWrite(1) == FALSE){
		printf("Disk data doesn't exist. \nCreating a new one...\n");
		Data_Create_New();
	}
	Data_ReadOrWrite(3);//�ر��ļ� 
	Data_ReadOrWrite(1);//ֻ��ģʽ 
	
	Data_Read();
	printf("Data loaded successfully.\n\n");
	Clust_now = 1;
	Data_ReadOrWrite(3);
}

void Data_Read(){
	fseek(fp, BYTES_PER_SECTOR, SEEK_CUR);//����DBR���� 
	for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)//ע�⣬�����Ǵ�0��ʼ�� 
		clust[i].status = (int) File_Scanf_Number(2);//��ȡFAT1 
	fseek(fp, SECTORS_PER_FAT * BYTES_PER_SECTOR, SEEK_CUR);//����FAT2
	fread(clust[1].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//��ȡ��Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fread(clust[i].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//��ȡ������ 
}

void Data_Create_New(){//done
	Data_ReadOrWrite(2);
	Data_DBR_Print();
	
	for(int k = 0; k <= 1; k++){//���FAT���� 
		fprintf(fp, "%c", 0xFF);
		fprintf(fp, "%c", 0xFF);
		for(int i = 1; i <= SECTORS_PER_FAT * BYTES_PER_SECTOR - 2; i++)
			fprintf(fp, "%c", 0);
	}
	
	fi *p = (fi*) malloc(sizeof(fi));
	Make_a_New_File(p, TRUE);
	strcpy(p->name[0], ".");
	File_Printf_32Bytes(p);
	strcpy(p->name[0], "..");
	File_Printf_32Bytes(p);
	free(p);
	
	for(int i = 2; i < (SECTORS_PER_CLUST * CLUST_MAX_COUNT) * BYTES_PER_SECTOR; i++)
		fprintf(fp, "%c", 0);//�ѿ�����ˢΪ0
	Data_ReadOrWrite(3);
}

void Data_Save(){
	Data_ReadOrWrite(2);
	Data_DBR_Print();
	for(int j = 0; j <= 1; j++)
		for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)
			File_Printf_Number(clust[i].status, 2);//��ӡFAT����ӡ����
	fwrite(clust[1].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//�����Ŀ¼ 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fwrite(clust[i].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//����������
	Data_ReadOrWrite(3);
}

fi *Filelist_Search_File(char *name, bool IsDir){
	char a[20], b[20];
	Str_Cut_Name_Into_Two(name, a, b);
	
	for(fi *p = HEAD; p != NULL; p = p->next)
		if(strcmp(a, p->name[0]) == 0 && strcmp(b, p->name[1]) == 0 && p->dir == IsDir)
			return p;//�ҵ��� 
			
			
	return NULL;//û�ҵ� 
}

void Filelist_Print(){
	printf("\n\n");
	for(fi *p = HEAD; p != NULL; p = p->next){
		Date_Print(p->date);
		printf("  ");
		Time_Print(p->time);
		printf("  ");
		printf("%s\t\t", p->dir == TRUE?"<DIR> ":"<FILE>");
		printf("%s",p->name[0]);
		if(!p->dir && p->name[1][0] != 0) printf(".%s", p->name[1]);
		printf("\n");
	}
	printf("\n\n");
}

void Filelist_Update(int id){
	if(id <= 0) return;
	
	fi t;
	for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(Str_Read_32Bytes(clust[id].byte + 32 * i, &t)){//�����ȡ�ɹ���������һ�����ڴ������ļ� 
			fi *p = (fi *) malloc(sizeof(fi));
			*p = t;
			p->next = NULL;
			Filelist_Add_File(p);
		}
			
	
	if(!Clust_Judge_Status(clust[id].status))//�������غ��滹���ļ� �ͼ�����ȡ 
		Filelist_Update(clust[id].status);
	else return;
}

void Filelist_Destroy(){
	FileCnt = 0;
	fi *p;
	for(fi *pre = HEAD; pre != NULL; pre= p){
		p = pre->next;
		free(pre);
	}
	HEAD = NULL;
}

void Filelist_Add_File(fi *p){
	p->next = HEAD;//ֱ����ͷ��������½�� 
	HEAD = p;
	FileCnt++;
}

int Clust_Judge_Status(int t){
	if(t >= 0x2 && t <= 0xFFEF) return 0;//�������ӵĴ� 
	else if(t == 0xFFFF) return 1;//�ļ�����
	else return 2;//�մ� 
}

bool Str_Read_32Bytes(char *str, fi *a){//done
	Str_Scanf_String(str, 8, a->name[0]);
	Str_Scanf_String(str + 8, 3, a->name[1]);
//�������0xE5��Ϊ��-27 ��ascii����ͬ�� 
	if(a->name[0][0] == -27 || a->name[0][0] == 0){//������ļ��Ѿ���ɾ�����߲����� 
		fseek(fp, 21, SEEK_CUR);//ֱ������ʣ��32 - 11 = 21���ֽ�
		return FALSE;
	}
	str = str + 11;
	if(*str++ == 0x10) //���ļ��Ǹ�Ŀ¼ 
		a->dir = TRUE; 
	else a->dir = FALSE;
	
	str += 10;//����10���������ֽ� 
	
	a->time = Str_Scanf_Number(str, 2);//��ȡʱ��
	str += 2;
	a->date = Str_Scanf_Number(str, 2);//��ȡ����
	str += 2;
	a->clust = Str_Scanf_Number(str, 2);//��ȡ�״غ�
	str += 2;
	a->len = Str_Scanf_Number(str, 4);//��ȡ�ļ���С 
	str += 4;
	
	return TRUE;
}

void File_Printf_32Bytes(fi *t){
	File_Printf_String(t->name[0], 8);
	File_Printf_String(t->name[1], 3);
	if(t->dir == TRUE)
		fprintf(fp, "%c", 0x10);
	else
		fprintf(fp, "%c", 0x20);
	for(int i = 1; i <= 10; i++)
		fprintf(fp, "%c", 0xFF);
	File_Printf_Number(t->time, 2);
	File_Printf_Number(t->date, 2);
	File_Printf_Number(t->clust, 2);
	File_Printf_Number(t->len, 4);
}

void Str_Printf_32Bytes(char *str, fi *t){
	strncpy(str, t->name[0], 8);
	str += 8;
	strncpy(str, t->name[1], 3);
	str += 3;
	
	if(t->dir == TRUE)
		*str++ = 0x10;
	else
		*str++ = 0x20;
			
	for(int i = 1; i <= 10; i++)
		*str++ = 0xFF;
	
	Str_Printf_Number(str, t->time, 2);
	str += 2;
	Str_Printf_Number(str, t->date, 2);
	str += 2;
	Str_Printf_Number(str, t->clust, 2);
	str += 2;
	Str_Printf_Number(str, t->len, 4);
}

void Str_Cut_Name_Into_Two(char *name, char *a, char *b){
	char temp[30];
	strcpy(temp, name);
	char *t = temp;
	
	if(strcmp(t, "..") == 0 || strcmp(t, ".") == 0)
		strcpy(a, temp);
	else{
		while(*t != 0 && *t != '.') t++;
		if(*t == 0) *(t + 1) = 0;
		*t = 0;
		strcpy(a, temp);
		strcpy(b, t + 1);
	}
}

void Filelist_Bubble_Sort(){
	for(int i = 0; i < FileCnt; i++)
		for(fi *p = HEAD; p->next != NULL; p = p->next)
			if(strcmp(p->name[0], p->next->name[0]) > 0){
				fi _a, _b, *pnx;//��������
				pnx = p->next;
				_a = *p;
				_b = *pnx;
				
				*p = _b;
				*pnx = _a;
				p->next = _a.next;
				pnx->next = _b.next;
			}
}

void Data_DBR_Print(){//done
	fprintf(fp, "%c%c%c",0xEB ,0x3C ,0x90 );
	fprintf(fp, "ByMrh929");
	File_Printf_Number(BYTES_PER_SECTOR, 2);
	File_Printf_Number(SECTORS_PER_CLUST, 1);
	File_Printf_Number(RESERVED_SECTORS, 2);//���������� 
	File_Printf_Number(2, 1);//FAT����
	File_Printf_Number(512, 2);//��Ŀ¼���� 
	File_Printf_Number(SECTORS_PER_FAT, 2);
	File_Printf_Number(63, 2);
	File_Printf_Number(0, 4);
	fprintf(fp, "FAT16   ");
	for(int i = 1; i <= 475; i++)
		fprintf(fp, "%c", 0x99);
	File_Printf_Number(43605, 2);
}

void Data_Add_A_File(fi *F, int id, int k){
	char *p = clust[id].byte;
	p += k * 32;//��ָ���ƶ�����Ҫ�޸ĵ��Ǹ��ļ���Ϣ֮ǰ 
		
	int newclust = 2;
	while(clust[newclust].status != 0) newclust++;//Ѱ��һ���أ���������ļ�����Ϣ 
	clust[newclust].status = 0xFFFF;
	memset(clust[newclust].byte, 0, sizeof(clust[newclust].byte));
	F->clust = newclust;
	
	Str_Printf_32Bytes(p, F);
	
}

void Str_Printf_Number(char *str, long long sum, int n){
	long long m;
	for(int i = 0; i < n; i++){
		m = sum % 256;
		*str++ = m;//���һλ������ָ����� 
		sum /= 256;
	}
}

long long Str_Scanf_Number(char *str, int n){//done
	long long sum = 0;
	char t;
	for(int i = 0; i < n ; i++){
		t = *str++;	//��ȡһλ����ָ����� 
		sum += t * (1 << (i * 8));
	}
	return sum;
}

void File_Printf_Number(long long sum, int n){//done
	long long m;
	for(int i = 0; i < n; i++){
		m = sum % 256;
		fprintf(fp, "%c", m);
		sum /= 256;
	}
}

long long File_Scanf_Number(int n){//done
	long long sum = 0;
	char t;
	for(int i = 0; i < n ; i++){ 
		fscanf(fp, "%c", &t);
		sum += t * (1 << (i * 8));
	}
	return sum;
}

void Str_Scanf_String(char *str, int t, char *dest){//done
	int i = 0;
	while(i < t){
		if(*(str + i) != ' ' && *(str + i) != 0){
			*(dest + i) = *(str + i);
			i++;
		}else break;
	}
	*(dest + i) = 0;
}

void File_Printf_String(char *str, int n){
	for(int i = 0; i < n; i++)
		fprintf(fp, "%c", *(str + i));
}

void Time_Print(int t){//done
	int h, m, s;
	s = (t & 0x1F) * 2;//ȡ����λ 
	t >>= 5;
	m = t & 0x3F;
	t >>= 6;
	h = t & 0x1F;
	printf("%02d:%02d:%02d", h, m, s);
}

void Date_Print(int t){//done
	int y, m, d;
	d = t & 0x1F;
	t >>= 5;
	m = t & 0xF;
	t >>= 4;
	y = (t & 0xFF) + 1980;
	printf("%04d/%02d/%02d", y, m, d);
	
}

int Time_Get(){//done
	time_t t;
    struct tm *lt;
	time (&t);//��ȡUnixʱ�����
    lt = localtime (&t);//תΪʱ��ṹ��
	return (lt->tm_hour * 2048) + (lt->tm_min * 32) + (lt->tm_sec / 2);
}

int Date_Get(){//done
	time_t t;
    struct tm *lt;
	time (&t);//��ȡUnixʱ�����
    lt = localtime (&t);//תΪʱ��ṹ��
    lt->tm_year += 1900;
    lt->tm_mon ++;//0����1�� 
	return (lt->tm_year - 1980) * 512 + (lt->tm_mon * 32) + (lt->tm_mday);
}

void Dir_Change(char *p, bool NEW){
	if(NEW == FALSE)
		dir_now--;
	else
		strcpy(dir[++dir_now], p);
}

void Dir_Print(){
	printf("\\");
	for(int i = 0; i <= dir_now; i++)
		printf("%s\\",dir[i]);
}

int Dir_Find_Empty(int id){
	for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(clust[id].byte[i * 32] == -27 || clust[id].byte[i * 32] == 0)
			return i;
	return -1;//û�ҵ�һ����λ 
}

void Make_a_New_File(fi *p, bool IsDir){
	p->clust = 1;
	p->len = 0;
	p->dir = IsDir;
	p->time = Time_Get();
	p->date = Date_Get();
	p->name[1][0] = 0;
}

void Clust_Clear_File(int id){
	if(Clust_Judge_Status(clust[id].status) == 0)//�������ӵĴ� 
		Clust_Clear_File(clust[id].status);
	clust[id].status = 0;//���Ϊδʹ�� 
	return;
}

void Clust_Clear_Dir(int id){
	//����������ļ���������Щ���ļ���Ŀ¼��������ɾ����Ȼ����ɾ������ 
	
	if(id <= 0) return;
	
	fi t;
	for(int i = 2; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(Str_Read_32Bytes(clust[id].byte + 32 * i, &t)){//�����ȡ�ɹ���������һ�����ڴ������ļ� 
			if(t.dir == TRUE)
				Clust_Clear_Dir(t.clust);//�ݹ�ɾ����Ŀ¼
			else
				Clust_Clear_File(t.clust);//�ݹ�ɾ�����ļ�
			
			*(clust[id].byte + 32 * i) = 0xE5;//���Ϊɾ�� 
		}
			
	if(Clust_Judge_Status(clust[id].status) == 0)//�������غ��滹���ļ� �ͼ�����ȡ 
		Clust_Clear_Dir(clust[id].status);
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
