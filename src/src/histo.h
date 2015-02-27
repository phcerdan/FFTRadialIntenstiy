/*
Histogram. Basic struct to handle histograms.
Copyright (C) 2015 Pablo Hernandez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HISTO_H_
#define HISTO_H_
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <iostream>
namespace histo {
/**
 * @brief Breaks method to optimal calculation of breaks based on input data and range.
 * Scott = 0.
 */
enum breaks_method {Scott = 0};

/**
 * @brief Help functions to manually creating breaks from input range
 * (low, upper) and desired number of bins.
 *
 * @tparam T Input data type.
 * @tparam PRECI It should be equal to T, except when T is int.
 * @param low first value of breaks.
 * @param upper last value of breaks.
 * @param bins number of divisions.
 *
 * @return breaks vector with the frontier values of each bin.
 */
template<typename T, typename PRECI = double>
std::vector<T> GenerateBreaksFromRangeAndBins(const T& low, const T& upper, const unsigned long int &bins){
    std::vector<T> breaks(bins+1);
    T width = (upper - low)/ static_cast<PRECI>(bins);
    for (auto i = 0; i!=bins+1; i++){
        breaks[i] = low + i*width;
    }
    return breaks;
};

template<typename T, typename PRECI = double>
std::vector<T> GenerateBreaksFromRangeAndBins(const std::pair<T,T> &range_low_upper, const unsigned long int &bins){
   T low   = range_low_upper.first;
   T upper = range_low_upper.second;
   return GenerateBreaksFromRangeAndBins<T, PRECI>(low, upper, bins);
};

/**
 * @brief Variance calculation from Container with data.
 *
 * @tparam T Type of data.
 * @tparam Container std:: type containing data (vector, array,...)
 * @param xs The container.
 *
 * @return Variance of type T.
 */
template <typename T, typename Container>
T variance_welford(const Container& xs)
{
    size_t N = 0;
    T M = 0, S = 0, Mprev = 0;
    for(auto x : xs) {
        ++N;
        Mprev = M;
        M += (x - Mprev) / N;
        S += (x - Mprev) * (x - M);
    }
    return S / (N-1);
}

/********PRECISE COMPARISONS TEMPLATE UTILITIES************/
/**@addtogroup PreciseComparisson
 * PreciseComparisson Template utilities
 * @{ */
/** Precise comparison: equal than.
* @param v1 type T, variable 1 to compare
* @param v2 type T, variable 2 to compare
* @return bool
*/
template<typename T>
bool isequalthan(const T& v1, const T& v2)
{
    return std::abs(v1-v2)<= std::numeric_limits<T>::epsilon();
}
/** @} */

class histo_error : public std::runtime_error {
public:
    histo_error(const std::string & s) : std::runtime_error(s){ };

};

/** Simple Histogram. PRECI is the precission of stored non-int values. PRECI_INTEGER is the preci for integer values. T is the data type. PRECI should be the same type as T, except when T is int.
 *
*/
/**
 * @brief Histogram inspired by R.
 * Simple, no dependancies, header-only.
 * It accepts different data values and precission.
 *
 * @tparam T Input data type.
 * @tparam PRECI It should be equal to T, except when T is int.
 * @tparam PRECI_INTEGER int type for counts data member.
 */
template <typename T, typename PRECI = double, typename PRECI_INTEGER = unsigned long int>
struct Histo {

/************* DATA *****************/
    /** Low and upper limit for breaks. */
    std::pair<T,T> range;
    /** Value of the breaks between bins, [low,...,upper].
     *  size.breaks = size.counts + 1. */
    std::vector<T> breaks;
    /** breaks.size() */
    unsigned long int bins{0};
    /** int vector holding the counts for each breaks interval.*/
    std::vector<PRECI_INTEGER> counts;

/********** CONSTRUCTORS ************/
    Histo() = default;

    /**
     * @brief Constructor that takes range as the min, and max values of data.
     *
     * @param data
     * @param method Method to calculate breaks from @histo::breaks_method
     */
    Histo( const std::vector<T> &data, histo::breaks_method method = Scott )
    {
        auto range_ptr = std::minmax_element(data.begin(), data.end());
        range     = std::make_pair(*range_ptr.first, *range_ptr.second);
        breaks    = CalculateBreaks(data, range, method);
        bins      = static_cast<decltype(bins)>( breaks.size() - 1);
        ResetCounts();
        FillCounts(data);
    };

