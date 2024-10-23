#include "JobQueues.h"

JobRunnerClass::JobRunnerClass () {
}

JobRunnerClass::~JobRunnerClass() {
	
	_keepRunning = false;
	
	for (auto queue : _queues) {
		vQueueDelete(queue.handle);
	}
}
// Queue config method to add a queue to the JobRunner's set of queues
// queueLength = number of items the queue can hold before it is full
// startDelayMillis = millisecs to wait before first starting to execute
// jobs on this queue if jobs from a higher priority queue were last being
// executed.
int JobRunnerClass::addQueue(int queueLength = 20, int startDelayMillis = 0) {
	auto handle = xQueueCreate(queueLength, sizeof(jobQueueEntry_t));
	int index = _queues.size();
	queue_t queue = {
		index,
		handle,
		startDelayMillis,
		false
	};
	_queues.push_back(queue);
	return index;
}
// Start the JobRunner going such that jobs queued start to be executed
void JobRunnerClass::begin(int stackSpace, int taskPriority, void(*callback)(int jobId, bool ret, String message, int execMillis)) {
	log_i("Starting JobRunTask");
	//taskParams_t params = {
	//	_queues,
	//	_keepRunning,
	//	_idle
	//};
	_keepRunning = true;
	_idle = true;
	_defaultCallback = callback;
	// It fails with a stack size of 2048
	xTaskCreate(JobRunTask, "JobQueues", stackSpace, (void*)this, taskPriority, NULL);
}

void JobRunnerClass::end() {
	_keepRunning = false; 
}

// Queue a job for execution on one of the queues
// An optional callback function will notify when the job has finished and report success or otherwise and execution time
int JobRunnerClass::addJob(int queueNum, std::function<void(void)> job, void(*callback)(int jobId, bool ret, String message, int execMillis)) {

  //Serial.println("In addJob");
  _idle = false;
  jobQueueEntry_t queueEntry = {
	  job,
	  _jobId + 1,
	  callback
  };

  if (xQueueSend(_queues[queueNum].handle, (void*)&queueEntry, 100)) {
	return ++_jobId;
  } else {
	return 0;
  }
}
int JobRunnerClass::jobCount() {
	int count = 0;
	for (auto queue : _queues) {
		count += jobCount(queue.index);
	}
	return count;
}

// The service function that executes jobs.  This runs within a separate FreeRTOS Task
void JobRunTask(void* args) {

  auto jobRunner = (JobRunnerClass*)args;
  jobQueueEntry_t queueEntry;
  int lastQueueIndex;
  unsigned long lastRunMillis;
  
  Serial.print("JobRunner: ");
  Serial.print(JobRunner._queues.size());
  Serial.println(" queues");
  jobRunner->_stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  
  while(jobRunner->_keepRunning) {
	  // Iterate through queues looking for a non-paused queue
	  for(auto queue : JobRunner._queues) {
		  if (! queue.isPaused) {
			  // ...that is not empty
			  if (uxQueueMessagesWaiting(queue.handle) > 0) {
  			  
				  // Delay starting jobs from a new lower priority queue
				  if (queue.index > lastQueueIndex) {
			  
					  if (millis() - lastRunMillis < queue.startDelayMillis) {
						  //Serial.println("start delay...");
						  continue;
					  }
				  }
				  //log_i("Reading from Q %d", queue.index);
				  if (xQueueReceive(queue.handle, &queueEntry, 10) == pdPASS) {
					  jobRunner->_idle = false;
					  int status;
					  String message;
					  unsigned long startMillis = millis();
					  // Execute the job capturing success/failure and possible status/message
					  //Serial.print("Running job "); Serial.println(queueEntry.jobId);
					  log_i("Running job %d", queueEntry.jobId);
					  vTaskDelay(10);
					  auto fn = std::move(queueEntry.job);
					  bool success = true;
					  String error = "";
					  try {
						fn();
					  } catch (std::exception const& ex) {
						error = ex.what();
						success = false;
					  }
					  //log_i("Ran job %d", queueEntry.jobId);
					  JobRunner._stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
					  // Call the execution result callback 
					  //log_i("Running callback for job %d", queueEntry.jobId);
					  JOBCALLBACK(cb) = queueEntry.callback != nullptr ? queueEntry.callback : _defaultCallback;
					  if (cb) {
						  cb(queueEntry.jobId, success, error, millis()-startMillis);
					  }
					  lastQueueIndex = queue.index;
					  lastRunMillis = millis();
					  //log_i("Done");
					  // Start another loop iteration;
					  continue; 
				  }
			  }
		  }
	  }
	  jobRunner->_idle = jobRunner->jobCount() == 0;
	  vTaskDelay(10);
  }
  vTaskDelete(NULL);
}

JobRunnerClass JobRunner;