/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de,
 *http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2017, SeisSol Group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/

#include "easitest.h"

void model(double y, double params[3]) {
    params[0] = 2670.0;
    if (y <= -800.0) {
        params[1] = 2.816717568E+10;
        params[2] = 2.817615756E+10;
    } else if (y >= -800.0 && y <= 800.0) {
        params[1] = 1.251489075E+10;
        params[2] = 1.251709350E+10;
    } else {
        params[1] = 3.203812032E+10;
        params[2] = 3.204375936E+10;
    }
}

void assertEqual(double y, ElasticMaterial const& material) {
    double params[3];
    model(y, params);

    assert(equal(material.rho, params[0]));
    assert(equal(material.mu, params[1]));
    assert(equal(material.lambda, params[2]));
}

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({{1, {0.0, -1000.0, 0.0}},
                                        {1, {0.0, -800.0, 0.0}},
                                        {1, {0.0, 0.0, 0.0}},
                                        {1, {0.0, 800.0, 0.0}},
                                        {1, {0.0, 1000.0, 0.0}}});

    auto material = elasticModel(argv[1], query);

    assertEqual(-1000.0, material[0]);
    assertEqual(-800.0, material[1]);
    assertEqual(0.0, material[2]);
    assertEqual(800.0, material[3]);
    assertEqual(1000.0, material[4]);

    return 0;
}
