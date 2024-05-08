#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Arduino.h"
#include "functional"

class Controller {
  public:
    Controller();
    ~Controller();
    void jobDone(int jobId, bool success);
    void showJobStatus();
    void jobCallback(int jobId, bool ret, int status, String message, int execMillis);
  private:
    bool _jobDone;
    bool _success;
    int _lastJobId;
};

extern Controller g_Controller;
#endif