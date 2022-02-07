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


	void setParams(struct model_params * _params) {
		params = _params;
		setlowvec(_params->bugdepth);
		}

	void setlowvec(int bugdepth){
		if(bugdepth > 1){
			lowvec.resize(bugdepth - 1,-1);
		}
		else lowvec.resize(1);
		
	}

	// void set_chg_pts(int bugdepth){
	// 	if(bugdepth > 1){
	// 		lowvec.resize(bugdepth - 1,-1);
	// 	}
	// 	else lowvec.resize(1);
		
	// }

	void print_lowvec(){
		for(uint64_t i = 0; i < lowvec.size(); i ++){
			model_print("%u: %d  ", i, lowvec[i]);
		}

	}
	////PCT - scheduler length
	// void incScheLen(){curr_sche_len++;}
	// int getScheLen(){return curr_sche_len;}


	////PCT params
	//void setParams(struct model_params * _params) {params = _params;}
	SNAPSHOTALLOC
private:

	//struct model_params * params;
	ModelExecution *execution;
	/** The list of available Threads that are not currently running */
	enabled_type_t *enabled;
	int enabled_len;
	int curr_thread_index;
	void set_enabled(Thread *t, enabled_type_t enabled_status);

	/** The currently-running Thread */
	Thread *current;


	// int num_instr;
	// int curr_sche_len;

	//PCT
	struct model_params * params;

	SnapVector<int> lowvec;
	//SnapVector<int> chg_pts;
	
};

#endif	/* __SCHEDULE_H__ */
