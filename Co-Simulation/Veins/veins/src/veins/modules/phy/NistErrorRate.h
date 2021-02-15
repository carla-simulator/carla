/*
 * Copyright (c) 2010 The Boeing Company
 * Copyright (c) 2014 Michele Segata <segata@ccs-labs.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */
#pragma once

#include <stdint.h>
#include <cmath>
#include "veins/modules/utility/ConstsPhy.h"

namespace veins {

/**
 * Model the error rate for different modulations and coding schemes.
 * Taken from the nist wifi model of ns-3
 */
class VEINS_API NistErrorRate {
public:
    NistErrorRate();

    static double getChunkSuccessRate(unsigned int datarate, enum Bandwidth bw, double snr_mW, uint32_t nbits);

private:
    /**
     * Return the coded BER for the given p and b.
     *
     * \param p
     * \param bValue
     * \return BER
     */
    static double calculatePe(double p, uint32_t bValue);
    /**
     * Return BER of BPSK at the given SNR.
     *
     * \param snr snr value
     * \return BER of BPSK at the given SNR
     */
    static double getBpskBer(double snr);
    /**
     * Return BER of QPSK at the given SNR.
     *
     * \param snr snr value
     * \return BER of QPSK at the given SNR
     */
    static double getQpskBer(double snr);
    /**
     * Return BER of QAM16 at the given SNR.
     *
     * \param snr snr value
     * \return BER of QAM16 at the given SNR
     */
    static double get16QamBer(double snr);
    /**
     * Return BER of QAM64 at the given SNR.
     *
     * \param snr snr value
     * \return BER of QAM64 at the given SNR
     */
    static double get64QamBer(double snr);
    /**
     * Return BER of BPSK at the given SNR after applying FEC.
     *
     * \param snr snr value
     * \param nbits the number of bits in the chunk
     * \param bValue
     * \return BER of BPSK at the given SNR after applying FEC
     */
    static double getFecBpskBer(double snr, uint32_t nbits, uint32_t bValue);
    /**
     * Return BER of QPSK at the given SNR after applying FEC.
     *
     * \param snr snr value
     * \param nbits the number of bits in the chunk
     * \param bValue
     * \return BER of QPSK at the given SNR after applying FEC
     */
    static double getFecQpskBer(double snr, uint32_t nbits, uint32_t bValue);
    /**
     * Return BER of QAM16 at the given SNR after applying FEC.
     *
     * \param snr snr value
     * \param nbits the number of bits in the chunk
     * \param bValue
     * \return BER of QAM16 at the given SNR after applying FEC
     */
    static double getFec16QamBer(double snr, uint32_t nbits, uint32_t bValue);
    /**
     * Return BER of QAM64 at the given SNR after applying FEC.
     *
     * \param snr snr value
     * \param nbits the number of bits in the chunk
     * \param bValue
     * \return BER of QAM64 at the given SNR after applying FEC
     */
    static double getFec64QamBer(double snr, uint32_t nbits, uint32_t bValue);
};

} // namespace veins
