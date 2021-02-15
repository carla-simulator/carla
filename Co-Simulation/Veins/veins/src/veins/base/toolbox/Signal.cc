//
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/base/toolbox/Signal.h"

#include <sstream>

#include "veins/base/phyLayer/AnalogueModel.h"

namespace veins {

Signal::Signal(const Signal& other)
    : spectrum(other.spectrum)
    , values(other.values)
    , numDataValues(other.numDataValues)
    , dataOffset(other.dataOffset)
    , centerFrequencyIndex(other.centerFrequencyIndex)
    , timingUsed(other.timingUsed)
    , sendingStart(other.sendingStart)
    , duration(other.duration)
    , propagationDelay(other.propagationDelay)
    , analogueModelList(other.analogueModelList)
    , numAnalogueModelsApplied(other.numAnalogueModelsApplied)
    , senderPoa(other.senderPoa)
    , receiverPoa(other.receiverPoa)
{
}

Signal::Signal(Spectrum spec)
    : spectrum(spec)
    , values(spectrum.getNumFreqs(), 0)
{
}

Signal::Signal(Spectrum spec, simtime_t start, simtime_t dur)
    : spectrum(spec)
    , values(spectrum.getNumFreqs(), 0)
    , timingUsed(true)
    , sendingStart(start)
    , duration(dur)
{
}

const Spectrum& Signal::getSpectrum() const
{
    return spectrum;
}

double& Signal::at(size_t index)
{
    return values.at(index);
}

const double& Signal::at(size_t index) const
{
    return values.at(index);
}

double& Signal::atFrequency(double frequency)
{
    size_t index = spectrum.indexOf(frequency);
    return values.at(index);
}

const double& Signal::atFrequency(double frequency) const
{
    size_t index = spectrum.indexOf(frequency);
    return values.at(index);
}

double* Signal::getValues()
{
    return values.data();
}

size_t Signal::getNumValues() const
{
    return values.size();
}

double Signal::getMax() const
{
    return getMaxInRange(0, values.size());
}

double& Signal::dataAt(size_t index)
{
    return values.at(dataOffset + index);
}

const double& Signal::dataAt(size_t index) const
{
    return values.at(dataOffset + index);
}

size_t Signal::getDataStart() const
{
    return dataOffset;
}

size_t Signal::getDataEnd() const
{
    return dataOffset + numDataValues;
}

double* Signal::getDataValues()
{
    return values.data() + dataOffset;
}

size_t Signal::getNumDataValues() const
{
    return numDataValues;
}

size_t Signal::getDataOffset() const
{
    return dataOffset;
}

double Signal::getDataMin() const
{
    return getMinInRange(dataOffset, dataOffset + numDataValues);
}

double Signal::getDataMax() const
{
    return getMaxInRange(dataOffset, dataOffset + numDataValues);
}

void Signal::setDataStart(size_t index)
{
    dataOffset = index;
}

void Signal::setDataEnd(size_t index)
{
    numDataValues = 1 + index - dataOffset;
}

void Signal::setDataNumValues(size_t num)
{
    numDataValues = num;
}

size_t Signal::getCenterFrequencyIndex() const
{
    return centerFrequencyIndex;
}

double Signal::getAtCenterFrequency() const
{
    return values[centerFrequencyIndex];
}

void Signal::setCenterFrequencyIndex(size_t index)
{
    centerFrequencyIndex = index;
}

bool Signal::greaterAtCenterFrequency(double threshold)
{
    if (values[centerFrequencyIndex] < threshold) return false;

    uint16_t maxAnalogueModels = analogueModelList->size();

    while (numAnalogueModelsApplied < maxAnalogueModels) {
        // Apply filter here
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this);
        numAnalogueModelsApplied++;

        if (values[centerFrequencyIndex] < threshold) return false;
    }
    return true;
}

bool Signal::smallerAtCenterFrequency(double threshold)
{
    if (values[centerFrequencyIndex] < threshold) return true;

    uint16_t maxAnalogueModels = analogueModelList->size();

    while (numAnalogueModelsApplied < maxAnalogueModels) {
        // Apply filter here
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this);
        numAnalogueModelsApplied++;

        if (values[centerFrequencyIndex] < threshold) return true;
    }
    return false;
}

