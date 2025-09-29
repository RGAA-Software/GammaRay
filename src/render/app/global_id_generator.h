//
// Created by RGAA on 29/09/2025.
//

#ifndef GAMMARAYPREMIUM_GLOBAL_ID_GENERATOR_H
#define GAMMARAYPREMIUM_GLOBAL_ID_GENERATOR_H

#include <atomic>

namespace tc
{

    class GlobalIdGenerator {
    public:
        static GlobalIdGenerator* Instance() {
            static GlobalIdGenerator generator;
            return &generator;
        }

        uint64_t NextId() {
            return ++counter_;
        }

    private:
        GlobalIdGenerator() : counter_(0) {}
        std::atomic<uint64_t> counter_;

    };

}

extern "C" {
    __declspec(dllexport) uint64_t GenNextGlobalId();
}

#endif //GAMMARAYPREMIUM_GLOBAL_ID_GENERATOR_H
