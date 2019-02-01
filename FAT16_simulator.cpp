#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#define TRUE 1
#define FALSE 0
#define ISCMD(t) strcmp(a, t) == 0
#define COMMAND_MAX 100
#define SECTORS_PER_CLUST 1//每簇扇区数
#define BYTES_PER_SECTOR 512//每扇区字节数
#define SECTORS_PER_FAT 128//每FAT扇区数
#define RESERVED_SECTORS 1//保留扇区数
#define SECTORS_ROOT SECTORS_PER_CLUST//根目录的扇区数
#define CLUST_MAX_COUNT (SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1) //簇的总个数 
#define ROOT (RESERVED_SECTORS + SECTORS_PER_FAT * 2) * BYTES_PER_SECTOR//根目录的位置
#define CLUST_SECOND ROOT + SECTORS_ROOT * BYTES_PER_SECTOR//第二个簇的位置 
FILE *fp;

struct clu{//一个簇 
	int status;
	char byte[BYTES_PER_SECTOR * SECTORS_PER_CLUST + 10]; //簇里的扇区 
}clust[CLUST_MAX_COUNT];

struct fi{//一个文件 
	bool dir;
	char name[2][20];
	int time;//修改时间 
	int date;//修改日期 
	int clust;//首簇号
	long long len;//文件大小
	fi *next;//下一个文件 
}file[1 + CLUST_MAX_COUNT * SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32];
int FileCnt, Clust_now;//当前文件的总个数与当前的簇 
fi *HEAD;//头指针 
char *dir[10000];//存放所有文件夹名字的数组
int dir_now; 

void Manu_Print();//打印帮助菜单
bool Command_Read();//读取命令 

bool Data_ReadOrWrite(const int);//文件操作，读、写、关闭
void Data_Initialize();//初始化数据文件
void Data_Create_New();//创建新的类FAT16数据文件
void Data_Read();//将数据读入内存 
void Data_Save();//保存文件 
void Data_DBR_Print();//创建DBR

fi *Filelist_Search_File(char *);//在已经读入的文件中寻找对应名称的文件 
void Filelist_Update(int);//更新文件列表 
void Filelist_Destroy();//文件列表清空 
void Filelist_Add_File(fi *);//在文件列表中加入一个元素 
void Filelist_Bubble_Sort();//用冒泡排序把文件列表按照字典序排序
void Filelist_Print();//打印出所有的文件 

void File_Printf_Number(long long, int);//以小端序方式打印 (对文件)
long long File_Scanf_Number(int);//读取n个字节的小端序数字
void File_Printf_String(char *,int);//以字符串向文件输出信息 
void File_Printf_32Bytes(fi *);//向文件中输出文件/文件夹的信息

void Str_Printf_Number(char *, long long, int);//以小端序方式打印 (对字符串)
long long Str_Scanf_Number(char *, int);//读取n个字节的小端序数字
void Str_Scanf_String(char *, int, char *);//以二进制格式读取字符串的内容 
bool Str_Read_32Bytes(char *, fi *);//读取一个文件/文件夹的信息
void Str_Printf_32Bytes(char *, fi *);//向字符串中输出文件/文件夹的信息

bool Clust_Judge_Status(int); //判断该簇的状态 
void Time_Print(int t);//输出时间
void Date_Print(int t);//输出日期 
int Time_Get();//获取当前时间 
int Date_Get();//获取当前日期 

void Dir_Change(char *, bool);//更改目录位置 
void Dir_Print();//打印当前目录
int Dir_Find_Empty(int);//在一个簇里面寻找空的位置 
void Make_a_New_File(fi *, bool);//返回一个创建好的空文件 

