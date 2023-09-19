/*
 * Coded by Hélène Perrier helene.perrier@liris.cnrs.fr
 * and Bastien Doignies bastien.doignies@liris.cnrs.fr 
 * and David Coeurjolly David.coeurjolly@liris.cnrs.fr
 *
 * Copyright (c) 2023 CNRS Université de Lyon
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the UTK project.
 */
#pragma once

#include <utk/utils/Pointset.hpp>
#include <utk/utils/Omp.hpp>
#include <cmath>

namespace utk
{
    class UnanchoredL2Discrepancy
    {
    public:
        UnanchoredL2Discrepancy() {}

        template<typename T>
        T compute(const Pointset<T>& pts)
        {
            const auto N = pts.Npts();
            const auto D = pts.Ndim();

            T sumprod1    = 0.0;
            T sumsumprod2 = 0.0;
            T sumsumprod3 = 0.0;
            
            #pragma omp parallel for reduction(+:sumprod1) reduction(+:sumsumprod2) reduction(+:sumsumprod3)
            for (OPENMP_UINT i = 0; i < N; i++)
            {
                T prod1 = 1.0;
                T prod2 = 1.0;
                
                for (uint32_t d = 0; d < D; d++)
                {
                    const T tmp = pts[i][d] * (1 - pts[i][d]); 
                    prod1 *= tmp;
                    prod2 *= tmp;
                }

                sumprod1    += prod1;
                sumsumprod2 += prod2;

                // Use the fact that max is a symetric function
                for (uint32_t j = i + 1; j < N; j++)
                {
                    T prod3 = 1.0;
                    for (uint32_t d = 0; d < D; d++)
                        prod3 *= (1. - std::max(pts[i][d], pts[j][d])) * std::min(pts[i][d], pts[j][d]);
                    sumsumprod3 += prod3;
                }
            }

            const T invN  = 1. / (T) N;
            const T invN2 = invN * invN;

            T value = std::pow(1. / 12., D) - std::pow(2.0, ((T)1 - (T)D)) * invN * sumprod1 + invN2 * (sumsumprod2 + 2. * sumsumprod3);
            return std::sqrt(value);
        }

        template<typename T>
        std::vector<T> compute(const std::vector<Pointset<T>>& ptss)
        {
            std::vector<T> rslts;
            rslts.reserve(ptss.size());
            for (const auto& pts : ptss)
                rslts.push_back(compute(pts));
            return rslts;
        }
    private:
    };
};