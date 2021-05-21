#include <iostream>
#include <string>
#include <thread>

using namespace std;

void clean() {
  cout << "Cleaning..." << endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  cout << "Finished cleaning" << endl;
}

void fullSpeedAhead() {
  cout << "Throttling up..." << endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  cout << "Full speed ahead" << endl;
}

void stopEngines() {
  cout << "Stopping engines..." << endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  cout << "Engines stopped" << endl;
}

int main()
{
  auto running = true;
  while(running) {
    cout << "Waiting for orders... (1 - clean, 2 - start engines, 3 - stop, 100 - exit)" << endl;
    string input = "";
    cin >> input;

    auto input_val = -1;

    try {
      input_val = stoi(input);
    } catch(exception& e) {
      // fuckin idiot
    }

    switch(input_val) {
      case 1:
      {
        // cleaning
        std::thread cleaners(clean);
        cleaners.detach();
        break;
      }
      case 2:
      {
        // full speed ahead
        std::thread engineCrewStart(fullSpeedAhead);
        engineCrewStart.join();
        break;
      }
      case 3:
      {
        // stop
        std::thread engineCrewStop(stopEngines);
        engineCrewStop.join();
        break;
      }
      case 100:
      {
        // exit
        running = false;
        break;
      }
      default:
      {
        cout << "Invalid order" << endl;
        break; 
      }
    }
  }
}