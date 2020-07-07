//Given a CSV files of student and teacher records, construct a B+ tree in a binary file where a table sector should be written in the beginning
//of the file which will hold the root pages and table ids of different records. Here two( student and teacher records ) types of records are 
//given in the input file. The root ( non leaf page ) offsets need to be stored in the table sector.

#include<stdio.h>
#include<string.h>

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

struct teachingstaff
{
	int staff_id;
	char name[24];
	char dept[12];
	char college[24];
};

struct leafPage
{
	int pagetype;
	struct student stud[10];
	char unused[28];
};

struct leafPageStaff
{
	int pagetype;
	struct teachingstaff staff[7];
	char unused[60];
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
struct leafPageStaff LPS;
struct tableSector ts;
struct teachingstaff TS;

int leafPageCount1 = 0;
int leafPageCount2 = 0;
int nonLeafCount = 0;

int getKey(FILE *fp, int offset, int spec)
{
	int key;
	fseek(fp, offset, SEEK_SET);
	fread(&nlp2, sizeof(struct nonLeafPage), 1, fp);
	if (nlp2.pagetype == 0)
	{
		if (spec == 0)
		{
			fseek(fp, offset, SEEK_SET);
			fread(&LP, sizeof(struct leafPage), 1, fp);
			return LP.stud[0].rollno;
		}
		else
		{
			fseek(fp, offset, SEEK_SET);
			fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
			return LPS.staff[0].staff_id;
		}
	}
	else
	{
		key = getKey(fp, nlp2.offset[0], spec);
	}
	return key;
}


int nonLeafPageInsertion(FILE *fp, int startOffset, int endOffset, int spec)
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
					NLP.key[count] = getKey(fp, NLP.offset[count], spec);
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
				NLP.key[count - 1] = getKey(fp, NLP.offset[count], spec);
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


void staffNonLeafPageInsertion1()
{
	int tempOffset, startOffset, endOffset, nonLeafOffset, count, brek = 0;
	FILE *fp;
	fp = fopen("Btree4.bin", "rb+");
	fseek(fp, 0, SEEK_END);
	nonLeafOffset = endOffset = ftell(fp);
	fseek(fp, ts.rootpageid[0] + 512, SEEK_SET);
	nonLeafCount = 0;
	while (ftell(fp) < endOffset)
	{
		nonLeafCount++;
		memset(&NLP, 0, sizeof(struct nonLeafPage));
		NLP.pagetype = 1;
		count = 0;
		while (count < 64)
		{
			memset(&LPS, 0, sizeof(struct leafPageStaff));
			if (count == 0)
			{
				NLP.offset[count] = ftell(fp);
				fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
				if (ftell(fp) == endOffset)
				{
					NLP.key[count] = LPS.staff[6].staff_id + 1;
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
				fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
				NLP.key[count - 1] = LPS.staff[0].staff_id;
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
			break;
		tempOffset = ftell(fp);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	startOffset = endOffset;
	endOffset = nonLeafOffset;
	while (nonLeafCount != 1)
	{
		tempOffset = endOffset;
		endOffset = nonLeafPageInsertion(fp, startOffset, endOffset, 1);
		startOffset = tempOffset;
	}
	fseek(fp, 0, SEEK_SET);
	fread(&ts, sizeof(struct tableSector), 1, fp);
	ts.tableid[1] = 1;
	fseek(fp, -512, SEEK_END);
	ts.rootpageid[1] = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fwrite(&ts, sizeof(struct tableSector), 1, fp);
	fclose(fp);
}

void nonLeafPageInsertion1()
{
	int tempOffset, startOffset, endOffset, nonLeafOffset, count, brek = 0;
	FILE *fp;
	fp = fopen("Btree4.bin", "rb+");
	fseek(fp, 0, SEEK_END);
	nonLeafOffset = endOffset = ftell(fp);
	fseek(fp, 64, SEEK_SET);
	while (ftell(fp) < endOffset)
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
				if (ftell(fp) == endOffset)
				{
					NLP.key[count] = LP.stud[9].rollno + 1;
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
			break;
		tempOffset = ftell(fp);
		fseek(fp, nonLeafOffset, SEEK_SET);
		fwrite(&NLP, sizeof(struct nonLeafPage), 1, fp);
		nonLeafOffset = ftell(fp);
		fseek(fp, tempOffset, SEEK_SET);
	}
	startOffset = endOffset;
	endOffset = nonLeafOffset;
	while (nonLeafCount != 1)
	{
		tempOffset = endOffset;
		endOffset = nonLeafPageInsertion(fp, startOffset, endOffset, 0);
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


void StaffleafPageInsertion(FILE *fp)
{

	int brek = 0, count = 0;
	FILE *fp1;
	fp1 = fopen("Btree4.bin", "rb+");
	fseek(fp1, 0, SEEK_END);
	while (1)
	{
		count = 0;
		memset(&LPS, 0, sizeof(struct leafPageStaff));
		while (count<7)
		{
			LPS.pagetype = 0;
			if (fscanf(fp, "%d,%[^,],%[^,],%[^\n]s", &TS.staff_id, &TS.name, &TS.college, &TS.dept) == EOF)
			{
				brek = 1;
				break;
			}
			LPS.staff[count++] = TS;
			memset(&TS, 0, sizeof(struct teachingstaff));
		}
		if (count != 0)
		{
			leafPageCount2++;
			fwrite(&LPS, sizeof(struct leafPageStaff), 1, fp1);
		}
		if (brek == 1)
		{
			break;
		}
	}
	fclose(fp1);
}

void leafPageInsertion(FILE *fp)
{

	int brek = 0, count = 0;
	FILE *fp1;
	fp1 = fopen("Btree4.bin", "wb");
	memset(&ts, 0, sizeof(struct tableSector));
	fwrite(&ts, sizeof(struct tableSector), 1, fp1);
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
			leafPageCount1++;
			fwrite(&LP, sizeof(struct leafPage), 1, fp1);
		}
		if (brek == 1)
		{
			break;
		}
	}
	fclose(fp1);
}

void rangeScan(int min, int max, int spec)
{
	int tempOffset, endOffset, found = 0, count = 0;
	FILE *fp;
	fp = fopen("Btree4.bin", "rb");
	if (spec == 0)
	{
		fseek(fp, ts.rootpageid[0], SEEK_SET);
		endOffset = leafPageCount1 * 512 + 64;
	}
	else
	{
		fseek(fp, ts.rootpageid[1], SEEK_SET);
		endOffset = (leafPageCount2 * 512) + ts.rootpageid[0] + 512;
	}
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
	while (ftell(fp) < endOffset)
	{
		fseek(fp, tempOffset, SEEK_SET);
		if (spec == 0)
		{
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
		}
		else
		{
			fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
			for (int i = 0; i < 7; i++)
			{
				if (LPS.staff[i].staff_id >= min&&LPS.staff[i].staff_id <= max&&LPS.staff[i].staff_id != 0)
				{
					if (found == 0)
					{
						printf("staff_id\tstaffname\tcollege\tdepartment\n");
					}
					found = 1;
					printf("%d\t%s\t%s\t%s\n", LPS.staff[i].staff_id, LPS.staff[i].name, LPS.staff[i].college, LPS.staff[i].dept);
					records++;
				}
				if (LPS.staff[i].staff_id > max)
				{
					brek = 1;
					break;
				}
			}
		}
		if (brek == 1)
			break;
		tempOffset = ftell(fp);
	}
	if (found == 0)
	{
		printf("there are no persons in the given range\n");
	}
	printf("%d\n", records);
	fclose(fp);
}

void groupByCollege(int id)
{
	FILE *fp;
	fp = fopen("Btree4.bin", "rb");
	int tempOffset, count = 0,found=0,endOffset,records=0;
	char college[20];
	fseek(fp, ts.rootpageid[1], SEEK_SET);
	tempOffset = ftell(fp);
	fread(&NLP, sizeof(struct nonLeafPage), 1, fp);
	while (1)
	{
		if (NLP.pagetype == 0)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
			for (int i = 0; i < 7; i++)
			{
				if (LPS.staff[i].staff_id == id)
				{
					strcpy(college, LPS.staff[i].college);
					found = 1;
					break;
				}
			}
			if (found == 0)
			{
				printf("staff_id doesnot exist in our database\n");
				return;
			}
			break;
		}
		else if (id < NLP.key[0] || NLP.offset[1] == 0)
		{
			tempOffset = NLP.offset[0];
		}
		else if (id >= NLP.key[62] && NLP.key[62] != 0)
		{
			tempOffset = NLP.offset[63];
		}
		else
		{
			count = 0;
			while (count < 62)
			{
				if (id >= NLP.key[count] && id < NLP.key[count + 1] || NLP.key[count + 1] == 0 && id >= NLP.key[count])
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
	if (found == 1)
	{
		found = 0;
		fseek(fp, 64, SEEK_SET);
		endOffset = leafPageCount1 * 512 + 64;
		tempOffset = ftell(fp);
		while (ftell(fp) < endOffset)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&LP, sizeof(struct leafPage), 1, fp);
			for (int i = 0; i < 10; i++)
			{
				if (!strcmp(LP.stud[i].college,college))
				{
					if (found == 0)
					{
						printf("rollnumber\tstudentname\tcollege\n");
					}
					found = 1;
					printf("%d\t%s\t%s\n", LP.stud[i].rollno, LP.stud[i].name, LP.stud[i].college);
					records++;
				}
			}
			tempOffset = ftell(fp);
		}
	}
	printf("%d\n", records);
}

void departments(int id)
{
	FILE *fp;
	fp = fopen("Btree4.bin", "rb");
	int tempOffset, count = 0, found = 0, endOffset, records = 0,dcount=0,sfound=0;
	char college[20],department[20][10];
	fseek(fp, ts.rootpageid[0], SEEK_SET);
	tempOffset = ftell(fp);
	fread(&NLP, sizeof(struct nonLeafPage), 1, fp);
	while (1)
	{
		if (NLP.pagetype == 0)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&LP, sizeof(struct leafPage), 1, fp);
			for (int i = 0; i < 10; i++)
			{
				if (LP.stud[i].rollno == id)
				{
					strcpy(college, LP.stud[i].college);
					printf("college is: %s\n", college);
					found = 1;
					break;
				}
			}
			if (found == 0)
			{
				printf("student roll number doesnot exist in our database\n");
				return;
			}
			break;
		}
		else if (id < NLP.key[0] || NLP.offset[1] == 0)
		{
			tempOffset = NLP.offset[0];
		}
		else if (id >= NLP.key[62] && NLP.key[62] != 0)
		{
			tempOffset = NLP.offset[63];
		}
		else
		{
			count = 0;
			while (count < 62)
			{
				if (id >= NLP.key[count] && id < NLP.key[count + 1] || NLP.key[count + 1] == 0 && id >= NLP.key[count])
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
	if (found == 1)
	{
		found = 0;
		fseek(fp, ts.rootpageid[0]+512, SEEK_SET);
		endOffset = leafPageCount2 * 512 + ftell(fp);
		tempOffset = ftell(fp);
		while (ftell(fp) < endOffset)
		{
			fseek(fp, tempOffset, SEEK_SET);
			fread(&LPS, sizeof(struct leafPageStaff), 1, fp);
			for (int i = 0; i < 7; i++)
			{
				sfound = 0;
				if (!strcmp(LPS.staff[i].college, college))
				{
					if (found == 0)
					{
						printf("departments are :\n");
					}
					found = 1;
					if (dcount == 0)
					{
						strcpy(department[dcount++], LPS.staff[i].dept);
						records++;
					}
					else
					{
						for (int j = 0; j < dcount; j++)
						{
							if (!strcmp(department[j], LPS.staff[i].dept))
							{
								sfound = 1;
								continue;
							}
						}
						if (sfound == 0)
						{
							strcpy(department[dcount++], LPS.staff[i].dept);
							records++;
						}
					}
				}
			}
			tempOffset = ftell(fp);
		}
	}
	if (found == 0)
	{
		printf("ther are no departments in that college\n");
	}
	else
	{
		for (int i = 0; i < dcount; i++)
		{
			printf("%s\n", department[i]);
		}
	}
	printf("%d\n", records);
}

void menu()
{
	printf("this program is about storing two tables in a single binary file\n");
	printf("1-students details range scan\n");
	printf("2-staff details range scan\n");
	printf("3-student details group by college\n");
	printf("4-departments in a particular college by given student roll number\n");
	printf("0-exit\n");
}

int main()
{
	int min, max, ch;
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
	fp = fopen("Btree4.csv", "r");
	if (fp == NULL)
	{
		printf("unable to open file\n");
		return -1;
	}
	StaffleafPageInsertion(fp, 1);
	fclose(fp);
	staffNonLeafPageInsertion1(1);
	do
	{
		menu();
		printf("kindly enter your choice\n");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
			printf("enter two student roll numbers to range scan the student details\n");
			scanf("%d%d", &min,&max);
			rangeScan(min,max,0);
			break;
		case 2:
			printf("enter two staff_ids to range scan the staff details\n");
			scanf("%d%d", &min, &max);
			rangeScan(min, max, 1);
			break;
		case 3:
			printf("enter staff_id to get the student details of that particular college\n");
			scanf("%d", &min);
			groupByCollege(min);
			break;
		case 4:
			printf("enter student roll number to get avalable departments in his college\n");
			scanf("%d", &min);
			departments(min);
			break;
		}
		printf("\n\n");
	} while (ch != 0);
	return 0;
}
