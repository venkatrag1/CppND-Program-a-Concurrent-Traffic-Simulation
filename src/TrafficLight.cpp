#include "TrafficLight.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // DONE: FP.5a : The method receive should use std::unique_lock<std::mutex>
  // and _condition.wait() to wait for and receive new messages and pull them
  // from the queue using move semantics. The received object should then be
  // returned by the receive function.
  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] { return !_messages.empty(); });

  T msg = std::move(_messages.back());
  _messages.pop_back();
  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // DONE: FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex> as well as _condition.notify_one() to add a new
  // message to the queue and afterwards send a notification.

  std::lock_guard<std::mutex> uLock(_mutex);
  _messages.push_back(std::move(msg));
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // DONE: FP.5b : add the implementation of the method waitForGreen, in which
  // an infinite while-loop runs and repeatedly calls the receive function on
  // the message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    TrafficLightPhase message = _queue.receive();
    if (message == TrafficLightPhase::green) {
      return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // DONE: FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // DONE: FP.2a : Implement the function with an infinite loop that measures
  // the time between two loop cycles and toggles the current phase of the
  // traffic light between red and green and sends an update method to the
  // message queue using move semantics. The cycle duration should be a random
  // value between 4 and 6 seconds. Also, the while-loop should use
  // std::this_thread::sleep_for to wait 1ms between two cycles.
  auto t_last_updated = std::chrono::high_resolution_clock::now();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> d(
      4000, 6000); // Uniform distribution between 4000ms to 6000ms
  int current_duration_ms = d(gen);

  while (true) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1)); // Reduce CPU util

    auto t_current = std::chrono::high_resolution_clock::now();
    auto elapsed_duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t_current - t_last_updated).count();

    if (elapsed_duration_ms >= current_duration_ms) {
      _currentPhase = (getCurrentPhase() == TrafficLightPhase::red)
                          ? TrafficLightPhase::green
                          : TrafficLightPhase::red;
      t_last_updated = t_current; // Update start time
      current_duration_ms = d(gen); // Resample cycle duration for this light
      TrafficLightPhase currentPhase = getCurrentPhase(); // Make a copy of _currentPhase to move
      _queue.send(std::move(currentPhase));
    }
  }
}
