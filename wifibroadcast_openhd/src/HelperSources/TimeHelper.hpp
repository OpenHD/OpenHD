//
// Created by geier on 18/01/2020.
//

#ifndef LIVEVIDEO10MS_TIMEHELPER_HPP
#define LIVEVIDEO10MS_TIMEHELPER_HPP

#include <chrono>
#include <deque>
#include <algorithm>
#include <sstream>
#include <iostream>
#include "StringHelper.hpp"

// This file holds various classes/namespaces usefully for measuring and comparing
// latency samples

namespace MyTimeHelper{
    // R stands for readable. Convert a std::chrono::duration into a readable format
    // Readable format is somewhat arbitrary, in this case readable means that for example
    // 1second has 'ms' resolution since for values that big ns resolution probably isn't needed
    static std::string R(const std::chrono::steady_clock::duration& dur){
        const auto durAbsolute=std::chrono::abs(dur);
        if(durAbsolute>=std::chrono::seconds(1)){
            // More than one second, print as decimal with ms resolution.
            const auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            return std::to_string(ms/1000.0f)+"s";
        }
        if(durAbsolute>=std::chrono::milliseconds(1)){
            // More than one millisecond, print as decimal with us resolution
            const auto us=std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
            return std::to_string(us/1000.0f)+"ms";
        }
        if(durAbsolute>=std::chrono::microseconds(1)){
            // More than one microsecond, print as decimal with ns resolution
            const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
            return std::to_string(ns/1000.0f)+"us";
        }
        const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
        return std::to_string(ns)+"ns";
    }
    static std::string ReadableNS(uint64_t nanoseconds){
        return R(std::chrono::nanoseconds(nanoseconds));
    }
	static std::string timeSamplesAsString(const std::vector<std::chrono::nanoseconds>& samples){
		std::stringstream ss;
		int counter=0;
        for(const auto& sample:samples){
           ss<<","<<MyTimeHelper::R(sample);
		   counter++;
		   if(counter%10==0 && counter!=samples.size()){
		     ss<<"\n";
		   }
        }
		return ss.str();
	}
};

// Use this class to compare many samples of the same kind
// Saves the minimum,maximum and average of all the samples
// The type of the samples is for example std::chrono::nanoseconds when measuring time intervalls
template<typename T>
class BaseAvgCalculator{
private:
    // do not forget the braces to initialize with 0
    T sum{};
    long nSamples=0;
    T min=std::numeric_limits<T>::max();
    T max{};
public:
    BaseAvgCalculator(){reset();};
    void add(const T& value){
        if(value<T(0)){
            std::cout<<"Cannot add negative value\n";
            return;
        }
        sum+=value;
        nSamples++;
        if(value<min){
            min=value;
        }
        if(value>max){
            max=value;
        }
    }
    // Returns the average of all samples.
    // If 0 samples were recorded, return 0
    T getAvg()const{
        if(nSamples == 0)return T(0);
        return sum / nSamples;
    }
    // Returns the minimum value of all samples
    T getMin()const{
        return min;
    }
    // Returns the maximum value of all samples
    T getMax()const{
        return max;
    }
    // Returns the n of samples that were processed
    long getNSamples()const{
        return nSamples;
    }
    // Reset everything (as if zero samples were processed)
    void reset(){
        sum={};
        nSamples=0;
        // Workaround for std::numeric_limits returning 0 for std::chrono::nanoseconds
        if constexpr (std::is_same_v<T,std::chrono::nanoseconds>){
            min=std::chrono::nanoseconds::max();
        }else{
            min=std::numeric_limits<T>::max();
        }
        max={};
    }
    // Merges two AvgCalculator(s) that hold the same types of samples together
    BaseAvgCalculator<T> operator+(const BaseAvgCalculator<T>& other){
        BaseAvgCalculator<T> ret;
        ret.add(this->getAvg());
        ret.add(other.getAvg());
        const auto min1=std::min(this->getMin(),other.getMin());
        const auto max1=std::max(this->getMax(),other.getMax());
        ret.min=min1;
        ret.max=max1;
        return ret;
    }
    // max delta between average and min / max
    std::chrono::nanoseconds getMaxDifferenceMinMaxAvg()const{
        const auto deltaMin=std::chrono::abs(getAvg()-getMin());
        const auto deltaMax=std::chrono::abs(getAvg()-getMax());
        if(deltaMin>deltaMax)return deltaMin;
        return deltaMax;
    }
    std::string getAvgReadable(const bool averageOnly=false)const{
        std::stringstream ss;
        if constexpr (std::is_same_v<T,std::chrono::nanoseconds>){
            // Class stores time samples
            if(averageOnly){
                ss<<"avg="<<MyTimeHelper::R(getAvg());
                return ss.str();
            }
            ss<<"min="<<MyTimeHelper::R(getMin())<<" max="<<MyTimeHelper::R(getMax())<<" avg="<<MyTimeHelper::R(getAvg());
        }else{
            // Class stores other type of samples
            if(averageOnly){
                ss<<"avg="<<getAvg();
                return ss.str();
            }
            ss<<"min="<<getMin()<<" max="<<getMax()<<" avg="<<getAvg();
        }
        return ss.str();
    }
    float getAvg_ms(){
        return (float)(std::chrono::duration_cast<std::chrono::microseconds>(getAvg()).count())/1000.0f;
    }
};
// Default is using timestamps
using AvgCalculator=BaseAvgCalculator<std::chrono::nanoseconds>;
using AvgCalculatorSize=BaseAvgCalculator<std::size_t>;


