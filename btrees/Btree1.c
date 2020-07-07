//Given a list of student records in a CSV file, 1st task is to construct a B+tree in a binary file where all the leaf nodes are the student 
//records and non leaf nodes hold the keys and offsets of the records which are present in the leaf nodes. 2nd task is to query the B+Tree 
//which is in the binary file based on the student roll numbers

#include<stdio.h>

struct student
{
	int type;
	int rollno;
	char name[28];
	char college[28];
};

struct nonLeafNode
{
	int type;
	int offset[3];
	int key[2];
	char unused[40];
};

struct student st;
struct nonLeafNode NLN,nln,nlnk;
int leaf_Count = 0;
int nonLeafCount = 0;

void menu()
{
	printf("1-rollnumber checking\n");
	printf("2-students in between given rollnumber range scan\n");
	printf("0-exit\n");
}

int getKey(FILE *fp,int offset)
{
	int key;
	fseek(fp, offset, SEEK_SET);
	fread(&nlnk, sizeof(struct nonLeafNode), 1, fp);
	if (nlnk.type == 0)
	{
		fseek(fp, offset, SEEK_SET);
		fread(&st, sizeof(struct student), 1, fp);
		return st.rollno;
	}
	else
	{
		key=getKey(fp, nlnk.offset[0]);
	}
	return key;
}

int nonLeafCreation(FILE *fp,int startOffset,int endOffset)
{
	int tempOffset,nonLeafOffset;
	nonLeafCount = 0;
	nonLeafOffset = endOffset;
	fseek(fp, startOffset, SEEK_SET);
	while (ftell(fp)<endOffset)
	{
		nonLeafCount++;
		memset(&NLN, 0, sizeof(struct nonLeafNode));
		NLN.type = 1;
		NLN.offset[0] = ftell(fp);
		fread(&nln, sizeof(struct nonLeafNode), 1, fp);
		if (ftell(fp) == endOffset)
		{
			NLN.key[0] = getKey(fp,nln.offset[0]);
			fseek(fp, nonLeafOffset, SEEK_SET);
			fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
			nonLeafOffset = ftell(fp);
			break;
		}
		NLN.offset[1] = ftell(fp);
		fread(&nln, sizeof(struct nonLeafNode), 1, fp);
		tempOffset = ftell(fp);
		NLN.key[0] = getKey(fp,nln.offset[0]);
		if (tempOffset == endOffset)
		{
			fseek(fp, nonLeafOffset, SEEK_SET);
			fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
			nonLeafOffset = ftell(fp);
			break;
		}
		fseek(fp, tempOffset, SEEK_SET);
		NLN.offset[2] = ftell(fp); 
		fread(&nln, sizeof(struct nonLeafNode), 1, fp);
		tempOffset = ftell(fp);
		NLN.key[1] = getKey(fp,nln.offset[0]);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	return nonLeafOffset;
}

void nonLeafCreation1()
{
	int nonLeafOffset = leaf_Count * 64;
	int tempOffset,startOffset,endOffset;
	FILE *fp;
	fp = fopen("B+tree1.bin", "rb+");
	while ( ftell(fp)< leaf_Count*64)
	{
		nonLeafCount++;
		memset(&NLN, 0, sizeof(struct nonLeafNode));
		NLN.type = 1;
		NLN.offset[0] = ftell(fp);
		fread(&st, sizeof(struct student), 1, fp);
		if (ftell(fp)==leaf_Count*64)
		{
			NLN.key[0] = st.rollno;
			fseek(fp, nonLeafOffset, SEEK_SET);
			fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
			nonLeafOffset = ftell(fp);
			break;
		}
		NLN.offset[1] = ftell(fp);
		fread(&st, sizeof(struct student), 1, fp);
		NLN.key[0] = st.rollno;
		if (ftell(fp)==leaf_Count*64)
		{
			fseek(fp, nonLeafOffset, SEEK_SET);
			fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
			nonLeafOffset = ftell(fp);
			break;
		}
		NLN.offset[2] = ftell(fp);
		fread(&st, sizeof(struct student), 1, fp);
		NLN.key[1] = st.rollno;
		tempOffset = ftell(fp);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLN, sizeof(struct nonLeafNode), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	endOffset = nonLeafOffset;
	startOffset = leaf_Count * 64;
	while (nonLeafCount != 1)
	{
		tempOffset = endOffset;
		endOffset = nonLeafCreation(fp, startOffset, endOffset);
		startOffset = tempOffset;
	}
	fclose(fp);
}

void fileReading_and_writing(FILE *fp)
{
	FILE *fp1;
	fp1 = fopen("B+Tree1.bin", "wb");
	memset(&st, 0, sizeof(struct student));
	while ((fscanf(fp, "%d,%[^,],%[^\n]s", &st.rollno, &st.name, &st.college)) != EOF)
	{
		st.type = 0;
		fwrite(&st, sizeof(struct student), 1, fp1);
		memset(&st, 0, sizeof(struct student));
		leaf_Count++;
	}
	fclose(fp1);
	nonLeafCreation1();
}

void searchByRollnumber(int rollnum)
{
	int tempOffset;
	FILE *fp;
	fp = fopen("B+Tree1.bin", "rb");
	fseek(fp, -64, SEEK_END);
	tempOffset = ftell(fp);
	fread(&NLN, sizeof(struct nonLeafNode), 1, fp);
	while (1)
	{
		if (NLN.type == 0)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&st, sizeof(struct student), 1, fp);
			if (st.rollno == rollnum)
				printf("%s\t%s\n", st.name, st.college);
			else
				printf("student with that rollnumber doesnot exist\n");
			break;
		}
		else if (rollnum < NLN.key[0]||NLN.offset[1]==0)
		{
			tempOffset = NLN.offset[0];
		}
		else if (rollnum >= NLN.key[0] && rollnum < NLN.key[1]||NLN.key[1]==0&& rollnum >= NLN.key[0])
		{
			tempOffset = NLN.offset[1];
		}
		else
		{
			tempOffset = NLN.offset[2];
		}
		fseek(fp, tempOffset, SEEK_SET);
		fread(&NLN, sizeof(struct nonLeafNode), 1, fp);
	}
	fclose(fp);
}

