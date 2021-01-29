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

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/POA.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/base/phyLayer/AnalogueModel.h"

namespace veins {

/**
 * A Signal represents the power measured on a channel.
 *
 * The Signal is parameterized by the frequency range, time interval and spatial posititon.
 * The signal power is stored in milliwatt.
 * Signals can be combined arithmetically to, e.g., compute interference introduced by several overlapping signals.
 *
 * @see SignalUtils
 * @see Spectrum
 */
class VEINS_API Signal {
public:
    Signal() = default;

    /**
     * Copy another Signal.
     */
    Signal(const Signal& other);

    /**
     * Create a Signal with zero power and without timing information.
     */
    explicit Signal(Spectrum spec);

    /**
     * Create a Signal with zero power for a time interval.
     */
    Signal(Spectrum spec, simtime_t start, simtime_t duration);
    ~Signal() = default;

    /**
     * Get the Spectrum this Signal is defined on.
     */
    const Spectrum& getSpectrum() const;

    /**
     * @name Element access
     */
    ///@{
    /**
     * Get the power in milliwatt for the given frequency index.
     *
     * @param index index of the power level to return
     * @return a reference to the power level
     */
    double& at(size_t index);

    /**
     * Get the power in milliwatt for the given frequency index.
     *
     * @param index index of the power level to return
     * @return a reference to the power level
     */
    const double& at(size_t index) const;

    /**
     * Get the power in milliwatt for the given frequency.
     *
     * @param freq frequency of the power level to return
     * @return a reference to the power level
     */
    double& atFrequency(double freq);

    /**
     * Get the power in milliwatt for the given frequency.
     *
     * @param freq frequency of the power level to return
     * @return a reference to the power level
     */
    const double& atFrequency(double freq) const;

    /**
     * Access the underlying power values directly.
     *
     * @see getNumValues()
     * @return A pointer to the individual values. The amount of valid entries is defined by getNumValues.
     */
    double* getValues();

    /**
     * Returns the number of power values stored in this signal.
     *
     * Corresponds to the associated Spectrum's number of defined frequencies.
     */
    size_t getNumValues() const;

    /**
     * Return the maximum power level of any frequency.
     */
    double getMax() const;
    ///@}

    /**
     * @name Element access on the defined data interval
     *
     * The data interval is a subrange of frequencies, which are used to transmit the actual data. This range is relevant for computing signal power since power on other frequencies does not influence the encoded data.
     */
    ///@{
    /**
     * Get the power in milliwatt for the given data frequency index.
     *
     * @param index index of the power level to return
     * @return a reference to the power level
     */
    double& dataAt(size_t index);

    /**
     * Get the power in milliwatt for the given data frequency index.
     *
     * @param index index of the power level to return
     * @return a reference to the power level
     */
    const double& dataAt(size_t index) const;

    /**
     * Get the absolute frequency index of the first data frequency.
     */
    size_t getDataStart() const;

    /**
     * Get the absolute frequency index of first past-the-end data frequency.
     */
    size_t getDataEnd() const;

    /**
     * Access the underlying data range power levels directly.
     *
     * @see getNumDataValues()
     */
    double* getDataValues();

    /**
     * The number of values in the data frequency subrange.
     */
    size_t getNumDataValues() const;

    /**
     * Get the offset of the data frequency range in the absolute frequency range.
     *
     * Synonym for getDataStart()
     */
    size_t getDataOffset() const;

    /**
     * Minimum power level in milliwatt in the data frequency range.
     */
    double getDataMin() const;

    /**
     * Maximum power level in milliwatt in the data frequency range.
     */
    double getDataMax() const;

    /**
     * Change the data frequency range's start.
     *
     * @param index new data frequency range's absolute start index
     */
    void setDataStart(size_t index);

    /**
     * Change the data frequency range's end.
     *
     * This implicitly changes the number of values in the data frequncey range.
     *
     * @param index new data frequency range's absolute end index
     */
    void setDataEnd(size_t index);

    /**
     * Change the data frequency range's length.
     *
     * This implicitly changes the data frequncey range's end.
     *
     * @param num new data frequency range's length
     */
    void setDataNumValues(size_t num);
    ///@}

    /**
     * @name Center frequency access
     */
    ///@{
    /**
     * Get the center frequency's index.
     */
    size_t getCenterFrequencyIndex() const;

    /**
     * Get the center frequency's power level in milliwatt.
     */
    double getAtCenterFrequency() const;

    /**
     * Set the center frequency position.
     *
     * @param index the new center frequency's absolute frequency index
     */
    void setCenterFrequencyIndex(size_t index);

    /**
     * Predicate testing whether the power level at the center frequency exceeds a threshold.
     *
     * @param threshold the threshold to test
     */
    bool greaterAtCenterFrequency(double threshold);

