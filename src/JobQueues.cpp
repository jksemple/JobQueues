#include "JobQueues.h"

JobRunnerClass::JobRunnerClass () {
}

JobRunnerClass::~JobRunnerClass() {
	
	this->_keepRunning = false;
	
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
void JobRunnerClass::begin(int stackSpace, int taskPriority) {
	log_i("Starting JobRunTask");
	taskParams_t params = {
		_queues,
		_keepRunning
	};
	_keepRunning = true;
	// It fails with a stack size of 2048
	xTaskCreate(JobRunTask, "JobQueues", stackSpace, (void*)&params, taskPriority, NULL);
}

// Queue a job for execution on one of the queues
// An optional callback function will notify when the job has finished and report success or otherwise and execution time
int JobRunnerClass::addJob(int queueNum, std::function<bool(int&, String&)> job, void(*callback)(int jobId, bool ret, int status, String message, int execMillis)) {

  //Serial.println("In addJob");
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

// The service function that executes jobs.  This runs within a separate FreeRTOS Task
void JobRunTask(void* args) {

  taskParams_t* params = (taskParams_t*)args;
  jobQueueEntry_t queueEntry;
  int lastQueueIndex;
  unsigned long lastRunMillis;
  
  Serial.print("JobRunner: ");
  Serial.print(JobRunner._queues.size());
  Serial.println(" queues");
  JobRunner._stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  
  while(JobRunner._keepRunning) {
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
					  int status;
					  String message;
					  unsigned long startMillis = millis();
					  // Execute the job capturing success/failure and possible status/message
					  //Serial.print("Running job "); Serial.println(queueEntry.jobId);
					  log_i("Running job %d", queueEntry.jobId);
					  vTaskDelay(500);
					  auto fn = std::move(queueEntry.job);
					  bool ret = fn(status, message);
					  log_i("Ran job %d", queueEntry.jobId);
					  JobRunner._stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
					  // Call the execution result callback 
					  log_i("Running callback for job %d", queueEntry.jobId);
					  if (queueEntry.callback) {
						  queueEntry.callback(queueEntry.jobId, ret, status, message, millis()-startMillis);
					  }
					  lastQueueIndex = queue.index;
					  lastRunMillis = millis();
					  log_i("Done");
					  // Start another loop iteration;
					  continue; 
				  }
			  }
		  }
	  }
	  vTaskDelay(20);
  }
  vTaskDelete(NULL);
}

JobRunnerClass JobRunner;