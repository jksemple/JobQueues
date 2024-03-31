#include "JobQueues.h"
#include "esp_system.h"

bool allDone = false;
int jobsCompleted;
unsigned long heap1;

void myCallback(int jobId, bool ret, int status, String message, int execMillis) {
  Serial.print("JobId "); Serial.print(jobId);
  Serial.print(ret ? " succeeded" : " failed");
  Serial.print(" Status="); Serial.print(status); 
  Serial.print(" message="); Serial.print(message); 
  Serial.print(" ran in "); Serial.print(execMillis); Serial.println("ms");
  
  jobsCompleted += 1;
  if (jobsCompleted == 12) allDone = true;
}

int normalQueue;
int slowQueue;

typedef struct {
  int number;
  const char* version;
  String filename;
} writeJobParams_t;

typedef struct {
  const char* serverURL;
  int port;
  String filename;
} uploadJobParams_t;

int queueWrite(writeJobParams_t* writeJobParams, int queue);
int queueUpload(uploadJobParams_t* uploadJobParams, int queue);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Started");

  normalQueue = JobRunner.addQueue(20, 0);
  slowQueue = JobRunner.addQueue(20, 1000);

  JobRunner.begin();
  delay(50);
  Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
  heap1 = esp_get_free_heap_size();

  // Queue up a fake disk write job
  int jobId = queueWrite1(normalQueue);
  Serial.print("Queued JobId = "); Serial.println(jobId);

  // Queue up 10x fake upload jobs to run on the slower queue
  queueUploads(slowQueue);
  delay(3000);

  // Queue up a second disk write job to run on the normal queue with altered parameters
  // When run this demonstrates the normal queue having priority over the slower queue
  jobId = queueWrite2(normalQueue);
  Serial.print("Queued JobId = "); Serial.println(jobId);

  Serial.println();
  Serial.print("Free heap = "); Serial.println(esp_get_free_heap_size());
  Serial.println();
}

void loop() {
  while (! allDone) {
    Serial.println("Foreground activity running"); 
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
  while (1);
}

int queueWrite(writeJobParams_t* writeJobParams, int queue) {

  std::function<bool(int&, String&)> writeJob = [=](int& status, String& message) { 
      Serial.print("Writing file "); Serial.println(writeJobParams->number);
      delay(2000);
      Serial.print("Filename: "); Serial.println(writeJobParams->filename);
      delay(1000);
      Serial.print("Version: "); Serial.println(writeJobParams->version);
      status = 13; 
      message = "ok"; 

      free ((void*)writeJobParams);
      return true; 
  };
  int myUploadJobId = JobRunner.addJob(queue, writeJob, myCallback);
  return myUploadJobId;
}

int queueWrite1(int queue) {
  auto writeParams1 = new writeJobParams_t {
    1,
    "1.4",
    String("ABC.txt")
  };
  return queueWrite(writeParams1, queue);
}

int queueWrite2(int queue) {
  auto writeParams2 = new writeJobParams_t {
    2,
    "3.5",
    String("DEF.txt")
  };
  return queueWrite(writeParams2, queue);
}

int queueUpload(uploadJobParams_t* uploadJobParams, int queue) {

  std::function<bool(int&, String&)> uploadJob = [=](int& status, String& message) { 
      Serial.print("Uploading to "); Serial.print(uploadJobParams->serverURL); Serial.print(" "); Serial.println(uploadJobParams->port);
      delay(500);
      Serial.print("Filename: "); Serial.println(uploadJobParams->filename);
      delay(100);
      Serial.println("Finished uploading"); 
      status = 42; 
      message = "done " + uploadJobParams->filename; 

      free((void*)uploadJobParams);
      return true; 
  };
  int myUploadJobId = JobRunner.addJob(queue, uploadJob, myCallback);
  return myUploadJobId;
}

void queueUploads(int queue) {
    for (int j = 1; j <= 10; j++) {
    auto uploadParams = new uploadJobParams_t {
      "https://example.com",
      123,
      String("Chapter") + String(j) + String(".txt")
    };
    int job2Id = queueUpload(uploadParams, queue);
    Serial.print("Queued JobId = "); Serial.println(job2Id);
  }
}