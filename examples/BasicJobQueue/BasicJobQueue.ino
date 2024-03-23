#include "JobQueues.h"

bool jobDone = false;

void myCallback(int jobId, bool ret, int status, String message, int execMillis) {
  Serial.print("JobId "); Serial.print(jobId);
  Serial.print(ret ? " succeeded " : " failed ");
  Serial.print("Status="); Serial.print(status); 
  Serial.print(" message="); Serial.print(message); 
  Serial.print(" ran in "); Serial.print(execMillis); Serial.println("ms");
  
  if (jobId == 11) jobDone = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Started");

  int normalQueue = JobRunner.addQueue(20, 0);
  int slowQueue = JobRunner.addQueue(20, 1000);

  JobRunner.begin();

  std::function<bool(int&, String&)> writeToDiskJob = [](int& status, String& message) { 
      Serial.println("Writing to disk"); 
      delay(2000);
      Serial.println("Still writing to disk"); 
      delay(1000);
      Serial.println("Finished writing to disk"); 
      status = 13; 
      message = "ok"; 
      return true; 
  };

  Serial.println("Queueing");
  int myJobId1 = JobRunner.addJob(normalQueue, writeToDiskJob, myCallback);
  Serial.print("Queued jobId = "); Serial.println(myJobId1);

  for (int i = 1; i <= 10; i++) {
    std::function<bool(int&, String&)> uploadJob = [i](int& status, String& message) { 
        Serial.print("Uploading "); Serial.println(i);
        delay(500);
        Serial.print("Still uploading "); Serial.println(i);
        delay(100);
        Serial.println("Finished uploading"); 
        status = 42; 
        message = "done " + i; 
        return true; 
    };
    int myUploadJobId = JobRunner.addJob(slowQueue, uploadJob, myCallback);
    Serial.print("Queued JobId = "); Serial.println(myUploadJobId);
  }
  delay(2000);
  int myJobId2 = JobRunner.addJob(normalQueue, writeToDiskJob, myCallback);
  
}

void loop() {
  while(! jobDone) {
    Serial.println("Foreground activity");
    delay(300);
  }
  Serial.println("All done");
  while(1) ;
}


