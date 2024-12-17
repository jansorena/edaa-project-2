/** uhr: generic time performance tester
 * Author: LELE
 *
 * Things to set up:
 * 0. Includes: include all files to be tested,
 * 1. Time unit: in elapsed_time,
 * 2. What to write on time_data,
 * 3. Data type and distribution of RNG,
 * 4. Additive or multiplicative stepping,
 * 5. The experiments: in outer for loop. */

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

// Include to be tested files here
#include "../src/suffix_array.cpp"
#include "../src/suffix_array_lcp.cpp"

inline void validate_input(int argc, char *argv[], std::int64_t& runs,
    std::int64_t& lower, std::int64_t& upper, std::int64_t& step)
{
    if (argc != 6) {
        std::cerr << "Usage: <filename> <RUNS> <LOWER> <UPPER> <STEP>" << std::endl;
        std::cerr << "<filename> is the name of the file where performance data will be written." << std::endl;
        std::cerr << "It is recommended for <filename> to have .csv extension and it should not previously exist." << std::endl;
        std::cerr << "<RUNS>: numbers of runs per test case: should be >= 32." << std::endl;
        std::cerr << "<LOWER> <UPPER> <STEP>: range of test cases." << std::endl;
        std::cerr << "These should all be positive." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Read command line arguments
    try {
        runs = std::stoll(argv[2]);
        lower = std::stoll(argv[3]);
        upper = std::stoll(argv[4]);
        step = std::stoll(argv[5]);
    } catch (std::invalid_argument const& ex) {
        std::cerr << "std::invalid_argument::what(): " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (std::out_of_range const& ex) {
        std::cerr << "std::out_of_range::what(): " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Validate arguments
    if (runs < 4) {
        std::cerr << "<RUNS> must be at least 4." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (step <= 0 or lower <= 0 or upper <= 0) {
        std::cerr << "<STEP>, <LOWER> and <UPPER> have to be positive." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (lower > upper) {
        std::cerr << "<LOWER> must be at most equal to <UPPER>." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

inline void display_progress(std::int64_t u, std::int64_t v)
{
    const double progress = u / double(v);
    const std::int64_t width = 70;
    const std::int64_t p = width * progress;
    std::int64_t i;

    std::cout << "\033[1m[";
    for (i = 0; i < width; i++) {
        if (i < p)
            std::cout << "=";
        else if (i == p)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << std::int64_t(progress * 100.0) << "%\r\033[0m";
    std::cout.flush();
}

inline void quartiles(std::vector<double>& data, std::vector<double>& q)
{
    q.resize(5);
    std::size_t n = data.size();
    std::size_t p;

    std::sort(data.begin(), data.end());

    if (n < 4) {
        std::cerr << "quartiles needs at least 4 data points." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Get min and max
    q[0] = data.front();
    q[4] = data.back();

    // Find median
    if (n % 2 == 1) {
        q[2] = data[n / 2];
    } else {
        p = n / 2;
        q[2] = (data[p - 1] + data[p]) / 2.0;
    }

    // Find lower and upper quartiles
    if (n % 4 >= 2) {
        q[1] = data[n / 4];
        q[3] = data[(3 * n) / 4];
    } else {
        p = n / 4;
        q[1] = 0.25 * data[p - 1] + 0.75 * data[p];
        p = (3 * n) / 4;
        q[3] = 0.75 * data[p - 1] + 0.25 * data[p];
    }
}

std::string load_text(const std::string& filename, size_t max_size = 2ULL * 1024 * 1024 * 1024){
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    size_t read_size = std::min(file_size, max_size);
    file.seekg(0, std::ios::beg);

    // Read limited amount
    std::string text(read_size, '\0');
    file.read(&text[0], read_size);
    
    return text;
}

// Get a random pattern from a text
std::string get_random_pattern(const std::string& text, std::mt19937_64& rng, std::uniform_int_distribution<std::int64_t>& u_distr, std::int64_t pattern_length) {
    std::int64_t text_length = text.size();
    std::int64_t start = u_distr(rng) % (text_length - pattern_length);
    return text.substr(start, pattern_length);
}

std::string get_pattern(const std::string& text, std::mt19937_64& rng, std::uniform_int_distribution<std::int64_t>& u_distr, std::int64_t pattern_length) {
    std::int64_t text_length = text.size();
    std::int64_t start = u_distr(rng) % (text_length - pattern_length);
    return text.substr(start, pattern_length);
}


int main(int argc, char *argv[])
{
    // Validate and sanitize input
    std::int64_t runs, lower, upper, step;
    validate_input(argc, argv, runs, lower, upper, step);

    // Set up clock variables
    std::int64_t n, i, executed_runs;
    std::int64_t total_runs_additive = runs * (((upper - lower) / step) + 1);
    std::int64_t total_runs_multiplicative = runs * (floor(log(upper / double(lower)) / log(step)) + 1);
    std::vector<double> times(runs);
    std::vector<double> q;
    double mean_time, time_stdev, dev;
    auto begin_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> elapsed_time = end_time - begin_time;

    // Set up random number generation
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<std::int64_t> u_distr; // change depending on app

    // File to write time data
    std::ofstream time_data;
    time_data.open(argv[1]);
    time_data << "n,t_mean,t_stdev,t_Q0,t_Q1,t_Q2,t_Q3,t_Q4" << std::endl;

    std::ofstream construct_data;
    construct_data.open("construct_data.csv");
    construct_data << "n,time,space" << std::endl;

    // Begin testing
    std::cout << "\033[0;36mRunning tests...\033[0m" << std::endl << std::endl;
    executed_runs = 0;
    
    // Vector of text files
    std::string path = "/home/dataset/";
    //std::vector<std::string> text_files = {"sources", "dna", "proteins", "GCF_000001405.40_GRCh38.p14_genomic.fna"};
    //std::vector<std::string> text_files = {"sources", "dna", "proteins"};
    std::vector<std::string> text_files = {"sources"};
    //std::vector<std::string> text_files = {"GCF_000001405.40_GRCh38.p14_genomic.fna"};

    // Load text
    std::string text = load_text(path+text_files[0]);

    // Construct suffix array
    begin_time = std::chrono::high_resolution_clock::now();
    suffix_array_lcp salcp(text);
    end_time = std::chrono::high_resolution_clock::now();
    elapsed_time = end_time - begin_time;

    construct_data << text_files[0] << "," << elapsed_time.count() << "," << salcp.memory_usage() << std::endl;

    std::string text_pattern = load_text("pattern.txt");
    std::ofstream pattern_file("patternCheck.txt", std::ios::app);
    std::int64_t pattern_start = 0;        
    for (n = lower; n <= upper; n += step) {
        mean_time = 0;
        time_stdev = 0;

        // Generate random pattern
        std::int64_t pattern_length = n;        
        std::string pattern = text_pattern.substr(pattern_start, pattern_length);
        pattern_start += pattern_length;
        pattern_file << pattern << std::endl;

        //std::cout << "Pattern: " << pattern << std::endl;

        // Run to compute elapsed time (128)
        for (i = 0; i < runs; i++) {
            // Remember to change total depending on step type
            display_progress(++executed_runs, total_runs_additive);

            begin_time = std::chrono::high_resolution_clock::now();
            salcp.count(pattern);
            end_time = std::chrono::high_resolution_clock::now();

            elapsed_time = end_time - begin_time;
            times[i] = elapsed_time.count();

            mean_time += times[i];
        }

        // Compute statistics
        mean_time /= runs;

        for (i = 0; i < runs; i++) {
            dev = times[i] - mean_time;
            time_stdev += dev * dev;
        }

        time_stdev /= runs - 1; // Subtract 1 to get unbiased estimator
        time_stdev = std::sqrt(time_stdev);

        quartiles(times, q);

        time_data << pattern_length << "," << mean_time << "," << time_stdev << ",";
        time_data << q[0] << "," << q[1] << "," << q[2] << "," << q[3] << "," << q[4] << std::endl;
    }

    // This is to keep loading bar after testing
    std::cout << std::endl << std::endl;
    std::cout << "\033[1;32mDone!\033[0m" << std::endl;
    pattern_file.close();
    time_data.close();

    return 0;
}
