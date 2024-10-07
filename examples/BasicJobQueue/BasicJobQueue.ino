#include <Arduino.h>

#include "JobQueues.h"

bool jobDone = false;
int jobsCompleted;
unsigned long heap1;

int queueDiskWriteJob(int j, const char* author, String name);

void myCallback(int jobId, bool ret, String error, int execMillis) {
  Serial.print("JobId "); Serial.print(jobId);
  Serial.print(ret ? " succeeded" : " failed");
  if (!ret) { Serial.print(" error="); Serial.print(error); }
  Serial.print(" ran in "); Serial.print(execMillis); Serial.println("ms");
  
  jobsCompleted += 1;
  if (jobsCompleted == 4) jobDone = true;
}

int normalQueue;

typedef struct {
  int number;
  const char* author;
  String name;
} jobParams_t;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Started");

  normalQueue = JobRunner.addQueue(20, 0);
  
  JobRunner.begin();
  delay(50);
  Serial.print("Idle ?"); Serial.println(JobRunner.isIdle() ? "yes" : "no");
  Serial.println();
  Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
  heap1 = esp_get_free_heap_size();
  Serial.println();

  for (int j = 1; j <= 4; j++) {
    String param2 = "Oliver Twist";
    queueDiskWriteJob(j, "Dickens", param2);
    Serial.println();
    Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
    Serial.print("Idle ?"); Serial.println(JobRunner.isIdle() ? "yes" : "no");
    Serial.println();
  }
}

void loop() {
  while(! jobDone) {
    Serial.println("Foreground activity");
    Serial.print("Idle ?"); Serial.println(JobRunner.isIdle() ? "yes" : "no");
    delay(300);
  }
  Serial.println("All done");
  Serial.println();
  Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
  if (esp_get_free_heap_size() == heap1) {
    Serial.println("No memory leak");
  } else {
    Serial.print("Memory leak - "); Serial.print(heap1 - esp_get_free_heap_size()); Serial.println(" bytes");
  }
  Serial.print("Idle ?"); Serial.println(JobRunner.isIdle() ? "yes" : "no");
  while(1) ;
}

// Queue up a fake disk write job
int queueDiskWriteJob(int j, const char* author, String name) {
    auto params = new jobParams_t {
      j,
      author,
      name,
    };
    std::function<void(void)> writeToDiskJob = [=]() { 
      Serial.print("Writing to file "); Serial.println(params->number);
      delay(500);
      Serial.print("Name: "); Serial.println(params->name);
      delay(1000);
      Serial.print("Author: "); Serial.println(params->author);
      free((void*)params);
      return true; 
  };

  int myJobId = JobRunner.addJob(normalQueue, writeToDiskJob, myCallback);
  Serial.print("Queued jobId = "); Serial.println(myJobId);
  return myJobId;
}
