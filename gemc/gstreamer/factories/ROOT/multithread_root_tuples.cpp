#include <TFile.h>
#include <TTree.h>
#include <ROOT/TThreadedObject.hxx>
#include <TROOT.h>

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <string>

// example of writing ROOT TTrees in multiple threads
// to compile:
// g++ -std=c++17 -O2 multithread_root_tuples.cpp $(root-config --cflags --libs) -o multithread_root_tuples


// Mutex for thread-safe std::cout
std::mutex cout_mutex;

void writeTuple(int thread_id) {
	std::string filename = "tuple_thread_" + std::to_string(thread_id) + ".root";

	TFile file(filename.c_str(), "RECREATE");
	if (file.IsZombie()) {
		std::lock_guard<std::mutex> lock(cout_mutex);
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	TTree tree("mytree", "Thread-safe example TTree");

	std::vector<int> ids;
	std::vector<float> values;

	tree.Branch("ids", &ids);
	tree.Branch("values", &values);

	// AUTO FLUSH AND AUTOSAVE: flush every ~10 MB (adjust as needed)
	tree.SetAutoFlush(10 * 1024 * 1024); // 10 MB
	tree.SetAutoSave(30 * 1024 * 1024);  // 30 MB autosave snapshots

	const int total_events = 10000000;
	for (int entry = 0; entry < total_events; ++entry) {
		ids.clear();
		values.clear();

		int num_items = 10;
		for (int i = 0; i < num_items; ++i) {
			ids.push_back(entry * num_items + i);
			values.push_back(1.23f * i + thread_id);
		}

		tree.Fill();
	}

	tree.Write(); // Write entire tree metadata and baskets
	file.Close();

	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "Thread " << thread_id << " wrote file: " << filename << std::endl;
}

int main() {
	ROOT::EnableThreadSafety();

	const int num_threads = 8;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_threads; ++i) {
		threads.emplace_back(writeTuple, i);
	}

	for (auto& t : threads) {
		t.join();
	}

	std::cout << "All threads finished writing ROOT files." << std::endl;
	return 0;
}
