#include <Python.h>
#include "Setting.h"
#include "Random.h"
#include "Reader.h"
#include "Corrupt.h"
#include "Test.h"
#include "Valid.h"
#include <cstdlib>
#include <pthread.h>

extern "C"
void setInPath(char *path);

extern "C"
void setOutPath(char *path);

extern "C"
void setWorkThreads(INT threads);

extern "C"
void setBern(INT con);

extern "C"
void setHeadTailCrossSampling(INT con);

extern "C"
void setTrueNegativeSamplesFlag(INT con);

extern "C"
bool judgeHeadBatch();

extern "C"
INT getWorkThreads();

extern "C"
INT getEntityTotal();

extern "C"
INT getRelationTotal();

extern "C"
INT getTripleTotal();

extern "C"
INT getTrainTotal();

extern "C"
INT getTestTotal();

extern "C"
INT getValidTotal();

extern "C"
INT getDiffTotal();

extern "C"
void randReset();

extern "C"
void importTrainFiles();

extern "C"
void importNegativeSamples();

struct Parameter {
	INT id;
	INT *batch_h;
	INT *batch_t;
	INT *batch_r;
	REAL *batch_y;
	INT batchSize;
	INT negRate;
	INT negRelRate;
	INT headBatchFlag;
};

bool headOrTailInNegativeSample(INT head, INT tail) {
	if(headMap.find(head) == headMap.end()) {
		if(tailMap.find(head) == tailMap.end()) {
			if(headMap.find(tail) == headMap.end()) {
				if(tailMap.find(tail) == tailMap.end()) {
					return false;
				}
			}
		}
	}
	return true;
}

INT* performLinkPredictionForCorruption(INT tail, INT rel) {
	// Initialize the Python interpreter.
	Py_Initialize();
	// Create some Python objects that will later be assigned values.
	PyObject *pName, *pModule, *pDict, *pFunc, *pArgs, *pValue;
	// Convert the file name to a Python string.
	pName = PyString_FromString("Config");
	// Import the file as a Python module.
	pModule = PyImport_Import(pName);
	// Create a dictionary for the contents of the module.
	pDict = PyModule_GetDict(pModule);
	// Get the add method from the dictionary.
	pFunc = PyDict_GetItemString(pDict, "predict_head_entity"); //braucht als übergabeparameter t, r und k
	// Create a Python tuple to hold the arguments to the method.
	pArgs = PyTuple_New(3);
	pValue = PyInt_FromLong(tail);// -> convert the tail key
	PyTuple_SetItem(pArgs, 0, pValue);
	pValue = PyInt_FromLong(rel);// -> convert the relation key
	PyTuple_SetItem(pArgs, 1, pValue);
	pValue = PyInt_FromLong(1);// -> we only want the first element that is predicted
	PyTuple_SetItem(pArgs, 2, pValue);
	// Call the function with the arguments.
	PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
	// Print a message if calling the method failed.
	if(pResult == NULL)
		printf("Calling the add method failed.\n");
		// Convert the result to a long from a Python object.
	long result = PyInt_AsLong(pResult);
	INT* predictedHead = (long int*) (INT) result;
	// Destroy the Python interpreter.
	Py_Finalize();
	return predictedHead;
}

void* getBatchWithNewCorruption(void* con) {
	Parameter *para = (Parameter *)(con);
	INT id = para -> id;
	INT *batch_h = para -> batch_h;
	INT *batch_t = para -> batch_t;
	INT *batch_r = para -> batch_r;
	REAL *batch_y = para -> batch_y;
	INT batchSize = para -> batchSize;
	INT negRate = para -> negRate;
	INT negRelRate = para -> negRelRate;
	INT headBatchFlag = para -> headBatchFlag;
	INT lef, rig;
	if (batchSize % workThreads == 0) {
		lef = id * (batchSize / workThreads);
		rig = (id + 1) * (batchSize / workThreads);
	} else {
		lef = id * (batchSize / workThreads + 1);
		rig = (id + 1) * (batchSize / workThreads + 1);
		if (rig > batchSize) rig = batchSize;
	}
	REAL prob = 500;
	INT last = batchSize;
	for (INT batch = lef; batch < rig; batch++) {
		//define true example
		//perform link prediction
		//if predicted head is different from true head -> set negative sample
		//else -> get new triple from batch and do the same procedure
		bool correctHeadPredicted = true;
		INT* predictedHead;
		while(correctHeadPredicted) {
			INT i = rand_max(id, trainTotal);
			batch_h[batch] = trainList[i].h;
			batch_t[batch] = trainList[i].t;
			batch_r[batch] = trainList[i].r;
			batch_y[batch] = 1;
			last = batchSize;

			predictedHead = performLinkPredictionForCorruption(batch_t[batch], batch_r[batch]);
			if(*predictedHead != batch_h[batch]) correctHeadPredicted = false;
		}

		batch_h[batch + last] = *predictedHead;
		batch_t[batch + last] = batch_t[batch];
		batch_r[batch + last] = batch_r[batch];
		batch_y[batch + last] = -1;

		last += batchSize;

		pthread_exit(NULL);
	}
}