int main(){
	Data_Initialize();
	Manu_Print();
	bool FLAG = FALSE;
	char rt[6] = "root:";
	dir_now = 0;
	dir[dir_now] = rt;
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
		fi *t = Filelist_Search_File(b);
		if(t != NULL && t->dir == TRUE){
			Clust_now = t->clust;
			if(strcmp(b, "..") == 0){
				if(dir_now != 0)
					Dir_Change(b, FALSE);//回退
			}
			else if(strcmp(b, ".") !=0)
				Dir_Change(a, FALSE);
		}else
			printf("Invalid directory!\n\n");
			
	}else if(ISCMD("mkdir")){
		fi *t = Filelist_Search_File(b);
		if(t != NULL){//如果这个文件夹已经存在，就返回错误
			printf("Directory already exists!");
			return TRUE;
		}else{
			int nxtClust = Clust_now;
			int k = Dir_Find_Empty(nxtClust);
			while(k == -1){
				if(Clust_Judge_Status(clust[nxtClust].status) == 0)
					nxtClust = clust[nxtClust].status;//如果这个链表的其他簇还有空位 
				else{
					nxtClust = 2;//这个链表中已经不存在空位了，要找一个空簇，从第二个簇开始找 
					while(clust[nxtClust].status != 0) nxtClust++;
					
				}
				k = Dir_Find_Empty(nxtClust);//重新寻找空位 
			}
			
			char *p = clust[nxtClust].byte;
			p += k * 32;//找到应该插入文件信息的位置 
			
			int newclust = 2;
			while(clust[newclust].status != 0) newclust++;//寻找一个簇，用来存放文件夹内文件的信息 
			
			fi *F = (fi*) malloc(sizeof(fi));
			Make_a_New_File(F, TRUE);
			F->clust = newclust;
			clust[newclust].status = 0xFFFF;
			strcpy(F->name[0], b);
			
			Str_Printf_32Bytes(str, F);
			free(F);
			
			/*
					接下来要在开辟的空间中 填充文件夹信息，并且建立. 和..文件夹 
			
			*/
			
		}
		
		
	}else if(ISCMD("create")){
		//类似于新建文件夹，但是不用新建 .和.. 
		
		
	}else if(ISCMD("rmdir")){
		/*
			直接找到那个文件，首先递归删除该文件夹下的所有内容
			然后再把本身 标记为删除 
		*/ 
		
	}else if(ISCMD("rm")){
		//类似于删除文件夹，但不用递归删除 
		
	}else if(ISCMD("read")){
		/*
			根据簇号直接搜索到对应的文件并且读入 
		
		*/
		
	}else if(ISCMD("write")){
		/*
			边写文件边开新的簇 
		
		
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
	return TRUE;//继续循环 
}


bool Data_ReadOrWrite(const int t){//done
	switch(t){
		case 1: //只读 
			fp = fopen("fat16_data.bin", "rb");
			return fp != NULL;
		case 2: //可写 
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
	Data_ReadOrWrite(3);//关闭文件 
	Data_ReadOrWrite(1);//只读模式 
	
	Data_Read();
	printf("Data loaded successfully.\n\n");
	Clust_now = 1;
	Data_ReadOrWrite(3);
}

void Data_Read(){
	fseek(fp, BYTES_PER_SECTOR, SEEK_CUR);//跳过DBR区域 
	for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)//注意，这里是从0开始读 
		clust[i].status = (int) File_Scanf_Number(2);//读取FAT1 
	fseek(fp, SECTORS_PER_FAT * BYTES_PER_SECTOR, SEEK_CUR);//跳过FAT2
	fread(clust[1].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//读取根目录 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fread(clust[i].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//读取数据区 
}

void Data_Create_New(){//done
	Data_ReadOrWrite(2);
	Data_DBR_Print();
	
	for(int k = 0; k <= 1; k++){//填充FAT区域 
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
		fprintf(fp, "%c", 0);//把空扇区刷为0
	Data_ReadOrWrite(3);
}

void Data_Save(){
	Data_ReadOrWrite(2);
	Data_DBR_Print();
	for(int j = 0; j <= 1; j++)
		for(int i = 0; i <= -1 + SECTORS_PER_FAT * BYTES_PER_SECTOR >> 1; i++)
			File_Printf_Number(clust[i].status, 2);//打印FAT表，打印两份
	fwrite(clust[1].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_ROOT, fp);//保存根目录 
	for(int i = 2; i <= CLUST_MAX_COUNT; i++)
		fwrite(clust[i].byte, sizeof(char), BYTES_PER_SECTOR * SECTORS_PER_CLUST, fp);//保存数据区
	Data_ReadOrWrite(3);
}

fi *Filelist_Search_File(char *name){
	char *a = name, *b;
	if(strcmp(a, "..") == 0 || strcmp(a, ".") == 0){
		b = (char*) malloc(sizeof(char));
		*b = 0;
	}else{
		while(*name != '.' && *name != 0) name++;
		if(*name != 0){
			*name = 0;
			b = name + 1;
		}
	}
	
	for(fi *p = HEAD; p != NULL; p = p->next)
		if(strcmp(a, p->name[0]) == 0 && strcmp(b, p->name[1]) == 0)
			return p;//找到了 
	return NULL;//没找到 
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
		if(Str_Read_32Bytes(clust[id].byte + 32 * i, &t)){//如果读取成功，就申请一段新内存来存文件 
			fi *p = (fi *) malloc(sizeof(fi));
			*p = t;
			Filelist_Add_File(p);
		}
			
	
	if(!Clust_Judge_Status(clust[id].status))//如果这个簇后面还有文件 就继续读取 
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
	p->next = HEAD;//直接在头部添加上新结点 
	HEAD = p;
	FileCnt++;
}

bool Clust_Judge_Status(int t){
	if(t >= 0x2 && t <= 0xFFEF) return 0;//还有连接的簇 
	else if(t == 0xFFFF) return 1;//文件结束
	else return 2;//空簇 
}

bool Str_Read_32Bytes(char *str, fi *a){//done
	Str_Scanf_String(str, 8, a->name[0]);
	Str_Scanf_String(str + 8, 3, a->name[1]);

	if(a->name[0][0] == 0xE5 || a->name[0][0] == 0){//如果该文件已经被删除或者不存在 
		fseek(fp, 21, SEEK_CUR);//直接跳过剩下32 - 11 = 21个字节
		return FALSE;
	}
	str = str + 11;
	if(*str++ == 0x10) //该文件是个目录 
		a->dir = TRUE; 
	else a->dir = FALSE;
	
	str += 10;//跳过10个无意义字节 
	
	a->time = Str_Scanf_Number(str, 2);//读取时间
	str += 2;
	a->date = Str_Scanf_Number(str, 2);//读取日期
	str += 2;
	a->clust = Str_Scanf_Number(str, 2);//读取首簇号
	str += 2;
	a->len = Str_Scanf_Number(str, 4);//读取文件大小 
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

void Filelist_Bubble_Sort(){
	for(int i = 0; i < FileCnt; i++)
		for(fi *p = HEAD; p->next != NULL; p = p->next)
			if(strcmp(p->name[0], p->next->name[0]) > 0){
				fi _a, _b, *pnx;//交换函数
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
	File_Printf_Number(RESERVED_SECTORS, 2);//保留扇区数 
	File_Printf_Number(2, 1);//FAT数量
	File_Printf_Number(512, 2);//根目录项数 
	File_Printf_Number(SECTORS_PER_FAT, 2);
	File_Printf_Number(63, 2);
	File_Printf_Number(0, 4);
	fprintf(fp, "FAT16   ");
	for(int i = 1; i <= 475; i++)
		fprintf(fp, "%c", 0x99);
	File_Printf_Number(43605, 2);
}

void Str_Printf_Number(char *str, long long sum, int n){
	long long m;
	for(int i = 0; i < n; i++){
		m = sum % 256;
		*str++ = m;//输出一位数并且指针后移 
		sum /= 256;
	}
}

long long Str_Scanf_Number(char *str, int n){//done
	long long sum = 0;
	char t;
	for(int i = 0; i < n ; i++){
		t = *str++;	//读取一位并且指针后移 
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
	s = (t & 0x1F) * 2;//取后五位 
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
    struct tm * lt;
	time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
	return (lt->tm_hour * 2048) + (lt->tm_min * 32) + (lt->tm_sec / 2);
}

int Date_Get(){//done
	time_t t;
    struct tm * lt;
	time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
    lt->tm_year += 1900;
    lt->tm_mon ++;//0代表1月 
	return (lt->tm_year - 1980) * 512 + (lt->tm_mon * 32) + (lt->tm_mday);
}

void Dir_Change(char *p, bool NEW){
	if(NEW == FALSE)
		dir_now--;
	else
		dir[++dir_now] = p;
}

void Dir_Print(){
	printf("\\");
	for(int i = 0; i <= dir_now; i++)
		printf("%s\\",dir[i]);
}

int Dir_Find_Empty(int id){
	for(int i = 0; i < SECTORS_PER_CLUST * BYTES_PER_SECTOR / 32; i++)
		if(clust[id].byte[i * 32] == 0xE5 || clust[id].byte[i * 32] == 0)
			return i;
	return -1;//没找到一个空位 
}

void Make_a_New_File(fi *p, bool IsDir){
	p->clust = 1;
	p->len = 0;
	p->dir = IsDir;
	p->time = Time_Get();
	p->date = Date_Get();
	p->name[1][0] = 0;
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