void rollnumberRangeScan(int min, int max)
{
	int tempOffset,found=0;
	FILE *fp;
	fp = fopen("B+Tree1.bin", "rb");
	fseek(fp, -64, SEEK_END);
	tempOffset = ftell(fp);
	fread(&NLN, sizeof(struct nonLeafNode), 1, fp);
	while (1)
	{
		if (NLN.type == 0)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&st, sizeof(struct student), 1, fp);
			if (st.rollno >= min&&st.rollno<=max)
			{
				found = 1;
				printf("%d\t%s\t%s\n", st.rollno, st.name, st.college);
			}
			tempOffset = ftell(fp);
			break;
		}
		else if (min < NLN.key[0] || NLN.offset[1] == 0)
		{
			tempOffset = NLN.offset[0];
		}
		else if (min >= NLN.key[0] && min < NLN.key[1] || NLN.key[1] == 0 && min >= NLN.key[0])
		{
			tempOffset = NLN.offset[1];
		}
		else
		{
			tempOffset = NLN.offset[2];
		}
		fseek(fp, tempOffset, SEEK_SET);
		fread(&NLN, sizeof(struct nonLeafNode), 1, fp);
	}
	while (ftell(fp) < leaf_Count * 64)
	{
		fseek(fp, tempOffset, SEEK_SET);
		fread(&st, sizeof(struct student), 1, fp);
		if (st.rollno >= min&&st.rollno <= max)
		{
			found = 1;
			printf("%d\t%s\t%s\n",st.rollno,st.name,st.college);
		}
		if (st.rollno > max)
			break;
		tempOffset = ftell(fp);
	}
	if (found == 0)
	{
		printf("there are no students in the given range\n");
	}
	fclose(fp);
}

int main()
{
	int rollnum,max,ch;
	FILE *fp;
	fp = fopen("B+tree1.csv", "r");
	if (fp == NULL)
	{
		printf("unable to open the file\n");
		return -1;
	}
	fileReading_and_writing(fp);
	fclose(fp);
	printf("this program is about insertion of Btree leaf and nonleaf nodes into a file\n");
	do
	{
		menu();
		printf("enter your choice\n");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
			printf("enter your roll number\n");
			scanf("%d", &rollnum);
			searchByRollnumber(rollnum);
			break;
		case 2:
			printf("enter the rollnumbers in ascending order to print list of students\n");
			scanf("%d%d", &rollnum, &max);
			rollnumberRangeScan(rollnum, max);
			break;
		}
	} while (ch!=0);
	return 0;
}