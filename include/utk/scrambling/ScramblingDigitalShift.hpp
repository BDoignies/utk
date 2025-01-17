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

#include <utk/utils/FastPRNG.hpp>
#include <utk/utils/Pointset.hpp>
#include <utk/utils/FastPRNG.hpp>
#include <utk/utils/RadicalInversion.hpp>
#include <random>

namespace utk
{
    template <typename IntegerType = uint32_t>
    class ScramblingDigitalShift
    {
    public:
        ScramblingDigitalShift()
        { setRandomSeed(); }

        void setRandomSeed(uint64_t arg_seed) 
        { 
            if (arg_seed == 0) setRandomSeed();
            else mt.seed(arg_seed);
        }

        void setRandomSeed() 
        { 
            setRandomSeed(std::random_device{}());
        }

        template<typename T>
        bool Scramble(Pointset<T>& in)
        {
            static_assert(std::is_same_v<T, IntegerType>());
            std::vector<IntegerType> shifts(in.Ndim());
            for (uint32_t d = 0; d < in.Ndim(); d++)
                shifts[d] = mt() % std::numeric_limits<IntegerType>::max();
            
            for (uint32_t i = 0; i < in.Npts(); i++)
            {
                for (uint32_t d = 0; d < in.Ndim(); d++)
                {
                    in[i][d] = RadicalInverseBase2(in[i][d]) ^ shifts[d];
                }
            }
            return true;
        }

        template<typename T, typename D>
        bool Scramble(const Pointset<T>& in, Pointset<D>& out)
        {
            out.Resize(in.Npts(), in.Ndim());

            std::vector<IntegerType> shifts(in.Ndim());
            for (uint32_t d = 0; d < in.Ndim(); d++)
                shifts[d] = mt() % std::numeric_limits<IntegerType>::max();
            
            for (uint32_t i = 0; i < in.Npts(); i++)
            {
                for (uint32_t d = 0; d < in.Ndim(); d++)
                {
                    out[i][d] = convert<D>(RadicalInverseBase2(in[i][d]) ^ shifts[d]);
                }
            }
            return true;
        }

        template<typename T>
        bool Scramble(std::vector<Pointset<T>>& in)
        {
            bool result = true;
            for (auto& i : in)
                result = result && Scramble(i);
            return result;
        }


        template<typename T, typename D>
        bool Scramble(const std::vector<Pointset<T>>& in, std::vector<Pointset<D>>& out)
        {
            bool result = true;
            out.resize(in.size());

            for (std::size_t i = 0; i < in.size(); i++)
            {
                out[i].Resize(in[i].Npts(), in[i].Ndim());
                result = result && Scramble(in[i], out[i]);
            }
            return result;
        }
        
    private:
        template<typename T>
        static T convert(IntegerType x)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return static_cast<T>(x);
            }
            else
            {
                T r = 0.0;
                T b = 0.5;
                while (x)
                {
                    r += (x & 1) * b;

                    b *= 0.5;
                    x >>= 1;
                }

                return r;
            }
        }
    private:
        utk::PCG32 mt;
    };
}