uint16_t Signal::getNumAnalogueModelsApplied() const
{
    return numAnalogueModelsApplied;
}

AnalogueModelList* Signal::getAnalogueModelList() const
{
    return analogueModelList;
}

void Signal::setAnalogueModelList(AnalogueModelList* list)
{
    analogueModelList = list;
}

void Signal::applyAnalogueModel(uint16_t index)
{
    uint16_t maxAnalogueModels = analogueModelList->size();

    if (index >= maxAnalogueModels || index < numAnalogueModelsApplied) return;

    (*analogueModelList)[index]->filterSignal(this);
    numAnalogueModelsApplied++;
}

void Signal::applyAllAnalogueModels()
{
    uint16_t maxAnalogueModels = analogueModelList->size();
    while (numAnalogueModelsApplied < maxAnalogueModels) {
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this);

        numAnalogueModelsApplied++;
    }
}

POA Signal::getSenderPoa() const
{
    return senderPoa;
}

POA Signal::getReceiverPoa() const
{
    return receiverPoa;
}

void Signal::setSenderPoa(const POA& poa)
{
    senderPoa = poa;
}

void Signal::setReceiverPoa(const POA& poa)
{
    receiverPoa = poa;
}

simtime_t_cref Signal::getSendingStart() const
{
    return sendingStart;
}

simtime_t Signal::getSendingEnd() const
{
    return sendingStart + duration;
}

simtime_t Signal::getReceptionStart() const
{
    return sendingStart + propagationDelay;
}

simtime_t Signal::getReceptionEnd() const
{
    return sendingStart + propagationDelay + duration;
}

simtime_t_cref Signal::getDuration() const
{
    return duration;
}

simtime_t_cref Signal::getPropagationDelay() const
{
    return propagationDelay;
}

bool Signal::hasTiming() const
{
    return timingUsed;
}

void Signal::setPropagationDelay(simtime_t_cref delay)
{
    propagationDelay = delay;
}

void Signal::setTiming(simtime_t start, simtime_t dur)
{
    sendingStart = start;
    duration = dur;
    timingUsed = true;
}

Signal& Signal::operator=(const double value)
{
    std::fill(values.begin(), values.end(), value);
    return *this;
}

Signal& Signal::operator=(const Signal& other)
{
    if (this == &other) return *this;

    spectrum = other.getSpectrum();

    dataOffset = other.getDataOffset();

    centerFrequencyIndex = other.getCenterFrequencyIndex();

    numDataValues = other.getNumDataValues();

    values = other.values;

    analogueModelList = other.getAnalogueModelList();
    numAnalogueModelsApplied = other.getNumAnalogueModelsApplied();
    senderPoa = other.getSenderPoa();
    receiverPoa = other.getReceiverPoa();

    timingUsed = other.hasTiming();
    sendingStart = other.getSendingStart();
    duration = other.getDuration();
    propagationDelay = other.getPropagationDelay();

    return *this;
}

Signal& Signal::operator+=(const Signal& other)
{
    ASSERT(this->getSpectrum() == other.getSpectrum());
    ASSERT(!(this->timingUsed && other.timingUsed) || (this->sendingStart == other.sendingStart && this->duration == other.duration));

    std::transform(values.begin(), values.end(), other.values.begin(), values.begin(), std::plus<double>());
    return *this;
}

Signal& Signal::operator+=(const double value)
{
    std::transform(values.begin(), values.end(), values.begin(), [value](double other) { return other + value; });
    return *this;
}

Signal& Signal::operator-=(const Signal& other)
{
    ASSERT(this->getSpectrum() == other.getSpectrum());
    ASSERT(!(this->timingUsed && other.timingUsed) || (this->sendingStart == other.sendingStart && this->duration == other.duration));

    std::transform(values.begin(), values.end(), other.values.begin(), values.begin(), std::minus<double>());
    return *this;
}

Signal& Signal::operator-=(const double value)
{
    std::transform(values.begin(), values.end(), values.begin(), [value](double other) { return other - value; });
    return *this;
}

