/** @file schedule.h
 *	@brief Thread scheduler.
 */

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "mymemory.h"
#include "modeltypes.h"
#include "classlist.h"
#include "params.h"

typedef enum enabled_type {
	THREAD_DISABLED,
	THREAD_ENABLED,
	THREAD_SLEEP_SET
} enabled_type_t;

void enabled_type_to_string(enabled_type_t e, char *str);

/** @brief The Scheduler class performs the mechanics of Thread execution
 * scheduling. */
class Scheduler {
public:
	Scheduler();
	void register_engine(ModelExecution *execution);

	void add_thread(Thread *t);
	void remove_thread(Thread *t);
	void sleep(Thread *t);
	void wake(Thread *t);
	Thread * select_next_thread();
	void set_current_thread(Thread *t);
	Thread * get_current_thread() const;
	void print() const;
	enabled_type_t * get_enabled_array() const { return enabled; };
	void remove_sleep(Thread *t);
	void add_sleep(Thread *t);
	enabled_type_t get_enabled(const Thread *t) const;
	bool is_enabled(const Thread *t) const;
	bool is_enabled(thread_id_t tid) const;
	bool is_sleep_set(const Thread *t) const;
	bool is_sleep_set(thread_id_t tid) const;
	bool all_threads_sleeping() const;
	void set_scheduler_thread(thread_id_t tid);

	// related funcs
	uint64_t scheduler_get_nanotime()
	{
		struct timespec currtime;
		clock_gettime(CLOCK_MONOTONIC, &currtime);

		return currtime.tv_nsec;
	}

	void setParams(struct model_params * _params) {
		params = _params;
		setlowvec(params->bugdepth);
		set_chg_pts_byread(params->bugdepth, params->maxinstr);
		schelen_limit = 2 * params->maxscheduler;
		if(params->version == 1) {
			model_print("using pct version now. \n");
			pctactive();
		}
		else model_print("using c11tester original version now. \n");
		print_chg();
	}


	void setlowvec(int bugdepth){
		if(bugdepth > 1){
			lowvec.resize(bugdepth - 1,-1);
		}
		else lowvec.resize(1);
		
	}

	// void set_chg_pts(int bugdepth, int maxscheduler){
	// 	if(bugdepth <= 1){
	// 		chg_pts.resize(1, srand() % maxscheduler);
	// 	}
	// 	else{
	// 		chg_pts.resize(bugdepth - 1);
	// 		for(int i = 0; i < bugdepth - 1; i++){
	// 			int tmp = getRandom(maxscheduler); // [1, MAXSCHEDULER]
	// 			while(chg_pts.find(tmp)){
	// 				tmp = getRandom(maxscheduler);
	// 			}
	// 			chg_pts[i] = tmp;

	// 		}
	// 	}
		
	// }

	//pctwm
	void set_chg_pts_byread(int bugdepth, int maxinstr){
		if(bugdepth <= 1){
			chg_pts.resize(1,  getRandom(maxinstr));
		}
		else{
			chg_pts.resize(bugdepth - 1);
			for(int i = 0; i < bugdepth - 1; i++){
				int tmp = getRandom(maxinstr); // [1, MAXSCHEDULER]
				while(chg_pts.find(tmp)){
					tmp = getRandom(maxinstr);
				}
				chg_pts[i] = tmp;

			}
			for(int i = 0; i < bugdepth - 1; i++){
				for(int j = 1; j < bugdepth - 1; j++){
					if(chg_pts[j - 1] > chg_pts[j]){
						int tmp = chg_pts[j - 1];
						chg_pts[j - 1] = chg_pts[j];
						chg_pts[j] = tmp;
					}
				}
			}
		}
		
		// resort 
		

		
		
		
	}


	//pctwm - return bool: true : threadid in highvec(not change prio yet)
	bool inhighvec(int threadid){
		for(int i = 0; i < highsize; i++){
			if(highvec[i] == threadid) return true;
		}
		return false;
	}



	int getRandom(int range){

		//uint64_t seed = scheduler_get_nanotime();
		uint64_t seed = 19;
		srandom(seed);
				
		int res =  rand() % range;
		res = res < 1 ? 1 : res;
		return res;
	}


	void print_chg(){
		model_print("Change Priority Points:  ");
		for(uint64_t i = 0; i < chg_pts.size(); i++){
			model_print("[%u]: %d  ", i, chg_pts[i]);
		}
		model_print("\n");

	}


	void print_lowvec(){
		model_print("Low priority threads:  ");
		for(uint64_t i = 0; i < lowvec.size(); i++){
			model_print("[%u]: %d  ", i, lowvec[i]);
		}
		model_print("\n");

	}

	void incSchelen(){
		schelen++;
	}

	int getSchelen(){
		return schelen;
	}

	int find_chgidx(int currlen){ // rename the parameter to currlen - it may be readnum
		print_chg();
		int res = -1;
		for(uint i = 0; i < chg_pts.size(); i++){
			if(currlen == chg_pts[i]) res = i;
		}
		return res;
	}

