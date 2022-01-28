//
// Created by consti10 on 19.03.21.
//

#ifndef WIFIBROADCAST_FEC_H
#define WIFIBROADCAST_FEC_H

#include "HelperSources/Helper.hpp"
#include "ExternalCSources/fec/fec.h"
#include <vector>
#include <array>

// c++ wrapper around fec library
// NOTE: When working with FEC, people seem to use the terms block, fragments and more in different context(s).
// To avoid confusion,I decided to use the following notation:
// A block is formed by K primary and N-K secondary fragments. Each of these fragments must have the same size.
// Therefore,
// fragmentSize is the size of each fragment in this block (for some reason, this is called blockSize in the underlying c fec implementation).
// A primary fragment is a data packet
// A secondary fragment is a data correction (FEC) packet
// Note: for the fec_decode() step, it doesn't matter how many secondary fragments were created during the fec_encode() step -
// only thing that matters is how many secondary fragments you received (either enough for fec_decode() or not enough for fec_decode() )
// Also note: you obviously cannot use the same secondary fragment more than once

/**
 * @param fragmentSize size of each fragment in this block
 * @param primaryFragments list of pointers to memory for primary fragments
 * @param secondaryFragments list of pointers to memory for secondary fragments (fec fragments)
 * Using the data from @param primaryFragments constructs as many secondary fragments as @param secondaryFragments holds
 */
void fec_encode(unsigned int fragmentSize,
                const std::vector<uint8_t*>& primaryFragments,
                const std::vector<uint8_t*>& secondaryFragments){
    fec_encode(fragmentSize, (const unsigned char**)primaryFragments.data(), primaryFragments.size(), (unsigned char**)secondaryFragments.data(), secondaryFragments.size());
}

/**
 * @param fragmentSize size of each fragment in this block
 * @param primaryFragments list of pointers to memory for primary fragments. Must be same size as used for fec_encode()
 * @param indicesMissingPrimaryFragments list of the indices of missing primary fragments.
 * Example: if @param indicesMissingPrimaryFragments contains 2, the 3rd primary fragment is missing
 * @param secondaryFragmentsReceived list of pointers to memory for secondary fragments (fec fragments). Must not be same size as used for fec_encode(), only MUST contain "enough" secondary fragments
 * @param indicesOfSecondaryFragmentsReceived list of the indices of secondaryFragments that are used to reconstruct missing primary fragments.
 * Example: if @param indicesOfSecondaryFragmentsReceived contains {0,2}, the first secondary fragment has the index 0, and the second secondary fragment has the index 2
 * When this call returns, all missing primary fragments (gaps) have been filled / reconstructed
 */
void fec_decode(unsigned int fragmentSize,
                const std::vector<uint8_t*>& primaryFragments,
                const std::vector<unsigned int>& indicesMissingPrimaryFragments,
                const std::vector<uint8_t*>& secondaryFragmentsReceived,
                const std::vector<unsigned int>& indicesOfSecondaryFragmentsReceived){
    //std::cout<<"primaryFragmentsS:"<<primaryFragments.size()<<"\n";
    //std::cout<<"indicesMissingPrimaryFragments:"<<StringHelper::vectorAsString(indicesMissingPrimaryFragments)<<"\n";
    //std::cout<<"secondaryFragmentsS:"<<secondaryFragments.size()<<"\n";
    //std::cout << "indicesOfSecondaryFragments:" << StringHelper::vectorAsString(indicesOfSecondaryFragments) << "\n";
    for(const auto& idx:indicesMissingPrimaryFragments){
        assert(idx<primaryFragments.size());
    }
    assert(indicesMissingPrimaryFragments.size() <= indicesOfSecondaryFragmentsReceived.size());
    assert(indicesMissingPrimaryFragments.size() == secondaryFragmentsReceived.size());
    assert(secondaryFragmentsReceived.size() == indicesOfSecondaryFragmentsReceived.size());
    fec_decode(fragmentSize, (unsigned char**)primaryFragments.data(), primaryFragments.size(), (unsigned char**)secondaryFragmentsReceived.data(),
               (unsigned int*)indicesOfSecondaryFragmentsReceived.data(), (unsigned int*)indicesMissingPrimaryFragments.data(), indicesMissingPrimaryFragments.size());

}


