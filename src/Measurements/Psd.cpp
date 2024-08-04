/**
 * @file Psd.cpp
 * @author Mikhail Kalina (apollo.mk58@gmail.com)
 * @brief Power Spectral Density calculation module implementation
 * @version 0.1
 * @date 2024-06-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "Measurements/Psd.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "arduinoFFT.h"
#include "Debug.hpp"

using namespace Measurements;

namespace
{
    // Coefficient for Hamming window correction
    constexpr double windowCorrection = 1.59;

    /**
     * Calculate average value for elements
     */
    template <typename T>
    T getAverage(T *elements, size_t count)
    {
        assert(elements);
        assert(count > 0);

        double sum = 0;
        for (size_t idx = 0; idx < count; idx++)
        {
            sum += elements[idx];
        }

        T result = static_cast<T>(sum / count);
        return result;
    }

    /*
    These are the input and output vectors
    Input vectors receive computed results from FFT
    */
    double vReal[PSD::samplesCountMax];
    double vImag[PSD::samplesCountMax];

    // FFT object
    auto fft = ArduinoFFT<double>();
} // namespace

/**
 * @brief Prepare PSD calculations, setup data segment parameters
 *
 * @param[in] sampleCount Samples count in the segment
 * @param[in] sampleFrequency Sampling frequency
 */
void PSD::setup(size_t sampleCount, size_t sampleFrequency)
{
    // Save samples count
    _sampleCount = sampleCount;
    // Save sampling frequency
    _sampleFrequency = sampleFrequency;

    // Reset computed segment count
    _segmentCount = 0;
}

/**
 * @brief Compute PSD for the next segment
 *
 * @param[in] samples Data samples of the segmennt
 */
void PSD::computeSegment(const int16_t *samples)
{
    if (_segmentCount == 0)
    {
        // Clear PSD results before adding new data
        clear();
    }

    auto average = getAverage(samples, _sampleCount);
    for (size_t idx = 0; idx < _sampleCount; idx++)
    {
        vReal[idx] = samples[idx] - average;
        vImag[idx] = 0;
    }

    fft.windowing(vReal, _sampleCount, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft.compute(vReal, vImag, _sampleCount, FFT_FORWARD);
    fft.complexToMagnitude(vReal, vImag, _sampleCount);

    for (size_t idx = 0; idx < _sampleCount; idx++)
    {
        double bin = static_cast<double>(vReal[idx]) * vReal[idx] / _sampleFrequency / _sampleCount;
        if (idx > 0)
        {
            bin *= 2;
        }

        _bins[idx] += bin;
    }

    _segmentCount++;
}

/**
 * @brief Return PSD results
 * Reset accumulated segment count (finish previous segments computing) if there are any segments
 *
 * @return Calculated bins (only the first N/2 + 1 are usefull, where N = sampleCount)
 */
const double *PSD::getResult()
{
    if (_segmentCount > 0)
    {
        // Calculate average bins
        for (size_t idx = 0; idx < _sampleCount; idx++)
        {
            _bins[idx] = (_bins[idx] / _segmentCount) * windowCorrection * windowCorrection;
        }

        // Reset number of segment to prevent repeated result calculation
        _segmentCount = 0;
    }

    return _bins;
}

/**
 * @brief Clear PSD results
 */
void PSD::clear()
{
    // Clear all bins
    for (size_t idx = 0; idx < _sampleCount; idx++)
    {
        _bins[idx] = 0;
    }
}
