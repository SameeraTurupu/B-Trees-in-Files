//Given a CSV file of student records, construct a B+ tree in a binary file where a table sector should be written in the beginning of the file
//which will hold the root pages and table ids of different records. For now only one( student record ) type of record is given in the input file.
//The root ( non leaf page ) offset need to be stored in the table sector.

#include<stdio.h>

struct tableSector
{
	int tableid[8];
	int rootpageid[8];
};

struct student
{
	int rollno;
	char name[24];
	char college[20];
};

struct leafPage
{
	int pagetype;
	struct student stud[10];
	char unused[28];
};

struct nonLeafPage
{
	int pagetype;
	int key[63];
	int offset[64];
};

struct student st;
struct nonLeafPage NLP, nlp, nlp2;
struct leafPage LP;
struct tableSector ts;

int leafPageCount = 0;
int nonLeafCount = 0;

int getKey(FILE *fp, int offset)
{
	int key;
	fseek(fp, offset, SEEK_SET);
	fread(&nlp2, sizeof(struct nonLeafPage), 1, fp);
	if (nlp2.pagetype == 0)
	{
		fseek(fp, offset, SEEK_SET);
		fread(&LP, sizeof(struct leafPage), 1, fp);
		return LP.stud[0].rollno;
	}
	else
	{
		key = getKey(fp, nlp2.offset[0]);
	}
	return key;
}


