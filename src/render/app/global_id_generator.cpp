//
// Created by RGAA on 29/09/2025.
//

#include "global_id_generator.h"

uint64_t GenNextGlobalId() {
    return tc::GlobalIdGenerator::Instance()->NextId();
}