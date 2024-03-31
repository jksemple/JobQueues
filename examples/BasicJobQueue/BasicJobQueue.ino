#include "JobQueues.h"
#include "esp_system.h"

bool jobDone = false;
int jobsCompleted;
unsigned long heap1;

void myCallback(int jobId, bool ret, int status, String message, int execMillis) {
  Serial.print("JobId "); Serial.print(jobId);
  Serial.print(ret ? " succeeded" : " failed");
  Serial.print(" Status="); Serial.print(status); 
  Serial.print(" message="); Serial.print(message); 
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
  Serial.println();
  Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
  heap1 = esp_get_free_heap_size();
  Serial.println();

  for (int j = 1; j <= 4; j++) {
    String param2 = "Oliver Twist";
    queueDiskWriteJob(j, "Dickens", param2);
    Serial.println();
    Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
    Serial.println();
  }
}

void loop() {
  while(! jobDone) {
    Serial.println("Foreground activity");
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
  while(1) ;
}

// Queue up a fake disk write job
int queueDiskWriteJob(int j, const char* author, String name) {
    auto params = new jobParams_t {
      j,
      author,
      name,
    };
    std::function<bool(int&, String&)> writeToDiskJob = [=](int& status, String& message) { 
      Serial.print("Writing to file "); Serial.println(params->number);
      delay(500);
      Serial.print("Name: "); Serial.println(params->name);
      delay(1000);
      Serial.print("Author: "); Serial.println(params->author);
      status = 13; 
      message = "ok"; 
      free((void*)params);
      return true; 
  };

  int myJobId = JobRunner.addJob(normalQueue, writeToDiskJob, myCallback);
  Serial.print("Queued jobId = "); Serial.println(myJobId);
  return myJobId;
}
