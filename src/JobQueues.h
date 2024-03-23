#ifndef _JOB_QUEUES_H
#define _JOB_QUEUES_H

#include <Arduino.h>
#include <vector>
#include <functional>

typedef struct {
  int index;
  QueueHandle_t handle;
  int startDelayMillis;
  bool isPaused;
} queue_t;

typedef struct {
  std::function<bool(int&, String&)> job;
  int jobId;
  void(*callback)(int jobId, bool ret, int status, String message, int execMillis);
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
		void begin();
		int addJob(int queueNum, std::function<bool(int&, String&)> job, void(*callback)(int jobId, bool ret, int status, String message, int execMillis) = NULL);
		void pauseQueue(int queueNum = 0);
		int jobCount(int queueNum) { return uxQueueMessagesWaiting(_queues[queueNum].handle); };
		
	private:
		int _jobId;
		bool _keepRunning;
		std::vector<queue_t> _queues;
};

void JobRunTask(void* args);

extern JobRunnerClass JobRunner;
#endif