#ifndef FUZZER_H
#define FUZZER_H
#include "classlist.h"
#include "mymemory.h"
#include "stl-model.h"
#include "threads-model.h"

class Fuzzer {
public:
	Fuzzer() {}
	virtual int selectWrite(ModelAction *read, SnapVector<ModelAction *>* rf_set);
	
	//pctwm - two kinds of select write values
	virtual int selectWriteMyThread(ModelAction *read, SnapVector<ModelAction *>* rf_set, int tid);
	virtual int selectWriteOtherThread(ModelAction *read, SnapVector<ModelAction *>* rf_set, int tid);

	virtual bool has_paused_threads() { return false; }
	virtual Thread * selectThread(int * threadlist, int numthreads);
	virtual Thread *selectThreadbyid(int threadid);

	Thread * selectNotify(simple_action_list_t * waiters);
	bool shouldSleep(const ModelAction *sleep);
	bool shouldWake(const ModelAction *sleep);
	virtual bool waitShouldFail(ModelAction *wait);
	bool waitShouldWakeUp(const ModelAction *wait);
	bool randomizeWaitTime(ModelAction * timed_wait);
	virtual void register_engine(ModelChecker * _model, ModelExecution * execution) {}
	SNAPSHOTALLOC
private:
};
#endif
