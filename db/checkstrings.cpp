// To use, remove the binary, then "make checkstrings"
// Could say "all is well" when not, if some operators are deleted and the
// same number added

#include <stdio.h>
#include <string.h>
#include "operstrings.h"
#include "../include/operator.h"

int main() {
	if (sizeof(operStrings)/sizeof(char*) == opNumOf) {
		printf("All is correct\n");
		return 0;
	}
	if ((strcmp(operStrings[opFPlusd],	"opFPlusd") != 0)) {
		printf("Error before opFPlusd\n"); return 1; }
	if ((strcmp(operStrings[opSQRTq],	"opSQRTq") != 0)) {
		printf("Error before opSQRTq\n"); return 1;}
	if ((strcmp(operStrings[opGtrEqUns],	"opGtrEqUns") != 0)) {
		printf("Error before opGtrEqUns\n"); return 1;}
	if ((strcmp(operStrings[opTargetInst],	"opTargetInst") != 0)) {
		printf("Error before opTargetInst\n"); return 1;}
	if ((strcmp(operStrings[opList],	"opList") != 0)) {
		printf("Error before opList\n"); return 1;}
	if ((strcmp(operStrings[opMachFtr],	"opMachFtr") != 0)) {
		printf("Error before opMachFtr\n"); return 1;}
	if ((strcmp(operStrings[opFpop],	"opFpop") != 0)) {
		printf("Error before opFpop\n"); return 1;}
	if ((strcmp(operStrings[opExecute],	"opExecute") != 0)) {
		printf("Error before opExecute\n"); return 1;}
	if ((strcmp(operStrings[opWildStrConst],	"opWildStrConst") != 0)) {
		printf("Error before opWildStrConst\n"); return 1;}
	if ((strcmp(operStrings[opAnull],	"opAnull") != 0)) {
		printf("Error before opAnull\n"); return 1;}
	printf("Error near the end\n");
	return 1;
}