int nonLeafPageInsertion(FILE *fp, int startOffset, int endOffset)
{
	int tempOffset, nonLeafOffset, count, brek = 0;
	nonLeafCount = 0;
	nonLeafOffset = endOffset;
	fseek(fp, startOffset, SEEK_SET);
	while (ftell(fp)<endOffset)
	{
		nonLeafCount++;
		memset(&NLP, 0, sizeof(struct nonLeafPage));
		NLP.pagetype = 1;
		count = 0;
		while (count < 64)
		{
			memset(&nlp, 0, sizeof(struct leafPage));
			if (count == 0)
			{
				NLP.offset[count] = ftell(fp);
				fread(&nlp, sizeof(struct nonLeafPage), 1, fp);
				if (ftell(fp) == endOffset)
				{
					NLP.key[count] = getKey(fp, NLP.offset[count]);
					fseek(fp, nonLeafOffset, SEEK_SET);
					fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
					nonLeafOffset = ftell(fp);
					brek = 1;
					break;
				}
			}
			else
			{
				NLP.offset[count] = ftell(fp);
				fread(&nlp, sizeof(struct nonLeafPage), 1, fp);
				tempOffset = ftell(fp);
				NLP.key[count - 1] = getKey(fp, NLP.offset[count]);
				fseek(fp, tempOffset, SEEK_SET);
				if (ftell(fp) == endOffset)
				{
					fseek(fp, nonLeafOffset, SEEK_SET);
					fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
					nonLeafOffset = ftell(fp);
					brek = 1;
					break;
				}
			}
			count++;
		}
		if (brek == 1)
		{
			break;
		}
		tempOffset = ftell(fp);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	return nonLeafOffset;
}


void nonLeafPageInsertion1()
{
	int nonLeafOffset = (leafPageCount * 512)+64;
	int tempOffset, startOffset, endOffset, count, brek = 0;
	FILE *fp;
	fp = fopen("Btree3.bin", "rb+");
	fseek(fp, 64, SEEK_SET);
	while (ftell(fp) < (leafPageCount * 512)+64)
	{
		nonLeafCount++;
		memset(&NLP, 0, sizeof(struct nonLeafPage));
		NLP.pagetype = 1;
		count = 0;
		while (count < 64)
		{
			memset(&LP, 0, sizeof(struct leafPage));
			if (count == 0)
			{
				NLP.offset[count] = ftell(fp);
				fread(&LP, sizeof(struct leafPage), 1, fp);
				if (ftell(fp) == (leafPageCount * 512)+64)
				{
					NLP.key[count] = LP.stud[10].rollno + 1;
					fseek(fp, nonLeafOffset, SEEK_SET);
					fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
					nonLeafOffset = ftell(fp);
					brek = 1;
					break;
				}
			}
			else
			{
				NLP.offset[count] = ftell(fp);
				fread(&LP, sizeof(struct leafPage), 1, fp);
				NLP.key[count - 1] = LP.stud[0].rollno;
				if (ftell(fp) == (leafPageCount * 512)+64)
				{
					fseek(fp, nonLeafOffset, SEEK_SET);
					fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
					nonLeafOffset = ftell(fp);
					brek = 1;
					break;
				}
			}
			count++;
		}
		if (brek == 1)
			break;
		tempOffset = ftell(fp);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	endOffset = nonLeafOffset;
	startOffset = (leafPageCount * 512)+64;
	while (nonLeafCount != 1)
	{
		tempOffset = endOffset;
		endOffset = nonLeafPageInsertion(fp, startOffset, endOffset);
		startOffset = tempOffset;
	}
	fseek(fp, 0, SEEK_SET);
	fread(&ts, sizeof(struct tableSector), 1, fp);
	ts.tableid[0] = 0;
	fseek(fp, -512, SEEK_END);
	ts.rootpageid[0] = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fwrite(&ts, sizeof(struct tableSector), 1, fp);
	fclose(fp);
}


void leafPageInsertion(FILE *fp)
{

	int brek = 0, count = 0;
	FILE *fp1;
	fp1 = fopen("Btree3.bin", "wb");
	memset(&ts, 0, sizeof(struct tableSector));
	fwrite(&ts, sizeof(struct tableSector), 1, fp1);
	int x = ftell(fp1);
	while (1)
	{
		count = 0;
		memset(&LP, 0, sizeof(struct leafPage));
		while (count<10)
		{
			LP.pagetype = 0;
			if (fscanf(fp, "%d,%[^,],%[^\n]s", &st.rollno, &st.name, &st.college) == EOF)
			{
				brek = 1;
				break;
			}
			LP.stud[count++] = st;
			memset(&st, 0, sizeof(struct student));
		}
		if (count != 0)
		{
			leafPageCount++;
			fwrite(&LP, sizeof(struct leafPage), 1, fp1);
		}
		if (brek == 1)
		{
			break;
		}
	}
	fclose(fp1);
}

void rangeScan(int min, int max)
{
	int tempOffset, found = 0, count = 0;
	FILE *fp;
	fp = fopen("Btree3.bin", "rb");
	fseek(fp, -512, SEEK_END);
	tempOffset = ftell(fp);
	fread(&NLP, sizeof(struct nonLeafPage), 1, fp);
	while (1)
	{
		if (NLP.pagetype == 0)
		{
			break;
		}
		else if (min < NLP.key[0] || NLP.offset[1] == 0)
		{
			tempOffset = NLP.offset[0];
		}
		else if (min >= NLP.key[62] && NLP.key[62] != 0)
		{
			tempOffset = NLP.offset[63];
		}
		else
		{
			count = 0;
			while (count < 62)
			{
				if (min >= NLP.key[count] && min < NLP.key[count + 1] || NLP.key[count + 1] == 0 && min >= NLP.key[count])
				{
					tempOffset = NLP.offset[count + 1];
					break;
				}
				count++;
			}
		}
		fseek(fp, tempOffset, SEEK_SET);
		fread(&NLP, sizeof(struct nonLeafPage), 1, fp);
	}
	int brek = 0, records = 0;
	fseek(fp, tempOffset - 512, SEEK_SET);
	while (ftell(fp) < leafPageCount * 512)
	{
		fseek(fp, tempOffset, SEEK_SET);
		fread(&LP, sizeof(struct leafPage), 1, fp);
		for (int i = 0; i < 10; i++)
		{
			if (LP.stud[i].rollno >= min&&LP.stud[i].rollno <= max)
			{
				if (found == 0)
				{
					printf("rollnumber\tstudentname\tcollege\n");
				}
				found = 1;
				printf("%d\t%s\t%s\n", LP.stud[i].rollno, LP.stud[i].name, LP.stud[i].college);
				records++;
			}
			if (LP.stud[i].rollno > max)
			{
				brek = 1;
				break;
			}
		}
		if (brek == 1)
			break;
		tempOffset = ftell(fp);
	}
	if (found == 0)
	{
		printf("there are no students in the given range\n");
	}
	printf("%d\n", records);
	fclose(fp);
}

int main()
{
	int min, max;
	FILE *fp;
	fp = fopen("Btree3.csv", "r");
	if (fp == NULL)
	{
		printf("unable to open file\n");
		return -1;
	}
	leafPageInsertion(fp);
	fclose(fp);
	nonLeafPageInsertion1();
	printf("enter two rollnumbers to range scan the student names\n");
	scanf("%d%d", &min, &max);
	rangeScan(min, max);
	return 0;
}
