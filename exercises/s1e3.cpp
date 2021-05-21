#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <sstream>
#include <mutex>

using namespace std;

// 2 std::queue type variables
//   - Engine work and Clean work queue
// Engine and Cleaning crew should be represented by one thread each
//   instead of one function each
// Threads should run iteratively while checking for the done flag
//   set by user request (100)
// On each iteration, each thread should check its queue for a command
// If a command is found, execute command and wait for 1 second
// Else, wait 2 seconds
// Captain does not have to wait for any command to finish
// Should take 1 command for every 1 second


std::mutex clean_guard, engine_guard;

enum class Command {
  NoCommand = -1,
  Clean = 1,
  FullSpeedAhead,
  StopEngines,
  Exit = 100
};

istream& operator>>(istream& in, Command& c) {
  int val;

  if (in >> val) {
    switch(val) {
      case 1:
      case 2:
      case 3:
      case 100:
        c = static_cast<Command>(val);
        break;
      default:
        c = Command::NoCommand;
        break;
    }
  }

  return in;
}

void clean(queue<Command>& work) {
  std::ostringstream oss;
  oss << std::this_thread::get_id() << std::endl;
  printf("Clean thread - %s\n", oss.str().c_str());
  while (true) {
    Command task = Command::NoCommand;

    clean_guard.lock();
    if (!work.empty()) {
      // printf("clean_work: %d\n", work.size());
      task = work.back();
      work.pop();
    }
    clean_guard.unlock();

    if (task == Command::Exit) { break; }

    if (task == Command::NoCommand) {
      cout << "Cleaners -- No work to do\n";
      this_thread::sleep_for(std::chrono::milliseconds(2000));
      cout << "Cleaners -- Back to work\n";
      continue;
    }

    if (task == Command::Clean) {
      cout << "Cleaning...\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      cout << "Finished cleaning\n";
    }
  }
}

void engineWork(queue<Command>& work) {
  std::ostringstream oss;
  oss << std::this_thread::get_id() << std::endl;
  printf("Engine thread - %s\n", oss.str().c_str());
  while (true) {
    Command task = Command::NoCommand;

    engine_guard.lock();
    if (!work.empty()) {
      // printf("engine_work: %d\n", work.size());
      task = work.back();
      work.pop();
    }
    engine_guard.unlock();

    if (task == Command::Exit) { break; }

    if (task == Command::NoCommand) {
      cout << "Engine workers -- No work to do\n";
      this_thread::sleep_for(std::chrono::milliseconds(2000));
      cout << "Engine workers -- Back to work\n";
      continue;
    }

    if (task == Command::FullSpeedAhead) { cout << "Throttling up...\n"; }
    if (task == Command::StopEngines) { cout << "Stopping engines...\n"; }

    if (task == Command::FullSpeedAhead || task == Command::StopEngines) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    if (task == Command::FullSpeedAhead) { cout << "Full speed ahead\n"; }
    if (task == Command::StopEngines) { cout << "Engines stopped\n"; }
  }
}

int main()
{
  queue<Command> clean_work, engine_work;

  // Start threads
  std::thread cleaners(clean, ref(clean_work));
  std::thread engineCrew(engineWork, ref(engine_work));
  cleaners.detach();
  engineCrew.detach();

  auto running = true;
  while(running) {
    cout << "Waiting for orders... (1 - clean, 2 - start engines, 3 - stop, 100 - exit)\n";
    Command input;
    cin >> input;

    switch(input) {
      case Command::Clean:
      {
        // cleaning
        clean_work.push(input);
        break;
      }
      case Command::FullSpeedAhead:
      case Command::StopEngines:
      {
        engine_work.push(input);
        break;
      }
      case Command::Exit:
      {
        // exit
        // running = false;
        clean_work.push(input);
        engine_work.push(input);
        break;
      }
      default:
      {
        cout << "Invalid order\n";
        break; 
      }
    }

    std::this_thread::sleep_for(chrono::milliseconds(1000));
  }
}