    /**
     * Predicate testing whether the power level at the center frequency does not exceed a threshold.
     *
     * @param threshold the threshold to test
     */
    bool smallerAtCenterFrequency(double threshold);
    ///@}

    /**
     * @name Analogue models
     */
    ///@{
    /**
     * Get the number of AnalogueModels that have already been applied to the Signal.
     */
    uint16_t getNumAnalogueModelsApplied() const;

    /**
     * Get the AnalogueModels associated with this Signal.
     */
    AnalogueModelList* getAnalogueModelList() const;

    /**
     * Set the AnalogueModels associated with this Signal.
     *
     * @param list the new list of AnalogueModels
     */
    void setAnalogueModelList(AnalogueModelList* list);

    /**
     * Apply a specific AnalogueModel.
     *
     * @param index the index in the analogue model list of the model to be applied
     *
     * @see getAnalogueModelList()
     * @see AnalogueModel::filterSignal()
     */
    void applyAnalogueModel(uint16_t index);

    /**
     * Apply all AnalogueModels.
     *
     * The models are applied in the same order of the AnalogueModel list.
     *
     * @see getAnalogueModelList()
     * @see AnalogueModel::filterSignal()
     */
    void applyAllAnalogueModels();
    ///@}

    /**
     * @name Sender and receiver POAs
     */
    ///@{
    /**
     * Get this signal's sender POA.
     */
    POA getSenderPoa() const;

    /**
     * Get this signal's receiver POA.
     */
    POA getReceiverPoa() const;

    /**
     * Set this signal's sender POA.
     *
     * @param poa the new sender POA
     */
    void setSenderPoa(const POA& poa);

    /**
     * Set this signal's receiver POA.
     *
     * @param poa the new receiver POA
     */
    void setReceiverPoa(const POA& poa);
    ///@}

    /**
     * Timing
     */
    ///@{
    /**
     * Get the point in time at which the signal starts.
     *
     * Only valid if hasTiming() returns true.
     */
    simtime_t_cref getSendingStart() const;

    /**
     * Get the point in time at which the signal ends.
     *
     * Only valid if hasTiming() returns true.
     */
    simtime_t getSendingEnd() const;

    /**
     * Get the point in time at which the signal reception starts.
     *
     * This corresponds to the singal's start time, but is shifted by its propagation delay. Only valid if hasTiming() returns true.
     *
     * @see getPropagationDelay()
     */
    simtime_t getReceptionStart() const;

    /**
     * Get the point in time at which the signal reception ends.
     *
     * This corresponds to the singal's end time, but is shifted by its propagation delay. Only valid if hasTiming() returns true.
     *
     * @see getPropagationDelay()
     */
    simtime_t getReceptionEnd() const;

    /**
     * Get the duration.
     *
     * Only valid if hasTiming() returns true.
     */
    simtime_t_cref getDuration() const;

    /**
     * Get the propagation delay.
     *
     * The propagation delay is the time taken for the signal to travel from sender to receiver with the speed of light. Only valid if hasTiming() returns true.
     *
     * @note this value is explicly set and not necessarily computed based on the sender and receiver positions.
     */
    simtime_t_cref getPropagationDelay() const;

    /**
     * Predicate which indicates presence of timing information.
     */
    bool hasTiming() const;

    /**
     * Set the propagation delay.
     *
     * @param delay new propagation delay
     */
    void setPropagationDelay(simtime_t_cref delay);

    /**
     * Set the signal's timing information.
     *
     * @param start the new start time
     * @param duration the new duration
     */
    void setTiming(simtime_t start, simtime_t duration);
    ///@}

    /**
     * Assign a constant power level to all defined frequencies.
     *
     * @param value the power level in milliwatt
     */
    Signal& operator=(const double value);

    /**
     * Create a copy of another signal.
     *
     * @param other the other signal
     */
    Signal& operator=(const Signal& other);

    /**
     * @name Arithmetic operators
     */
    ///@{
    /**
     * Increment the power levels by another signal's power levels.
     *
     * @param other the other signal
     */
    Signal& operator+=(const Signal& other);

    /**
     * Increment the power levels by a constant.
     *
     * @param value power level to add in milliwatt
     */
    Signal& operator+=(const double value);

    /**
     * Decrement the power levels by another signal's power levels.
     *
     * @param other the other signal
     */
    Signal& operator-=(const Signal& other);

    /**
     * Decrement the power levels by a constant.
     *
     * @param value power level to substract in milliwatt
     */
    Signal& operator-=(const double value);

    /**
     * Multiply the power levels by another signal's power levels.
     *
     * @param other the other signal
     */
    Signal& operator*=(const Signal& other);

    /**
     * Multiply the power levels by a constant.
     *
     * @param value power level to multiply by in milliwatt
     */
    Signal& operator*=(const double value);