	void highvec_addthread(Thread *t);

	void print_highvec(){
		model_print("high priority vector: ");
		for(int i = 0; i < highsize; i++){
			model_print("[%d] : %d", i, highvec[i]);
		}
		model_print("\n");
	}


	void print_avails(int* availthreads, int availnum);
	int find_highest(int* availthreads, int availnum);
	void movethread(int lowvec_idx, int threadid);
	void pctactive(){
		usingpct = 1;
	}

	void print_current_avail_threads(){
		int availnum = 0;
		int availthreads[enabled_len];
	
		for (int i = 0;i < enabled_len;i++) {
			if (enabled[i] == THREAD_ENABLED)
				availthreads[availnum++] = i;
		}

		model_print("current %d avail threads.", availnum);
		for(int i = 0; i < availnum; i++){
			model_print("thread: %d, ", availthreads[i]);
		}
		model_print("\n");
	}

	//weak memory
	int get_highest_thread(){
		int availnum = 0;
		int availthreads[enabled_len];
	
		for (int i = 0;i < enabled_len;i++) {
			if (enabled[i] == THREAD_ENABLED)
				availthreads[availnum++] = i;
		}
		// only one thread is available
		if(availnum == 1){
			return availthreads[0];
		}

		uint findhigh = 0;
		while(findhigh < highvec.size()){
			for(int i = 0; i < availnum; i++){
				if(availthreads[i] == highvec[findhigh]){
					return availthreads[i];
				}
			}
		findhigh++;
		}

		uint findlow = 0;
		while(findlow < lowvec.size()){
			for(int i = 0; i < availnum; i++){
				if(availthreads[i] == lowvec[findlow]){
					return availthreads[i];
				}
			}
		findlow++;
		}

		return -1;
		
	}


	int get_scecond_high_thread(){
		int availnum = 0;
		int availthreads[enabled_len];
	
		for (int i = 0;i < enabled_len;i++) {
			if (enabled[i] == THREAD_ENABLED)
				availthreads[availnum++] = i;
		}
		// only one thread is available
		if(availnum == 1){
			return availthreads[0];
		}

		int highest1 = -1;
		int highest2 = -1;
		

		uint findhigh = 0;
		while(findhigh < highvec.size()){
			for(int i = 0; i < availnum; i++){
				if(availthreads[i] == highvec[findhigh]){
					if(highest1 != -1){
						highest2 = availthreads[i];
						return highest2;
					}
					else{
						highest1 = availthreads[i];
					}
					
				}
			}
		findhigh++;
		}

		if((highest1 == -1) || (highest2 == -1)){
			uint findlow = 0;
			while(findlow < lowvec.size()){
				for(int i = 0; i < availnum; i++){
					if(availthreads[i] == lowvec[findlow]){
						if(highest1 != -1){
							highest2 = availthreads[i];
							return highest2;
						}
						else{
							highest1 = availthreads[i];
						}
					
					}
				}
			findlow++;
			}
		}

		return highest2;

	}

			// weak memory model
	void add_external_readnum_thread(uint threadid){
		
		if (threadid >= external_readnum_thread.size()){
			int diff = threadid  - external_readnum_thread.size() + 1;
			for(int i = 0; i < diff; i++){
				external_readnum_thread.push_back(false);
			}
		}
		external_readnum_thread[threadid] = true;
		

	}

	bool deleteone_external_readnum_thread(uint threadid){
		
		if (threadid >= external_readnum_thread.size()){
			return false;
			}
		else{
			if(external_readnum_thread[threadid] > 0){
				external_readnum_thread[threadid] = false;
				return true;
			}
			else return false;
			
		}

		return true;

	}

	bool get_external_readnum_thread(uint threadid){
		
		if (threadid >= external_readnum_thread.size()){
			external_readnum_thread.push_back(false);
			return false;
		}
		else{
			return external_readnum_thread[threadid];
		}
	}

	void print_external_readnum_thread(){
		model_print("external_readnum job each thread: ");
		for(uint i = 0; i < external_readnum_thread.size(); i++){
			if(external_readnum_thread[i]){
				model_print("thread : %d need read externally. ", i);
			}
			else{
				model_print("thread : %d does not need read externally. ", i);
			}
			
		}
		model_print("\n");
	}

	


	SNAPSHOTALLOC
private:
	ModelExecution *execution;
	/** The list of available Threads that are not currently running */
	enabled_type_t *enabled;
	int enabled_len;
	int curr_thread_index;
	void set_enabled(Thread *t, enabled_type_t enabled_status);

	/** The currently-running Thread */
	Thread *current;

	//PCT
	struct model_params * params;

	SnapVector<int> lowvec;
	SnapVector<int> chg_pts;
	int schelen;

	SnapVector<int> highvec;
	int highsize;
	int schelen_limit;
	bool livelock;
	int usingpct;

	// weak memory - save the highest thread - for execution.cc to move
	int highest_id;

	SnapVector<bool> external_readnum_thread;

};

#endif	/* __SCHEDULE_H__ */
