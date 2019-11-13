#ifndef SETTING_H
#define SETTING_H
#define INT long
#define REAL float
#include <cstring>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <map>
#include "Triple.h"

std::string inPath = "../data/FB15K/";
std::string outPath = "../data/FB15K/";

extern "C"
void setInPath(char *path) {
	INT len = strlen(path);
	inPath = "";
	for (INT i = 0; i < len; i++)
		inPath = inPath + path[i];
	printf("Input Files Path : %s\n", inPath.c_str());
}

extern "C"
void setOutPath(char *path) {
	INT len = strlen(path);
	outPath = "";
	for (INT i = 0; i < len; i++)
		outPath = outPath + path[i];
	printf("Output Files Path : %s\n", outPath.c_str());
}

/*
============================================================
*/

INT workThreads = 1;

extern "C"
void setWorkThreads(INT threads) {
	workThreads = threads;
}

extern "C"
INT getWorkThreads() {
	return workThreads;
}

/*
============================================================
*/

INT relationTotal = 0;
INT entityTotal = 0;
INT tripleTotal = 0;
INT testTotal = 0;
INT trainTotal = 0;
INT validTotal = 0;
INT diffTotal = 0;
std::unordered_map<long int, Triple> headMap;
std::unordered_map<long int, Triple> tailMap;

extern "C"
INT getEntityTotal() {
	return entityTotal;
}

extern "C"
INT getRelationTotal() {
	return relationTotal;
}

extern "C"
INT getTripleTotal() {
	return tripleTotal;
}

extern "C"
INT getTrainTotal() {
	return trainTotal;
}

extern "C"
INT getTestTotal() {
	return testTotal;
}

extern "C"
INT getValidTotal() {
	return validTotal;
}

extern "C"
INT getDiffTotal() {
	return diffTotal;
}

extern "C"
std::unordered_map<long int, Triple> getHeadMap() {
	return headMap;
}

extern "C"
std::unordered_map<long int, Triple> getTailMap() {
	return tailMap;
}
/*
============================================================
*/

INT bernFlag = 0;
INT crossSamplingFlag = 0;
INT trueNegativeSamplesFlag = 0;

extern "C"
void setBern(INT con) {
	bernFlag = con;
}

extern "C"
void setHeadTailCrossSampling(INT con){
	crossSamplingFlag = con;
}

extern "C"
void setTrueNegativeSamplesFlag(INT con) {
	trueNegativeSamplesFlag = con;
}

#endif