    /**
     * @brief Constructor with fixed input range.
     * @param data
     * @param input_range low and upper value
     * @param method Method to calculate breaks from @histo::breaks_method
     */
    Histo(const std::vector<T> &data, std::pair<T,T> input_range, histo::breaks_method method = Scott ){
        breaks    = CalculateBreaks(data, range, method);
        bins      = static_cast<decltype(bins)>( breaks.size() - 1 );
        ResetCounts();
        FillCounts(data);
    };
    /**
     * @brief Constructor that accepts a vector of breaks.
     * You can use @histo::GenerateBreaksFromRangeAndBins
     * to help you creating the vector from specific number of bins and range.
     *
     * @param data
     * @param input_breaks
     */
    Histo(const std::vector<T> &data, std::vector<T> &input_breaks) {
        breaks(input_breaks);
        bins      = static_cast<decltype(bins)>( breaks.size() - 1 );
        ResetCounts();
        FillCounts(data);
    };

/********* PUBLIC METHODS ***********/
    /**
     * @brief Return the index of @counts associated to the input value
     *
     * @param value Ranging from range.first to range.second
     *
     * @return Index of counts
     */
    unsigned long int IndexFromValue(const T &value){
        unsigned long int lo{0},hi{bins}, newb; // include right border in the last bin.
        if(value >= breaks[lo] && (value < breaks[hi] || isequalthan(value,breaks[hi]) )){
            while( hi - lo >= 2){
                newb = (hi-lo)/2;
                if (value >= breaks[newb]) lo = newb;
                else hi = newb;
            }
        } else {
            throw histo_error(" Value: "+ std::to_string(value) +  " is out of bonds");
        }

        return lo;
    };

    void ResetCounts(){
        counts.resize(bins);
        std::for_each(std::begin(counts), std::end(counts),
                [](PRECI_INTEGER & vh){ vh = 0; });
    };

    /**
     * @brief Fill counts from data.
     * Breaks must have been set-up before calling this method.
     *
     * @param data
     *
     * @return Reference to the data member @counts
     */
    std::vector<PRECI_INTEGER>& FillCounts(const std::vector<T> &data){
        std::for_each(std::begin(data),std::end(data),
                [this](const T & v){
                    auto i   = IndexFromValue(v);
                    counts[i]++;
                });
        return counts;
    };

private:
    /**
     * @brief Method to wrap breaks calculation methods that
     * take into account the input data and range to optimize breaks vector.
     *
     * @param data
     * @param rang Range of breaks vector (low, upper)
     * @param method Method to calculate breaks from @histo::breaks_method
     *
     * @return Reference to data member: breaks.
     */
    std::vector<T>& CalculateBreaks(const std::vector<T> & data, const std::pair<T,T> & rang, histo::breaks_method method ){
        switch(method) {
        case Scott:
             return ScottMethod(data,rang);
             break;
        default:
             throw histo_error("No Method selected to calculate automatically breaks");
        }
    };

    /**
     * @brief Scott Method to calculate optimal breaks.
     * Calculate variance from @histo::variance_welford
     *
     * @param data
     * @param rang Range of breaks vector (low, upper)
     *
     * @return Reference to data member: breaks.
     */
    std::vector<T>& ScottMethod(const std::vector<T> &data, const std::pair<T,T> &rang){
        PRECI sigma = variance_welford<PRECI,std::vector<T>>(data);
        PRECI width  = 3.5 * sqrt(sigma) / static_cast<PRECI>( data.size() );
        unsigned long int n    = std::ceil( (rang.second - rang.first) / width);
        breaks.resize(n);
        for (unsigned long int i = 0; i!=n; i++){
            breaks[i] = range.first + i*width;
        }
        // Just in case... create new bins if the upper value is not included.
        while(breaks.back() < range.second){
            breaks.push_back((n+1)*width);
            n++;
        }
        // std::cout<< "n is: " << n <<" width is: "<< width  <<std::endl;
        // std::for_each(std::begin(breaks), std::end(breaks), [](const T& v) {std::cout<<v <<std::endl;});
        return breaks;
    };
};

} // End of namespace histo
#endif
