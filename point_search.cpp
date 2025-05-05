#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <vector>
#include <algorithm>
#include <thread>
#include <cmath>

#include "Ec.h"
#include "bloom/filter.hpp"

using namespace std;

inline uint64_t str_to_uint64(std::string const& value) {
  uint64_t result = 0;
  size_t const length = value.size();
  switch (length) {
    case 20:    result += (value[length - 20] - '0') * 10000000000000000000ULL;
    case 19:    result += (value[length - 19] - '0') * 1000000000000000000ULL;
    case 18:    result += (value[length - 18] - '0') * 100000000000000000ULL;
    case 17:    result += (value[length - 17] - '0') * 10000000000000000ULL;
    case 16:    result += (value[length - 16] - '0') * 1000000000000000ULL;
    case 15:    result += (value[length - 15] - '0') * 100000000000000ULL;
    case 14:    result += (value[length - 14] - '0') * 10000000000000ULL;
    case 13:    result += (value[length - 13] - '0') * 1000000000000ULL;
    case 12:    result += (value[length - 12] - '0') * 100000000000ULL;
    case 11:    result += (value[length - 11] - '0') * 10000000000ULL;
    case 10:    result += (value[length - 10] - '0') * 1000000000ULL;
    case  9:    result += (value[length -  9] - '0') * 100000000ULL;
    case  8:    result += (value[length -  8] - '0') * 10000000ULL;
    case  7:    result += (value[length -  7] - '0') * 1000000ULL;
    case  6:    result += (value[length -  6] - '0') * 100000ULL;
    case  5:    result += (value[length -  5] - '0') * 10000ULL;
    case  4:    result += (value[length -  4] - '0') * 1000ULL;
    case  3:    result += (value[length -  3] - '0') * 100ULL;
    case  2:    result += (value[length -  2] - '0') * 10ULL;
    case  1:    result += (value[length -  1] - '0');
  }
  return result;
}

std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) ++start;
    auto end = str.end();
    do { --end; } while (end != start && std::isspace(*end));
    return std::string(start, end + 1);
}

void print_time() {
    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char output[50];
    strftime(output, 50, "%H:%M:%S", &datetime);
    cout << "[" << output << "] ";
}

vector<uint64_t> break_down_to_pow10(uint64_t num) {
    vector<uint64_t> nums;
    string stri = to_string(num);
    int num_len = stri.length() - 2;
    for (int pw = num_len; pw >= 0; pw--) { nums.push_back(uint64_t(pow(10, pw))); }
    return nums;
}

