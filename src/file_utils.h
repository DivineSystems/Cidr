/*
	Author: Alan Cai
	Github: flowac
	Email:  alan@ld50.bid
	License:Public Domain 2018

	Description:	File related utilities
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "define.h"

extern uint32_t Cvt_r(const char *in_path, const char *out_path);

uint32_t fsize(FILE *fp)
{
	uint32_t size;
	if (!fp) return 0;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return size;
}

char *is_zip(const char *name)
{
	char *tmp1 = strstr(name, ".Z");
	char *tmp2 = strstr(name, ".z");
	if (!tmp1 || (tmp2 && tmp1 < tmp2)) tmp1 = tmp2;
	return tmp1 == name + strlen(name) - 1 ? tmp1 : 0;
}

uint32_t crc32_table(uint32_t x)
{
	for (uint32_t i = 0; i < 8; i++) x = (x & 1 ? 0 : (uint32_t)0xEDB88320L) ^ x >> 1;
	return x ^ (uint32_t)0xFF000000L;
}

uint32_t crc32(const char *file)
{
	static uint32_t table[BUF_256];
	FILE *fp = fopen(file, "rb");
	uint32_t crc = 0, i, size = fsize(fp);
	uint8_t *data = malloc(size);

	if (!fp || !data) goto RETURN;
	memset(data, 0, size);
	size = fread(data, 1, size, fp);
	if (!*table) for (i = 0; i < BUF_256; i++) table[i] = crc32_table(i);
	for (i = 0; i < size; i++) crc = table[(uint8_t)(crc ^ data[i])] ^ (crc >> 8);
RETURN:	if (fp)   fclose(fp);
	if (data) free(data);
	return crc;
}

uint32_t diff_txt(const char *left, const char *right, FILE *out)
{
	char  left2[BUF_256],  *left3 = 0,  left4[BUF_256] = {0};//Unzipped filename, indicator, buffer
	char right2[BUF_256], *right3 = 0, right4[BUF_256] = {0};
	FILE *left5 = 0, *right5 = 0;
	uint32_t line = 1, diff = 0;
	strcpy(left2, left);
	strcpy(right2, right);

	if (left3 = is_zip(left2)) {
		*left3 = 0;
		if (!Cvt_r(left, left2))    goto RETURN;
	}
	if (right3 = is_zip(right2)) {
		*right3 = 0;
		if (!Cvt_r(right, right2))  goto RETURN;
	}
	if (!(left5  = fopen(left2,  "r"))) goto RETURN;
	if (!(right5 = fopen(right2, "r"))) goto RETURN;
	for (; fgets(left4, BUF_256, left5) - fgets(right4, BUF_256, right5)
	     ; *left4 = 0, *right4 = 0, line++) {
		if (strcmp(left4, right4) == 0) continue;
		if (*left4)  fprintf(out, C_RED "--%05d %s" C_STD, line, left4);
		if (*right4) fprintf(out, C_GRN "++%05d %s" C_STD, line, right4);
		diff++;
	}
RETURN:	if (left3)  remove(left2);
	if (right3) remove(right2);
	if (left5)  fclose(left5);
	if (right5) fclose(right5);
	return diff;
}

uint32_t diff_bin(const char *left, const char *right) {return crc32(left) ^ crc32(right);}

