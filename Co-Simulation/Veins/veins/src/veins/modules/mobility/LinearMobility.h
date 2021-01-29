//
// Copyright (C) 2005 Emin Ilker Cetinbas
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

// author:      Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// part of:     framework implementation developed by tkn

#pragma once

#include "veins/base/modules/BaseMobility.h"

namespace veins {

/**
 * @brief Linear movement model. See NED file for more info.
 *
 * This mobility module expects a torus as playground ("useTorus"
 * Parameter of BaseWorldUtility module).
 *
 * NOTE: Does not yet support 3-dimensional movement.
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class VEINS_API LinearMobility : public BaseMobility {
protected:
    double angle; ///< angle of linear motion
    double acceleration; ///< acceleration of linear motion

    /** @brief always stores the last step for position display update */
    Coord stepTarget;

public:
    /** @brief Initializes mobility model parameters.*/
    void initialize(int) override;

protected:
    /** @brief Move the host*/
    void makeMove() override;

    void fixIfHostGetsOutside() override;
};

} // namespace veins