    /**
     * Divide the power levels by another signal's power levels.
     *
     * @param other the other signal
     */
    Signal& operator/=(const Signal& other);

    /**
     * Divide the power levels by a constant.
     *
     * @param value power level to divide by in milliwatt
     */
    Signal& operator/=(const double value);
    ///@}

    /**
     * Convert a signal to a human readable representation.
     *
     * @param os the outputstream to write to
     * @param s the signal to convert
     */
    friend std::ostream& operator<<(std::ostream& os, const Signal& s);

    /**
     * Calculate the common start of two signals, i.e. their minimum start times.
     *
     * @param lhs the first signal
     * @param rhs the second signal
     */
    friend inline simtime_t calculateStart(const Signal& lhs, const Signal& rhs);

    /**
     * Calculate the combined duration of two signals.
     *
     * @param lhs the first signal
     * @param rhs the second signal
     */
    friend inline simtime_t calculateDuration(const Signal& lhs, const Signal& rhs);

private:
    double getMinInRange(size_t freqIndexLow, size_t freqIndexHigh) const;
    double getMaxInRange(size_t freqIndexLow, size_t freqIndexHigh) const;

    Spectrum spectrum;

    std::vector<double> values;

    size_t numDataValues = 0;
    size_t dataOffset = 0;

    size_t centerFrequencyIndex = 0;

    bool timingUsed = false;
    /** @brief The start of the signal transmission at the sender module.*/
    simtime_t sendingStart = 0;
    /** @brief The duration of the signal transmission.*/
    simtime_t duration = 0;
    /** @brief The propagation delay of the transmission. */
    simtime_t propagationDelay = 0;

    AnalogueModelList* analogueModelList = nullptr;
    uint16_t numAnalogueModelsApplied = 0;

    POA senderPoa;
    POA receiverPoa;
};

/**
 * @name Arithmetic operators
 */
///@{
/**
 * Add two signals to each other.
 *
 * The resulting signal's power level is the sum of each individual signal's power levels.
 *
 * @param lhs the first signal
 * @param rhs the second signal
 */
Signal VEINS_API operator+(const Signal& lhs, const Signal& rhs);

/**
 * Increment a signal's power levels by a constant.
 *
 * @param lhs the signal to add
 * @param rhs power level to add in milliwatt
 */
Signal VEINS_API operator+(const Signal& lhs, double rhs);

/**
 * Increment a signal's power levels by a constant.
 *
 * @param lhs power level to add in milliwatt
 * @param rhs the signal to add
 */
Signal VEINS_API operator+(double lhs, const Signal& rhs);

/**
 * Substract two signals from each other.
 *
 * The resulting signal's power level is the difference of each individual signal's power levels.
 *
 * @param lhs the first signal
 * @param rhs the second signal
 */
Signal VEINS_API operator-(const Signal& lhs, const Signal& rhs);

/**
 * Decrement a signal's power levels by a constant.
 *
 * @param lhs the signal to substract from
 * @param rhs power level to substract in milliwatt
 */
Signal VEINS_API operator-(const Signal& lhs, double rhs);

/**
 * Decrement a constant power level by a signal's power levels.
 *
 * @param lhs power level to substract from in milliwatt
 * @param rhs the signal to substract
 */
Signal VEINS_API operator-(double lhs, const Signal& rhs);

/**
 * Multiply two signals by each other.
 *
 * The resulting signal's power level is the product of each individual signal's power levels.
 *
 * @param lhs the first signal
 * @param rhs the second signal
 */
Signal VEINS_API operator*(const Signal& lhs, const Signal& rhs);

/**
 * Multiply a signal's power levels by a constant.
 *
 * @param lhs the signal to multiply with
 * @param rhs power level to multiply by in milliwatt
 */
Signal VEINS_API operator*(const Signal& lhs, double rhs);

/**
 * Multiply a signal's power levels by a constant.
 *
 * @param lhs power level to multiply by in milliwatt
 * @param rhs the signal to multiply with
 */
Signal VEINS_API operator*(double lhs, const Signal& rhs);

/**
 * Divide two signals by each other.
 *
 * The resulting signal's power level is the division of each individual signal's power levels.
 *
 * @param lhs the first signal (dividend)
 * @param rhs the second signal (divisor)
 */
Signal VEINS_API operator/(const Signal& lhs, const Signal& rhs);

/**
 * Divide a signal's power levels by a constant.
 *
 * @param lhs the dividend
 * @param rhs the constnat divisor in milliwatt
 */
Signal VEINS_API operator/(const Signal& lhs, double rhs);

/**
 * Divide a a constant by a signal's power levels.
 *
 * @param rhs the constant dividend in milliwatt
 * @param lhs the divisor
 */
Signal VEINS_API operator/(double lhs, const Signal& rhs);
///@}

} // namespace veins