void* getNegativeBatch(void* con) {
	Parameter *para = (Parameter *)(con);
	INT id = para -> id;
	INT *batch_h = para -> batch_h;
	INT *batch_t = para -> batch_t;
	INT *batch_r = para -> batch_r;
	REAL *batch_y = para -> batch_y;
	INT batchSize = para -> batchSize;
	INT negRate = para -> negRate;
	INT negRelRate = para -> negRelRate;
	INT headBatchFlag = para -> headBatchFlag;
	INT lef, rig;
	if (batchSize % workThreads == 0) {
		lef = id * (batchSize / workThreads);
		rig = (id + 1) * (batchSize / workThreads);
	} else {
		lef = id * (batchSize / workThreads + 1);
		rig = (id + 1) * (batchSize / workThreads + 1);
		if (rig > batchSize) rig = batchSize;
	}
	REAL prob = 500;
	INT last = batchSize;
	for (INT batch = lef; batch < rig; batch++) {
		bool headTailInNegSamples = false;
		while(!headTailInNegSamples) {
			INT i = rand_max(id, trainTotal);
			batch_h[batch] = trainList[i].h;
			batch_t[batch] = trainList[i].t;
			batch_r[batch] = trainList[i].r;
			batch_y[batch] = 1;
			last = batchSize;
			if(headOrTailInNegativeSample(batch_h[batch], batch_t[batch])) {
				headTailInNegSamples = true;
			}
		}

		for (INT times = 0; times < negRate; times ++) {
			if(trueNegativeSamplesFlag) {
				//-------------------------------------------
				//first try: use a random true negativ sample
				//-------------------------------------------
				/*INT j = rand_max(id, diffTotal);
				batch_h[batch + last] = trueNegList[j].h;
				batch_t[batch + last] = trueNegList[j].t;
				batch_r[batch + last] = trueNegList[j].r;
				batch_y[batch + last] = -1;*/

				//---------------------------------------------------------
				//second try: use a sample that has a matching head or tail
				//---------------------------------------------------------
				Triple negTriple;

				if(headMap.find(batch_h[batch]) == headMap.end()) {
					if(tailMap.find(batch_h[batch]) == tailMap.end()) {
						if(headMap.find(batch_t[batch]) == headMap.end()) {
							if(tailMap.find(batch_t[batch]) == tailMap.end()) {
								INT j = rand_max(id, diffTotal);
								negTriple = trueNegList[j];
							} else {
								negTriple = tailMap.at(batch_t[batch]);
							}
						} else {
							negTriple = headMap.at(batch_t[batch]);
						}
					} else {
						negTriple = tailMap.at(batch_h[batch]);
					}
				} else {
					negTriple = headMap.at(batch_h[batch]);

				}

				batch_h[batch + last] = negTriple.h;
				batch_t[batch + last] = negTriple.t;
				batch_r[batch + last] = negTriple.r;
				batch_y[batch + last] = -1;

				last += batchSize;
			}
		}
		pthread_exit(NULL);
	}
}