//Note: By using "blockBuffer" as input the fecEncode / fecDecode function(s) don't need to allocate any new memory.
// Also note, indices in blockBuffer can refer to either primary or secondary fragments. Whereas when calling
// fec_decode(), secondary fragment numbers start from 0, not from nPrimaryFragments.
// These declarations are written such that you can do "variable block size" on tx and rx.

/**
 * @param fragmentSize size of each fragment to use for the FEC encoding step. FEC only works on packets the same size
 * @param blockBuffer (big) data buffer. The nth element is to be treated as the nth fragment of the block, either as primary or secondary fragment.
 * During the FEC step, @param nPrimaryFragments fragments are used to calculate nSecondaryFragments FEC blocks.
 * After the FEC step,beginning at position @param nPrimaryFragments ,@param nSecondaryFragments are stored at the following positions, each of size @param fragmentSize
 */
template<std::size_t S>
void fecEncode(unsigned int fragmentSize, std::vector<std::array<uint8_t,S>>& blockBuffer, unsigned int nPrimaryFragments, unsigned int nSecondaryFragments){
    assert(fragmentSize <= S);
    assert(nPrimaryFragments+nSecondaryFragments<=blockBuffer.size());
    auto primaryFragmentsP= GenericHelper::convertToP(blockBuffer,0,nPrimaryFragments);
    auto secondaryFragmentsP=GenericHelper::convertToP(blockBuffer,nPrimaryFragments,blockBuffer.size()-nPrimaryFragments);
    secondaryFragmentsP.resize(nSecondaryFragments);
    //const auto before=std::chrono::steady_clock::now();
    fec_encode(fragmentSize, primaryFragmentsP, secondaryFragmentsP);
    //const auto delta=std::chrono::steady_clock::now()-before;
    //std::cout<<"fec_encode step took:"<<std::chrono::duration_cast<std::chrono::microseconds>(delta).count()<<"us\n";
}

enum FragmentStatus{UNAVAILABLE=0,AVAILABLE=1};

/**
 * @param fragmentSize size of each fragment
 * @param blockBuffer blockBuffer (big) data buffer. The nth element is to be treated as the nth fragment of the block, either as primary or secondary fragment.
 * @param nPrimaryFragments n of primary fragments used during encode step
 * @param fragmentStatusList information which (primary or secondary fragments) were received.
 * values from [0,nPrimaryFragments[ are treated as primary fragments, values from [nPrimaryFragments,size[ are treated as secondary fragments.
 * @return indices of reconstructed primary fragments
 */
template<std::size_t S>
std::vector<unsigned int> fecDecode(unsigned int fragmentSize, std::vector<std::array<uint8_t,S>>& blockBuffer, const unsigned int nPrimaryFragments, const std::vector<FragmentStatus>& fragmentStatusList){
    assert(fragmentSize <= S);
    assert(fragmentStatusList.size() <= blockBuffer.size());
    assert(fragmentStatusList.size()==blockBuffer.size());
    std::vector<unsigned int> indicesMissingPrimaryFragments;
    std::vector<uint8_t*> primaryFragmentP(nPrimaryFragments);
    for(unsigned int idx=0;idx<nPrimaryFragments;idx++){
        if(fragmentStatusList[idx] == UNAVAILABLE){
            indicesMissingPrimaryFragments.push_back(idx);
        }
        primaryFragmentP[idx]=blockBuffer[idx].data();
    }
    // find enough secondary fragments
    std::vector<uint8_t*> secondaryFragmentP;
    std::vector<unsigned int> secondaryFragmentIndices;
    for(int i=0; i < fragmentStatusList.size() - nPrimaryFragments; i++) {
        const auto idx = nPrimaryFragments + i;
        if(fragmentStatusList[idx] == AVAILABLE){
            secondaryFragmentP.push_back(blockBuffer[idx].data());
            secondaryFragmentIndices.push_back(i);
        }
    }
    // make sure we got enough secondary fragments
    assert(secondaryFragmentP.size()>=indicesMissingPrimaryFragments.size());
    // assert if fecDecode is called too late (e.g. more secondary fragments than needed for fec
    assert(indicesMissingPrimaryFragments.size()==secondaryFragmentP.size());
    // do fec step
    fec_decode(fragmentSize,primaryFragmentP,indicesMissingPrimaryFragments,secondaryFragmentP,secondaryFragmentIndices);
    return indicesMissingPrimaryFragments;
}