auto main() -> int {
	Ec ec; InitEc();

	EcInt pk; pk.Set(1);
    uint64_t mult = 2;
    vector<EcInt> S_table;
    for (int i = 0; i < 256; i++)
    {
        S_table.push_back(pk);
        pk.Mul_u64(pk, mult);
    }
    print_time(); cout << "S_table generated" << endl;

    uint64_t range_start, range_end, block_width;
    string temp, search_pub;
    ifstream inFile("settings.txt");
    getline(inFile, temp); range_start = str_to_uint64(temp);
    getline(inFile, temp); range_end = str_to_uint64(temp);
    getline(inFile, temp); block_width = str_to_uint64(temp);
    getline(inFile, temp); search_pub = trim(temp);
    inFile.close();
    print_time(); cout << "Range Start: " << range_start << " bits" << endl;
    print_time(); cout << "Range End  : " << range_end << " bits" << endl;
    print_time(); cout << "Block Width: 2^" << block_width << endl;
    print_time(); cout << "Search Pub : " << search_pub << endl;

    EcInt first_scalar, second_scalar, pre_calc_sum;
    first_scalar.Assign(S_table[range_start - 1]);
    second_scalar.Assign(S_table[range_start - 2]);
    first_scalar.Add(second_scalar);
    pre_calc_sum.Assign(first_scalar);
    
    string bloomfile1 = "bloom1.bf";
    string bloomfile2 = "bloom2.bf";
    using filter = boost::bloom::filter<std::string, 32>;
    
    print_time(); cout << "Loading Bloomfilter bloom1.bf" << endl;
    filter bf1;
    std::ifstream in1(bloomfile1, std::ios::binary);
    std::size_t c1;
    in1.read((char*) &c1, sizeof(c1));
    bf1.reset(c1); // restore capacity
    boost::span<unsigned char> s1 = bf1.array();
    in1.read((char*) s1.data(), s1.size()); // load array
    in1.close();

    
    print_time(); cout << "Loading Bloomfilter bloom2.bf" << endl;
    filter bf2;
    std::ifstream in2(bloomfile2, std::ios::binary);
    std::size_t c2;
    in2.read((char*) &c2, sizeof(c2));
    bf2.reset(c2); // restore capacity
    boost::span<unsigned char> s2 = bf2.array();
    in2.read((char*) s2.data(), s2.size()); // load array
    in2.close();
    
    auto pow10_nums = break_down_to_pow10(uint64_t(pow(2, block_width)));
    vector<EcPoint> pow10_points;
    EcInt pow_key;
    for (auto n : pow10_nums) {
        pow_key.Set(n);
        pow10_points.push_back(ec.MultiplyG(pow_key));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto addition_search = [&]() {
        uint64_t mult = 2;
        int save_counter = 0;
        string temp, cpub;
        EcPoint starting_point, stride_point, calc_point;
        EcInt stride_sum, stride;
        ifstream inFile("settings1.txt");
        getline(inFile, temp);
        starting_point.SetHexStr(trim(temp).data());
        getline(inFile, temp);
        stride_sum.SetHexStr(trim(temp).data());
        inFile.close();
        
        stride.Set(uint64_t(pow(2, block_width)));
        stride_point = ec.MultiplyG(stride);
        
        while (true) {
            cpub = ec.GetPublicKeyHex(starting_point);
            if (bf1.may_contain(cpub)) {
                print_time(); cout << "BloomFilter Hit " << bloomfile1 << " (Even Point) [Lower Range Half]" << endl;
                EcPoint P = starting_point;
                vector<uint64_t> privkey_num;
                int index = 0;
                string cpub1;
                for (auto p : pow10_points) {
                    int count = 0;
                    cpub1 = ec.GetPublicKeyHex(P);
                    while (bf1.may_contain(cpub1)) {
                        P = ec.SubtractPoints(P, p);
                        cpub1 = ec.GetPublicKeyHex(P);
                        count += 1;
                    }
                    privkey_num.push_back(pow10_nums[index] * (count - 1));
                    P = ec.AddPoints(P, p);
                    index += 1;
                }
                EcInt Int_steps, Int_temp, privkey;
                uint64_t steps = 0;
                for (auto i : privkey_num) { steps += i; }
                Int_steps.Set(steps);
                Int_temp.Assign(stride_sum);
                Int_temp.Sub(Int_steps);
                privkey.Assign(pre_calc_sum);
                privkey.Sub(Int_temp);
                privkey.Mul_u64(privkey, mult);
                calc_point = ec.MultiplyG(privkey);
                if (ec.GetPublicKeyHex(calc_point) == search_pub) {
                	char privKey[100];
                	privkey.GetHexStr(privKey);
                    print_time(); cout << "Privatekey: " << privKey << endl;
                    ofstream outFile;
                    outFile.open("found.txt", ios::app);
                    outFile << privKey << '\n';
                    outFile.close();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = end - start;
                    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
                    duration -= hours;
                    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
                    duration -= minutes;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
                    print_time(); cout << "Elapsed time: (" << hours.count() << ")hours (" << minutes.count() << ")minutes (" << seconds.count() << ")seconds\n";
                    exit(0);
                }
                print_time(); cout << "False Positive" << endl;
            }
            
            if (bf2.may_contain(cpub)) {
                print_time(); cout << "BloomFilter Hit " << bloomfile2 << " (Odd Point) [Lower Range Half]" << endl;
                EcPoint P = starting_point;
                vector<uint64_t> privkey_num;
                int index = 0;
                string cpub2;
                for (auto p : pow10_points) {
                    int count = 0;
                    cpub2 = ec.GetPublicKeyHex(P);
                    while (bf2.may_contain(cpub2)) {
                        P = ec.SubtractPoints(P, p);
                        cpub2 = ec.GetPublicKeyHex(P);
                        count += 1;
                    }
                    privkey_num.push_back(pow10_nums[index] * (count - 1));
                    P = ec.AddPoints(P, p);
                    index += 1;
                }
                EcInt Int_steps, Int_temp, privkey, one;
                one.Set(1);
                uint64_t steps = 0;
                for (auto i : privkey_num) { steps += i; }
                Int_steps.Set(steps);
                Int_temp.Assign(stride_sum);
                Int_temp.Sub(Int_steps);
                privkey.Assign(pre_calc_sum);
                privkey.Sub(Int_temp);
                privkey.Mul_u64(privkey, mult);
                privkey.Add(one);
                calc_point = ec.MultiplyG(privkey);
                if (ec.GetPublicKeyHex(calc_point) == search_pub) {
                    char privKey[100];
                	privkey.GetHexStr(privKey);
                    print_time(); cout << "Privatekey: " << privKey << endl;
                    ofstream outFile;
                    outFile.open("found.txt", ios::app);
                    outFile << privKey << '\n';
                    outFile.close();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = end - start;
                    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
                    duration -= hours;
                    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
                    duration -= minutes;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
                    print_time(); cout << "Elapsed time: (" << hours.count() << ")hours (" << minutes.count() << ")minutes (" << seconds.count() << ")seconds\n";
                    exit(0);
                }
                print_time(); cout << "False Positive" << endl; 
            }
            starting_point = ec.AddPoints(starting_point, stride_point);
            stride_sum.Add(stride);
            save_counter += 1;
            if (save_counter % 50000000 == 0) {
                cpub = ec.GetPublicKeyHex(starting_point);
                char strideSum[100];
                stride_sum.GetHexStr(strideSum);
                ofstream outFile;
                outFile.open("settings1.txt");
                outFile << cpub <<'\n';
                outFile << strideSum << '\n';
                outFile.close();
                save_counter = 0;
                print_time(); cout << "Save Data written to settings1.txt" << endl;
            }
        }
    };
    
    auto subtraction_search = [&]() {
        uint64_t mult = 2;
        int save_counter = 0;
        string temp, cpub;
        EcPoint starting_point,stride_point, calc_point;
        EcInt stride_sum, stride;
        ifstream inFile("settings2.txt");
        getline(inFile, temp);
        starting_point.SetHexStr(trim(temp).data());
        getline(inFile, temp);
        stride_sum.SetHexStr(trim(temp).data());
        inFile.close();
        
        stride.Set(uint64_t(pow(2, block_width)));
        stride_point = ec.MultiplyG(stride);
        
        while (true) {
            cpub = ec.GetPublicKeyHex(starting_point);
            if (bf1.may_contain(cpub)) {
                print_time(); cout << "BloomFilter Hit " << bloomfile1 << " (Even Point) [Higher Range Half]" << endl;
                EcPoint P = starting_point;
                vector<uint64_t> privkey_num;
                int index = 0;
                string cpub1;
                for (auto p : pow10_points) {
                    int count = 0;
                    cpub1 = ec.GetPublicKeyHex(P);
                    while (bf1.may_contain(cpub1)) {
                        P = ec.SubtractPoints(P, p);
                        cpub1 = ec.GetPublicKeyHex(P);
                        count += 1;
                    }
                    privkey_num.push_back(pow10_nums[index] * (count - 1));
                    P = ec.AddPoints(P, p);
                    index += 1;
                }
                EcInt Int_steps, Int_temp, privkey;
                uint64_t steps = 0;
                for (auto i : privkey_num) { steps += i; }
                Int_steps.Set(steps);
                Int_temp.Assign(stride_sum);
                Int_temp.Add(Int_steps);
                privkey.Assign(pre_calc_sum);
                privkey.Add(Int_temp);
                privkey.Mul_u64(privkey, mult);
                calc_point = ec.MultiplyG(privkey);
                if (ec.GetPublicKeyHex(calc_point) == search_pub) {
                    char privKey[100];
                	privkey.GetHexStr(privKey);
                    print_time(); cout << "Privatekey: " << privKey << endl;
                    ofstream outFile;
                    outFile.open("found.txt", ios::app);
                    outFile << privKey << '\n';
                    outFile.close();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = end - start;
                    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
                    duration -= hours;
                    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
                    duration -= minutes;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
                    print_time(); cout << "Elapsed time: (" << hours.count() << ")hours (" << minutes.count() << ")minutes (" << seconds.count() << ")seconds\n";
                    exit(0);
                }
                print_time(); cout << "False Positive" << endl;
            }
            
            if (bf2.may_contain(cpub)) {
                print_time(); cout << "BloomFilter Hit " << bloomfile2 << " (Odd Point) [Higher Range Half]" << endl;
                EcPoint P = starting_point;
                vector<uint64_t> privkey_num;
                int index = 0;
                string cpub2;
                for (auto p : pow10_points) {
                    int count = 0;
                    cpub2 = ec.GetPublicKeyHex(P);
                    while (bf2.may_contain(cpub2)) {
                        P = ec.SubtractPoints(P, p);
                        cpub2 = ec.GetPublicKeyHex(P);
                        count += 1;
                    }
                    privkey_num.push_back(pow10_nums[index] * (count - 1));
                    P = ec.AddPoints(P, p);
                    index += 1;
                }
                EcInt Int_steps, Int_temp, privkey, one;
                one.Set(1);
                uint64_t steps = 0;
                for (auto i : privkey_num) { steps += i; }
                Int_steps.Set(steps);
                Int_temp.Assign(stride_sum);
                Int_temp.Add(Int_steps);
                privkey.Assign(pre_calc_sum);
                privkey.Add(Int_temp);
                privkey.Mul_u64(privkey, mult);
                privkey.Add(one);
                calc_point = ec.MultiplyG(privkey);
                if (ec.GetPublicKeyHex(calc_point) == search_pub) {
                    char privKey[100];
                	privkey.GetHexStr(privKey);
                    print_time(); cout << "Privatekey: " << privKey << endl;
                    ofstream outFile;
                    outFile.open("found.txt", ios::app);
                    outFile << privKey << '\n';
                    outFile.close();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = end - start;
                    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
                    duration -= hours;
                    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
                    duration -= minutes;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
                    print_time(); cout << "Elapsed time: (" << hours.count() << ")hours (" << minutes.count() << ")minutes (" << seconds.count() << ")seconds\n";
                    exit(0);
                }
                print_time(); cout << "False Positive" << endl;
            }
            starting_point = ec.SubtractPoints(starting_point, stride_point);
            stride_sum.Add(stride);
            save_counter += 1;
            if (save_counter % 50000000 == 0) {
                cpub = ec.GetPublicKeyHex(starting_point);
                char strideSum[100];
                stride_sum.GetHexStr(strideSum);
                ofstream outFile;
                outFile.open("settings2.txt");
                outFile << cpub <<'\n';
                outFile << strideSum << '\n';
                outFile.close();
                save_counter = 0;
                print_time(); cout << "Save Data written to settings2.txt" << endl;
            }
        }
    };
    
    print_time(); cout << "Search in progress..." << endl;
    
    std::thread thread1(addition_search);
    std::thread thread2(subtraction_search);
    
    thread1.join();
    thread2.join();
	
}