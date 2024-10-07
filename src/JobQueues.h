#ifndef _JOB_QUEUES_H
#define _JOB_QUEUES_H

#include <Arduino.h>
#include <vector>
#include <functional>

#define JOBFUNC std::function<void(void)>
#define JOBCALLBACK(f) void(*f)(int jobId, bool ret, String message, int execMillis)

typedef struct {
  int index;
  QueueHandle_t handle;
  int startDelayMillis;
  bool isPaused;
} queue_t;

typedef struct {
  std::function<void(void)> job;
  int jobId;
  void(*callback)(int jobId, bool ret, String message, int execMillis);
} jobQueueEntry_t;

typedef struct {
  std::vector<queue_t> queues;
  bool keepRunning;
} taskParams_t;

class JobRunnerClass {
	friend void JobRunTask(void* args);
	public:
		JobRunnerClass();
		~JobRunnerClass();
		int addQueue(int queueLength, int startDelayMillis);
		void begin(int stackSpace = 4096, int taskPriority = 1);
		void end();
		int addJob(int queueNum, std::function<void(void)> job, void(*callback)(int jobId, bool ret, String message, int execMillis) = NULL);
		void pauseQueue(int queueNum = 0);
		int jobCount(int queueNum) { return uxQueueMessagesWaiting(_queues[queueNum].handle); };
		bool isIdle() { return _idle; }
		int jobCount();
		int getStackHighWaterMark() { return _stackHighWaterMark; };
		
	private:
		int _jobId;
		bool _keepRunning;
		bool _idle;
		int _taskPriority;
		int _stackHighWaterMark;
		std::vector<queue_t> _queues;
};

void JobRunTask(void* args);

extern JobRunnerClass JobRunner;
#endif