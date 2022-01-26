//
// Created by consti10 on 31.12.21.
//

#ifndef WIFIBROADCAST_RANDOMBUFFERPOT_H
#define WIFIBROADCAST_RANDOMBUFFERPOT_H

#include <random>
#include <cassert>
#include <memory>
#include <string>
#include <algorithm>
#include "Helper.hpp"

namespace SemiRandomBuffers{
    /**
     * Create @param nBuffers buffers of size @param bufferSize filled with semi-random data.
     * Each of the generated buffers contains different data than the previous generated one (by a almost 100% chance)
     */
    static std::vector<std::shared_ptr<std::vector<uint8_t>>> createSemiRandomBuffers(const std::size_t nBuffers,const std::size_t bufferSize){
        // NOTE for some reason the following code does not what I'd expect:
        //std::vector<std::shared_ptr<std::vector<uint8_t>>> ret(nBuffers,std::make_shared<std::vector<uint8_t>>(bufferSize));
        std::vector<std::shared_ptr<std::vector<uint8_t>>> ret(nBuffers);
        for(int i=0;i<ret.size();i++){
            ret[i]=std::make_shared<std::vector<uint8_t>>(bufferSize);
        }
        assert(ret.size()==nBuffers);
        // fill all buffers with semi random data
        std::mt19937 random_engine(0);
        std::uniform_int_distribution<> distrib(0, 256);

        for(auto& buffer:ret){
            assert(buffer->size()==bufferSize);
            // NOTE: I think this one doesn't work since the mt19937 is copied with each invocation ?!
            // std::generate(buffer->data(),buffer->data()+buffer->size(),random_engine);
            for(auto& value:*buffer.get()){
                value=distrib(random_engine);
            }
        }
        /*for(int i=0;i<ret.size();i++){
            for(int j=0;j<ret.size();j++){
                if(i!=j){
                    //std::cout<<"x"<<i<<","<<j<<"\n";
                    auto buf1=ret[i];
                    auto buf2=ret[j];
                    //std::cout<<StringHelper::vectorAsString(*buf1.get())<<"\n";
                    //std::cout<<StringHelper::vectorAsString(*buf2.get())<<"\n";
                    assert(GenericHelper::compareVectors(*buf1.get(),*buf2.get())==false);
                }
            }
        }*/
        /*for(int i=0;i<ret.size();i++){
            auto buf1=ret[i];
            std::cout<<StringHelper::vectorAsString(*buf1.get())<<"\n";
        }*/
        return ret;
    }
    // same as above, but different return type
    template<size_t S>
    static std::vector<std::array<uint8_t,S>> createSemiRandomBuffers2(const std::size_t nBuffers){
        std::vector<std::array<uint8_t,S>> ret(nBuffers);
        std::mt19937 random_engine(0);
        std::uniform_int_distribution<> distrib(0, 256);
        for(auto& buffer:ret){
            for(auto& value:buffer){
                value=distrib(random_engine);
            }
        }
        return ret;
    }
}

// holds x buffers with (semi-random) data.
class RandomBufferPot{
public:
    /**
     * Holds @param nBuffers random data buffers of size @param bufferSize
     */
    RandomBufferPot(const std::size_t nBuffers,const std::size_t bufferSize){
        m_buffers= SemiRandomBuffers::createSemiRandomBuffers(nBuffers,bufferSize);
    }
    // get a semi-random data buffer for this sequence number. If the sequence number is higher than the n of allocated buffers,
    // it loops around. As long as this pot is big enough, it should be sufficient to emulate a random data stream
    std::shared_ptr<std::vector<uint8_t>> getBuffer(uint64_t sequenceNumber){
        auto index=sequenceNumber % m_buffers.size();
        return m_buffers.at(index);
    }
private:
    std::vector<std::shared_ptr<std::vector<uint8_t>>> m_buffers;
    //static constexpr const uint32_t SEED=12345;
};

#endif //WIFIBROADCAST_RANDOMBUFFERPOT_H