Signal& Signal::operator*=(const Signal& other)
{
    ASSERT(this->getSpectrum() == other.getSpectrum());
    ASSERT(!(this->timingUsed && other.timingUsed) || (this->sendingStart == other.sendingStart && this->duration == other.duration));

    std::transform(values.begin(), values.end(), other.values.begin(), values.begin(), std::multiplies<double>());
    return *this;
}

Signal& Signal::operator*=(const double value)
{
    std::transform(values.begin(), values.end(), values.begin(), [value](double other) { return other * value; });
    return *this;
}

Signal& Signal::operator/=(const Signal& other)
{
    ASSERT(this->getSpectrum() == other.getSpectrum());
    ASSERT(!(this->timingUsed && other.timingUsed) || (this->sendingStart == other.sendingStart && this->duration == other.duration));

    std::transform(values.begin(), values.end(), other.values.begin(), values.begin(), std::divides<double>());
    return *this;
}

Signal& Signal::operator/=(const double value)
{
    std::transform(values.begin(), values.end(), values.begin(), [value](double other) { return other / value; });
    return *this;
}

Signal operator+(const Signal& lhs, const Signal& rhs)
{
    Signal result(lhs);
    result += rhs;
    return result;
}

Signal operator+(const Signal& lhs, double rhs)
{
    Signal result(lhs);
    result += rhs;
    return result;
}

Signal operator+(double lhs, const Signal& rhs)
{
    Signal result(rhs);
    result += lhs;
    return result;
}

Signal operator-(const Signal& lhs, const Signal& rhs)
{
    Signal result(lhs);
    result -= rhs;
    return result;
}

Signal operator-(const Signal& lhs, double rhs)
{
    Signal result(lhs);
    result -= rhs;
    return result;
}

Signal operator-(double lhs, const Signal& rhs)
{
    return lhs + (-1 * rhs);
}

Signal operator*(const Signal& lhs, const Signal& rhs)
{
    Signal result(lhs);
    result *= rhs;
    return result;
}

Signal operator*(const Signal& lhs, double rhs)
{
    Signal result(lhs);
    result *= rhs;
    return result;
}

Signal operator*(double lhs, const Signal& rhs)
{
    Signal result(rhs);
    result *= lhs;
    return result;
}

Signal operator/(const Signal& lhs, const Signal& rhs)
{
    Signal result(lhs);
    result /= rhs;
    return result;
}

Signal operator/(const Signal& lhs, double rhs)
{
    Signal result(lhs);
    result /= rhs;
    return result;
}

Signal operator/(double lhs, const Signal& rhs)
{
    // Create constant signal
    Signal sigLhs(rhs.getSpectrum());
    sigLhs = lhs;
    return sigLhs / rhs;
}

std::ostream& operator<<(std::ostream& os, const Signal& s)
{
    os << "Signal(";
    if (s.timingUsed) {
        os << "interval: (" << s.getReceptionStart() << ", " << s.getReceptionEnd() << "), ";
    }
    os << s.spectrum << ", ";
    std::ostringstream ss;
    for (auto&& value : s.values) {
        if (ss.tellp() != 0) {
            ss << ", ";
        }
        ss << value;
    }
    os << ss.str();
    os << ")";
    return os;
}

/***********************/

simtime_t calculateStart(const Signal& lhs, const Signal& rhs)
{
    return std::max(lhs.sendingStart, rhs.sendingStart);
}

simtime_t calculateDuration(const Signal& lhs, const Signal& rhs)
{
    simtime_t temp = std::min(lhs.sendingStart + lhs.duration, rhs.sendingStart + rhs.duration) - std::max(lhs.sendingStart, rhs.sendingStart);
    return (temp > 0) ? temp : 0;
}

double Signal::getMinInRange(size_t freqIndexLow, size_t freqIndexHigh) const
{
    return *(std::min_element(values.begin() + freqIndexLow, values.begin() + freqIndexHigh));
}

double Signal::getMaxInRange(size_t freqIndexLow, size_t freqIndexHigh) const
{
    return *(std::max_element(values.begin() + freqIndexLow, values.begin() + freqIndexHigh));
}

} // namespace veins
