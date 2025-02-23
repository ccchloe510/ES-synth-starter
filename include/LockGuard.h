#ifndef LOCKGUARD_H
#define LOCKGUARD_H

#include <STM32FreeRTOS.h>

// RAII-style lock guard for FreeRTOS mutexes
class LockGuard {
public:
  // Acquires the semaphore when constructed
  LockGuard(SemaphoreHandle_t sem, TickType_t wait = portMAX_DELAY)
    : sem_(sem) {
      xSemaphoreTake(sem_, wait);
  }
  // Releases the semaphore when the guard goes out of scope
  ~LockGuard() {
    xSemaphoreGive(sem_);
  }
private:
  SemaphoreHandle_t sem_;
};

#endif // LOCKGUARD_H