// randomly select a possible combination of received indices (either primary or secondary).
static void testFecCPlusPlusWrapperY(const int nPrimaryFragments,const int nSecondaryFragments){
    fec_init();
    srand (time(NULL));
    constexpr auto FRAGMENT_SIZE=1446;

    auto txBlockBuffer=GenericHelper::createRandomDataBuffers<FRAGMENT_SIZE>(nPrimaryFragments + nSecondaryFragments);
    std::cout<<"XSelected nPrimaryFragments:"<<nPrimaryFragments<<" nSecondaryFragments:"<<nSecondaryFragments<<"\n";

    fecEncode(FRAGMENT_SIZE, txBlockBuffer, nPrimaryFragments, nSecondaryFragments);
    std::cout<<"Encode done\n";

    for(int test=0;test<100;test++) {
        // takes nPrimaryFragments random (possible) indices without duplicates
        // NOTE: Perhaps you could calculate all possible permutations, but these would be quite a lot.
        // Therefore, I just use n random selections of received indices
        auto receivedFragmentIndices= GenericHelper::takeNRandomElements(
                GenericHelper::createIndices(nPrimaryFragments + nSecondaryFragments),
                nPrimaryFragments);
        assert(receivedFragmentIndices.size()==nPrimaryFragments);
        std::cout<<"(Emulated) receivedFragmentIndices"<<StringHelper::vectorAsString(receivedFragmentIndices)<<"\n";

        auto rxBlockBuffer=std::vector<std::array<uint8_t,FRAGMENT_SIZE>>(nPrimaryFragments+nSecondaryFragments);
        std::vector<FragmentStatus> fragmentMap(nPrimaryFragments+nSecondaryFragments,FragmentStatus::UNAVAILABLE);
        for(const auto idx:receivedFragmentIndices){
            rxBlockBuffer[idx]=txBlockBuffer[idx];
            fragmentMap[idx]=FragmentStatus::AVAILABLE;
        }

        fecDecode(FRAGMENT_SIZE, rxBlockBuffer, nPrimaryFragments, fragmentMap);

        for(unsigned int i=0;i<nPrimaryFragments;i++){
            std::cout<<"Comparing fragment:"<<i<<"\n";
            GenericHelper::assertArraysEqual(txBlockBuffer[i], rxBlockBuffer[i]);
        }
    }
}

// Note: This test will take quite a long time ! (or rather ages :) when trying to do all possible combinations. )
void testFecCPlusPlusWrapperX(){
    std::cout<<"testFecCPlusPlusWrapper Begin\n";
    //constexpr auto MAX_N_P_F=128;
    //constexpr auto MAX_N_S_F=128;
    // else it really takes ages
    constexpr auto MAX_N_P_F=32;
    constexpr auto MAX_N_S_F=32;
    for(int nPrimaryFragments=1;nPrimaryFragments<MAX_N_P_F;nPrimaryFragments++){
        for(int nSecondaryFragments=0;nSecondaryFragments<MAX_N_S_F;nSecondaryFragments++){
            testFecCPlusPlusWrapperY(nPrimaryFragments,nSecondaryFragments);
        }
    }
    std::cout<<"testFecCPlusPlusWrapper End\n";
}


#endif //WIFIBROADCAST_FEC_H
