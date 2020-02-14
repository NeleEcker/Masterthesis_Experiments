#include <Python.h>
#include "Setting.h"
#include "Random.h"
#include "Reader.h"
#include "Corrupt.h"
#include "Test.h"
#include "Valid.h"
#include <cstdlib>
#include <pthread.h>
#include <iostream>

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
	/*if(headMap.find(head) == headMap.end()) {
		if(tailMap.find(head) == tailMap.end()) {
			if(headMap.find(tail) == headMap.end()) {
				if(tailMap.find(tail) == tailMap.end()) {
					return false;
				}
			}
		}
	}*/
	if(headMap.find(head) == headMap.end()) {
		if(tailMap.find(tail) == tailMap.end()) {
			return false;
		}
	}
	return true;
}

void* printArray(int arr[], int arraysize)
{
	char delim = ',';

	std::string s;
	for (unsigned i = 0; i < arraysize; i++) {
		if (i) {
			s.push_back(delim);
		}
		s.push_back(arr[i] + '0');
	}

	std::cout << s;

	return 0;
}

void* getTestBatch(void* con) {
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
	INT randomPicked = 0;
	INT totalPicked = 0;

	for (INT batch = lef; batch < rig; batch++) {
		INT i = rand_max(id, trainTotal);
		batch_h[batch] = trainList[i].h;
		batch_t[batch] = trainList[i].t;
		batch_r[batch] = trainList[i].r;
		batch_y[batch] = 1;
		INT last = batchSize;
		totalPicked = totalPicked + 1;
		//headMap = getHeadMap();
		//tailMap = getTailMap();
		for (INT times = 0; times < negRate; times ++) {
			INT j = 0;
			Triple negTriple;
			if(trueNegativeSamplesFlag) {
				if(headMap.find(batch_h[batch]) == headMap.end()) {
					if(tailMap.find(batch_h[batch]) == tailMap.end()) {
						if(headMap.find(batch_t[batch]) == headMap.end()) {
							if(tailMap.find(batch_t[batch]) == tailMap.end()) {
								j = rand_max(id, diffTotal);
								negTriple = trueNegList[j];
								randomPicked = randomPicked +1;
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
	}
	printf("totalPicked: %d", totalPicked);
	printf("randomPicked: %d", randomPicked);
	pthread_exit(NULL);
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
	//INT totalPicked = 0;
	//INT randomPicked = 0;
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
	//INT counter = 1;
	for (INT batch = lef; batch < rig; batch++) {
		//totalPicked = totalPicked + 1;
		INT counter = 1;
		bool headTailInNegSamples = false;
		while(!headTailInNegSamples) {
			INT i = rand_max(id, trainTotal);
			batch_h[batch] = trainList[i].h;
			batch_t[batch] = trainList[i].t;
			batch_r[batch] = trainList[i].r;
			batch_y[batch] = 1;
			last = batchSize;
			counter += 1;
			if(headOrTailInNegativeSample(batch_h[batch], batch_t[batch]) == true) {
				headTailInNegSamples = true;
			}
			if(counter == batchSize) { //make sure a random value can be picked as well
				headTailInNegSamples = true;
			}
		}

		for (INT times = 0; times < negRate; times ++) {
			if(trueNegativeSamplesFlag) {
				Triple negTriple;

				if(headMap.find(batch_h[batch]) == headMap.end()) {
					if(tailMap.find(batch_h[batch]) == tailMap.end()) {
						if(headMap.find(batch_t[batch]) == headMap.end()) {
							if(tailMap.find(batch_t[batch]) == tailMap.end()) {
								INT j = rand_max(id, diffTotal);
								negTriple = trueNegList[j];
								//randomPicked = randomPicked +1;
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
				//printf("This was also reached, %d", negTriple.h);
				batch_h[batch + last] = negTriple.h;
				batch_t[batch + last] = negTriple.t;
				batch_r[batch + last] = negTriple.r;
				batch_y[batch + last] = -1;
				last += batchSize;
			}
		}
	}
	//printf("totalPicked: %d\n", totalPicked);
	//printf("randomPicked: %d\n", randomPicked);
	pthread_exit(NULL);
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
			INT j = 0;
			Triple negTriple;
			if(trueNegativeSamplesFlag) {
				switch(negSampleVersion) {
					case 1:
						j = rand_max(id, diffTotal);
						batch_h[batch + last] = trueNegList[j].h;
						batch_t[batch + last] = trueNegList[j].t;
						batch_r[batch + last] = trueNegList[j].r;
						break;
					case 2:
						if(headMap.find(batch_h[batch]) == headMap.end()) {
							if(tailMap.find(batch_h[batch]) == tailMap.end()) {
								if(headMap.find(batch_t[batch]) == headMap.end()) {
									if(tailMap.find(batch_t[batch]) == tailMap.end()) {
										j = rand_max(id, diffTotal);
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
						break;
					case 4:
						if(headMap.find(batch_h[batch]) == headMap.end()) {
							if(tailMap.find(batch_t[batch]) == tailMap.end()) {
								j = rand_max(id, diffTotal);
								negTriple = trueNegList[j];
							} else {
								negTriple = tailMap.at(batch_t[batch]);
							}
						} else {
							negTriple = headMap.at(batch_h[batch]);
						}

						batch_h[batch + last] = negTriple.h;
						batch_t[batch + last] = negTriple.t;
						batch_r[batch + last] = negTriple.r;
						break;
					default:
						break;
				}
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
void callPython() {
	//Py_Initialize();
	PyRun_SimpleString("print(\"Hello world\")");
	//Py_Finalize();
}

extern "C"
void sampling(INT *batch_h, INT *batch_t, INT *batch_r, REAL *batch_y, INT batchSize, INT negRate = 1, INT negRelRate = 0, INT headBatchFlag = 0, INT epochNumber = 0) {
	pthread_t *pt = (pthread_t *)malloc(workThreads * sizeof(pthread_t));
	Parameter *para = (Parameter *)malloc(workThreads * sizeof(Parameter));
	//printf("EpochNumber: %d\n", epochNumber);
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

		switch(negSampleVersion){
			case 0: //corrupt negatives
				pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
				break;
			case 1: //random samples
				pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
				break;
			case 2: // if possible take matching element
				pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
				break;
			case 3: // look through all positives to find matching negatives
				pthread_create(&pt[threads], NULL, getNegativeBatch, (void*)(para+threads));
				break;
			case 4: // if possible take matching head for head and matching tail for tail
				pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
				break;
			case 5: //try to avoid overfitting
				pthread_create(&pt[threads], NULL, getBatch, (void*)(para+threads));
				break;
			case 7: //test how often fallback is used
				pthread_create(&pt[threads], NULL, getTestBatch, (void*)(para+threads));
				break;
			default:
				break;
		}
	}
	for (INT threads = 0; threads < workThreads; threads++)
		pthread_join(pt[threads], NULL);
	free(pt);
	free(para);
}

/*extern "C"
void samplingWithLinkPrediction(INT *batch_h, INT *batch_t, INT *batch_r, REAL *batch_y, INT batchSize, INT negRate = 1, INT negRelRate = 0, INT headBatchFlag = 0, INT epochNumber = 0, INT randomNumbers[], INT corruptedHeads[]) {
	pthread_t *pt = (pthread_t *)malloc(workThreads * sizeof(pthread_t));
	Parameter *para = (Parameter *)malloc(workThreads * sizeof(Parameter));
	//printf("EpochNumber: %d\n", epochNumber);
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

		switch(negSampleVersion){
			case 5: //link prediction (python in c++)
				pthread_create(&pt[threads], NULL, getBatchWithNewCorruption, (void*)(para+threads));
				break;
			default:
				break;
		}
	}
	for (INT threads = 0; threads < workThreads; threads++)
		pthread_join(pt[threads], NULL);
	free(pt);
	free(para);
}*/

/*void* getBatchWithNewCorruption2(void* con, INT rand, INT corruptedHead) {
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
	INT counter = 1;
	for (INT batch = lef; batch < rig; batch++) {
			batch_h[batch + last] = negTriple.h;
			batch_t[batch + last] = negTriple.t;
			batch_r[batch + last] = negTriple.r;
			batch_y[batch + last] = -1;

			last += batchSize;
		pthread_exit(NULL);
	}
}*/

int main() {
	importTrainFiles();
	return 0;
}
