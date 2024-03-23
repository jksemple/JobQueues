# JobQueues
 
This library allows jobs to be run in a background thread/task on ESP32.
It uses FreeRTOS to run the background task and one or more queues to pass jobs to it.
If multiple queues are setup jobs are executed in queue order so jobs in the first queue will all be executed before any in the second queue etc.
If a job were added to, say, the empty first queue it would be executed when the current job (say from the second queue) has finished. Execution would then resume with the rest of the second queue.
Only one job runs at any time across all the queues.

## Intention

JobQueues was written to allow I/O blocking operations such as reading and writing to disk or uploading/downloading to the Net to be separated from time critical foreground activities.  
It replaces what were a couple of very application-specific background tasks with a single generic job queueing system that is re-useable across projects.

## Configuration

Queues need to be added to the JobRunner object (.addQueue) before it is started. 
Each successive queue is lower in the execution order than the previous one.
A queue can optionally be given a 'startDelayMillis' parameter which stops jobs in that queue from being executed for a period of time if the last job executed was in an earlier queue.  
This stops a lower priority, long-running job from blocking what might be a sparse stream of higher priority jobs that are not quite frequent enough to keep their queue full.
The background task can be started at any FreeRTOS Priority level defaulting to 1.

## Job queueing

Jobs are added to the queues (.addJob) as a std::function wrapper around a lambda expression that contains the code to be run. 
Any parameters that are to be passed in to the job need to be defined as captures in the lambda and care should be taken to ensure that any stack allocated objects passed in will not go out of scope when the calling method has gone out of scope.  
This typically means using malloc or strdup to copy objects and then freeing them within the lambda before execution ends.

## Function Descriptions

#### JobRunner

A singleton JobRunner object is instantiated as the entry point to all functionality.  Initially it has no queues. At least one must be added

#### Add a queue

param **`int queueLength`** The number of jobs allowed to be queued before the queue is full

param **`int startDelayMillis`** The number of millisecs that execution will be delayed for jobs in this queue if the last job was in a higher priority (lower-numbered) queue

return **`int queueNum`** The number for the queue that has been added

```cpp
int addQueue(int queueLength, int startDelayMillis);
```

#### Start JobRunner

param **`int taskPriority`** The FreeRTOS priority for the background task defaulting to 1 but can be set higher (up to 32?) as needed

```cpp
void begin(int taskPriority = 1);
```

#### Add a Job

param **`int queueNum`** The 0-based index of the queue to add to

param **`std::function<bool(int&, String&)> job`** The lambda expression to be executed in the background task

param **`void (*callback)(int jobId, bool ret, int status, String message, int execMillis)`** a callback function through which to receive notification when the job has been executed

return **`int jobId`** The id of the Job for later correlation with execution notifications. Zero if the queue is full

```cpp
int addJob(int queueNum, std::function<bool(int&, String&)> job, void(*callback)(int jobId, bool ret, int status, String message, int execMillis) = NULL);
```

#### Pause queue

param **`int queueNum`** The 0-based index of the queue to pause

```cpp
void pauseQueue(int queueNum = 0);
```
#### Queue size

param **`int queueNum`** The 0-based index of the queue to query

return **`int count`** The number of jobs in this queue

```cpp
int jobCount(int queueNum = 0);
```

The MIT License (MIT)

Copyright (c) 2024 Jonathan Semple (jksemple)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

