#include "Controller.h"

Controller::Controller() {
  _jobDone = false;
  _success = false;
}

Controller::~Controller() {}

void Controller::jobDone(int jobId, bool success) {
  _lastJobId = jobId;
  _jobDone = true;
  _success = success;
}

void Controller::showJobStatus() {
  log_i("Jobstatus: lastJobId %d done %d, success %d", _lastJobId, _jobDone, _success);
}