void* getBatch(void* con) {
	Parameter *para = (Parameter *)(con);
	INT id = para -> id;
	INT *batch_h = para -> batch_h;
	INT *batch_t = para -> batch_t;
	INT *batch_r = para -> batch_r;
	REAL *batch_y = para -> batch_y;
	INT batchSize = para -> batchSize;
	INT negRate = para -> negRate;
	INT negRelRate = para -> negRelRate;
	INT headBatchFlag = para -> headBatchFlag;
	INT lef, rig;
	if (batchSize % workThreads == 0) {
		lef = id * (batchSize / workThreads);
		rig = (id + 1) * (batchSize / workThreads);
	} else {
		lef = id * (batchSize / workThreads + 1);
		rig = (id + 1) * (batchSize / workThreads + 1);
		if (rig > batchSize) rig = batchSize;
	}
	REAL prob = 500;
	for (INT batch = lef; batch < rig; batch++) {
		INT i = rand_max(id, trainTotal);
		batch_h[batch] = trainList[i].h;
		batch_t[batch] = trainList[i].t;
		batch_r[batch] = trainList[i].r;
		batch_y[batch] = 1;
		INT last = batchSize;
		//headMap = getHeadMap();
		//tailMap = getTailMap();
		for (INT times = 0; times < negRate; times ++) {
			if(trueNegativeSamplesFlag) {
				//-------------------------------------------
				//first try: use a random true negativ sample
				//-------------------------------------------
				/*INT j = rand_max(id, diffTotal);
				batch_h[batch + last] = trueNegList[j].h;
				batch_t[batch + last] = trueNegList[j].t;
				batch_r[batch + last] = trueNegList[j].r;
				batch_y[batch + last] = -1;*/

				//---------------------------------------------------------
				//second try: use a sample that has a matching head or tail
				//---------------------------------------------------------
				Triple negTriple;

				if(headMap.find(batch_h[batch]) == headMap.end()) {
					if(tailMap.find(batch_h[batch]) == tailMap.end()) {
						if(headMap.find(batch_t[batch]) == headMap.end()) {
							if(tailMap.find(batch_t[batch]) == tailMap.end()) {
								INT j = rand_max(id, diffTotal);
								negTriple = trueNegList[j];
							} else {
								negTriple = tailMap.at(batch_t[batch]);
							}
						} else {
							negTriple = headMap.at(batch_t[batch]);
						}
					} else {
						negTriple = tailMap.at(batch_h[batch]);
					}
				} else {
					negTriple = headMap.at(batch_h[batch]);

				}

				batch_h[batch + last] = negTriple.h;
				batch_t[batch + last] = negTriple.t;
				batch_r[batch + last] = negTriple.r;
				batch_y[batch + last] = -1;

				last += batchSize;
			} else if (!crossSamplingFlag){
				if (bernFlag)
					prob = 1000 * right_mean[trainList[i].r] / (right_mean[trainList[i].r] + left_mean[trainList[i].r]);
				if (randd(id) % 1000 < prob) {
					batch_h[batch + last] = trainList[i].h;
					batch_t[batch + last] = corrupt_head(id, trainList[i].h, trainList[i].r);
					batch_r[batch + last] = trainList[i].r;
				} else {
					batch_h[batch + last] = corrupt_tail(id, trainList[i].t, trainList[i].r);;
					batch_t[batch + last] = trainList[i].t;
					batch_r[batch + last] = trainList[i].r;
				}
				batch_y[batch + last] = -1;
				last += batchSize;
			} else  {
				if(headBatchFlag){
					batch_h[batch + last] = corrupt_tail(id, trainList[i].t, trainList[i].r);
					batch_t[batch + last] = trainList[i].t;
					batch_r[batch + last] = trainList[i].r;
				} else {
					batch_h[batch + last] = trainList[i].h;
					batch_t[batch + last] = corrupt_head(id, trainList[i].h, trainList[i].r);
					batch_r[batch + last] = trainList[i].r;
				}
				batch_y[batch + last] = -1;
				last += batchSize;
			}
		}
		for (INT times = 0; times < negRelRate; times++) {
			batch_h[batch + last] = trainList[i].h;
			batch_t[batch + last] = trainList[i].t;
			batch_r[batch + last] = corrupt_rel(id, trainList[i].h, trainList[i].t);
			batch_y[batch + last] = -1;
			last += batchSize;
		}
	}
	pthread_exit(NULL);
}

extern "C"
void sampling(INT *batch_h, INT *batch_t, INT *batch_r, REAL *batch_y, INT batchSize, INT negRate = 1, INT negRelRate = 0, INT headBatchFlag = 0, INT epochNumber = 0) {
	pthread_t *pt = (pthread_t *)malloc(workThreads * sizeof(pthread_t));
	Parameter *para = (Parameter *)malloc(workThreads * sizeof(Parameter));
	for (INT threads = 0; threads < workThreads; threads++) {
		para[threads].id = threads;
		para[threads].batch_h = batch_h;
		para[threads].batch_t = batch_t;
		para[threads].batch_r = batch_r;
		para[threads].batch_y = batch_y;
		para[threads].batchSize = batchSize;
		para[threads].negRate = negRate;
		para[threads].negRelRate = negRelRate;
		para[threads].headBatchFlag = headBatchFlag;
		if(trueNegativeSamplesFlag) {
			pthread_create(&pt[threads], NULL, getNegativeBatch, (void*)(para+threads));
			/*if(epochNumber > 1) {
				pthread_create(&pt[threads], NULL, getBatchWithNewCorruption, (void*)(para+threads));
			} else {
				pthread_create(&pt[threads], NULL, getNegativeBatch, (void*)(para+threads));
			}*/
		} else {
			pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
		}
	}
	for (INT threads = 0; threads < workThreads; threads++)
		pthread_join(pt[threads], NULL);
	free(pt);
	free(para);
}

int main() {
	importTrainFiles();
	return 0;
}