// Instead of storing only the min, max and average this stores
// The last n samples in a queue. However, this makes calculating the min/max/avg values much more expensive
// And therefore should only be used with a small sample size.
class AvgCalculator2{
private:
    const size_t sampleSize;
    std::deque<std::chrono::nanoseconds> samples;
public:
    // Use zero for infinite n of recorded samples
	// Be carefully with memory in this case
    explicit AvgCalculator2(size_t sampleSize=60):sampleSize(sampleSize){};
    //
    void add(const std::chrono::nanoseconds& value){
        if(value<std::chrono::nanoseconds(0)){
            std::cout<<"Cannot add negative value\n";
            return;
        }
        samples.push_back(value);
        // Remove the oldest sample if needed
        if(sampleSize!=0 &&samples.size()>sampleSize){
            samples.pop_front();
        }
    }
    std::chrono::nanoseconds getAvg()const{
        if(samples.empty()){
            return std::chrono::nanoseconds(0);
        }
        std::chrono::nanoseconds sum{0};
        for(const auto sample:samples){
            sum+=sample;
        }
        return sum / samples.size();
    }
    std::chrono::nanoseconds getMin()const{
        return *std::min_element(samples.begin(),samples.end());
    }
    std::chrono::nanoseconds getMax()const{
        return *std::max_element(samples.begin(),samples.end());
    }
    void reset(){
        samples.resize(0);
    }
	size_t getNSamples()const{
        return samples.size();
    }
    std::string getAvgReadable(const bool averageOnly=false)const{
        std::stringstream ss;
        if(averageOnly){
            ss<<"avg="<<MyTimeHelper::R(getAvg());
            return ss.str();
        }
        ss<<"min="<<MyTimeHelper::R(getMin())<<" max="<<MyTimeHelper::R(getMax())<<" avg="<<MyTimeHelper::R(getAvg())<<" N samples="<<samples.size();
        return ss.str();
    }
    std::string getAllSamplesAsString()const{
         std::stringstream ss;
         for(const auto& sample:samples){
            ss<<" "<<MyTimeHelper::R(sample);
         }
         return ss.str();
    }
    // Sort all the samples from low to high
    std::vector<std::chrono::nanoseconds> getSamplesSorted()const{
        auto ret=std::vector<std::chrono::nanoseconds>(samples.begin(),samples.end());
        std::sort(ret.begin(), ret.end());
        return ret;
    }
    std::string getAllSamplesSortedAsString()const{
        const auto valuesSorted=getSamplesSorted();
        return MyTimeHelper::timeSamplesAsString(valuesSorted);
    }
	// Returns up to count lowest and highest samples
	std::string getNValuesLowHigh(int n=10)const{
		auto valuesSorted=getSamplesSorted();
		if(n>valuesSorted.size()/2){
			n=valuesSorted.size()/2;
		}
        const auto xPercentLow=std::vector<std::chrono::nanoseconds>(valuesSorted.begin(),valuesSorted.begin()+n);
        const auto tmpBegin=valuesSorted.begin()+valuesSorted.size()-n;
        const auto xPercentHigh=std::vector<std::chrono::nanoseconds>(tmpBegin,valuesSorted.end());
        std::stringstream ss;
        ss<<n<<" lowest values:\n";
        ss<<MyTimeHelper::timeSamplesAsString(xPercentLow);
        ss<<"\n"<<n<<" highest values:\n";
        ss<<MyTimeHelper::timeSamplesAsString(xPercentHigh);
		ss<<"\n";
        return ss.str();	
	}
	// returns the one percent low / high values
    std::string getOnePercentLowHigh()const{
        auto valuesSorted=getSamplesSorted();
        const auto sizeOnePercent=valuesSorted.size()/100;
        const auto onePercentLow=std::vector<std::chrono::nanoseconds>(valuesSorted.begin(),valuesSorted.begin()+sizeOnePercent);
        const auto tmpBegin=valuesSorted.begin()+valuesSorted.size()-sizeOnePercent;
        const auto onePercentHigh=std::vector<std::chrono::nanoseconds>(tmpBegin,valuesSorted.end());
        std::stringstream ss;
        ss<<"One Percent low:\n";
        ss<<MyTimeHelper::timeSamplesAsString(onePercentLow);
        ss<<"\nOne Percent high:\n";
        ss<<MyTimeHelper::timeSamplesAsString(onePercentHigh);
		ss<<"\n";
        return ss.str();
    }
};


class Chronometer:public AvgCalculator {
public:
    explicit Chronometer(std::string name="Unknown"):mName(std::move(name)){}
    void start(){
        startTS=std::chrono::steady_clock::now();
    }
    void stop(){
        const auto now=std::chrono::steady_clock::now();
        const auto delta=(now-startTS);
        AvgCalculator::add(delta);
    }
    void printInIntervalls(const std::chrono::steady_clock::duration& interval,const bool avgOnly=true) {
        const auto now=std::chrono::steady_clock::now();
        if(now-lastLog>interval){
            lastLog=now;
            std::cout<<(mName)<<"Avg: "<<AvgCalculator::getAvgReadable(avgOnly)<<"\n";
            reset();
        }
    }
private:
    const std::string mName;
    std::chrono::steady_clock::time_point startTS;
    std::chrono::steady_clock::time_point lastLog=std::chrono::steady_clock::now();
};

class RelativeCalculator{
private:
    long sum=0;
    long sumAtLastCall=0;
public:
    RelativeCalculator() = default;
    void add(unsigned long x){
        sum+=x;
    }
    long getDeltaSinceLastCall() {
        long ret = sum - sumAtLastCall;
        sumAtLastCall = sum;
        return ret;
    }
    long getAbsolute()const{
        return sum;
    }
    void reset(){
        sum=0;
        sumAtLastCall=0;
    }
};


#endif //LIVEVIDEO10MS_TIMEHELPER_HPP