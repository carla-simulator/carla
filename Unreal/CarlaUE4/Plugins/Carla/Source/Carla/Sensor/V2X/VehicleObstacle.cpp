
// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>
#include <limits>
#include <vector>
#include <list>

#include "PathLossModel.h"

namespace
{
    auto compareDistance = [](const DiffractionObstacle &a, const DiffractionObstacle &b)
    { return a.d < b.d; };
    auto compareHeight = [](const DiffractionObstacle &a, const DiffractionObstacle &b)
    { return a.h < b.h; };

    using ObstacleIterator = std::list<DiffractionObstacle>::const_iterator;
    ObstacleIterator findMainObstacle(ObstacleIterator, ObstacleIterator);
    ObstacleIterator findSecondaryObstacle(ObstacleIterator, ObstacleIterator);
} // namespace

DiffractionObstacle::DiffractionObstacle(double distTx, double height) : d(distTx), h(height)
{
}

DiffractionPath::DiffractionPath() : attenuation(0.0), d(0.0)
{
}

double PathLossModel::computeSimpleKnifeEdge(double heightTx, double heightRx, double heightObs, double distTxRx, double distTxObs)
{
    // following calculations are similar to equation 29 of ITU-R P.526-13:
    //  v = sqrt(2d/lambda * alpha1 * alpha2)
    //
    // with sinus approximation for small angles:
    //  alpha1 =~ sin alpha1 = h / d1
    //  alpha2 =~ sin alpha2 = h / d2
    //
    //  v = sqrt(2d / lambda * h / d1 * h / d2) = sqrt(2) * h / sqrt(lambda * d1 * d2 / d)
    //
    // with d1 = distTxObs, d2 = distRxObs, d = distTxRx,

    // distance between Rx and obstacle
    const double distRxObs = distTxRx - distTxObs;
    // signed height relative to the line connecting Tx and Rx at obstacle position
    const double obsHeightTxRxLine = (heightRx - heightTx) / distTxRx * distTxObs + heightTx;
    const double h = heightObs - obsHeightTxRxLine;
    // calculate the Fresnel ray
    const double r = sqrt(lambda * distTxObs * distRxObs / distTxRx);
    static const double root_two = sqrt(2.0);
    const double v = root_two * h / r;

    double loss = 0.0;
    if (v > -0.78)
    {
        // approximation of Fresnel-Kirchoff loss given by ITU-R P.526, equation 31 (result in dB):
        // J(v) = 6.9 + 20 log(sqrt((v - 01)^2 + 1) + v - 0.1)
        loss = 6.9 + 20.0 * log10(sqrt(std::pow(v - 0.1, 2) + 1.0) + v - 0.1);
    }

    return loss;
}

DiffractionPath PathLossModel::computeMultipleKnifeEdge(const std::list<DiffractionObstacle> &obs)
{
    DiffractionPath path;

    // determine main and secondary obstacles
    std::vector<ObstacleIterator> mainObs;
    mainObs.push_back(obs.begin()); // Tx
    for (ObstacleIterator it = obs.begin(); it != obs.end();)
    {
        it = findMainObstacle(it, obs.end());
        if (it != obs.end())
        {
            mainObs.push_back(it);
        }
    }
    // NOTE: Rx is added by loop as last main obstacle
    struct SecondaryObstacle
    {
        SecondaryObstacle(ObstacleIterator tx, ObstacleIterator obs, ObstacleIterator rx) : tx(tx), obstacle(obs), rx(rx) {}
        ObstacleIterator tx;
        ObstacleIterator obstacle;
        ObstacleIterator rx;
    };

    std::vector<SecondaryObstacle> secObs;
    std::vector<double> mainObsDistances;
    for (std::size_t i = 0, j = 1; j < mainObs.size(); ++i, ++j)
    {
        const double d = mainObs[j]->d - mainObs[i]->d;
        path.d += sqrt(std::pow(d, 2) + std::pow(mainObs[j]->h - mainObs[i]->h, 2));
        mainObsDistances.push_back(d);

        const auto delta = std::distance(mainObs[i], mainObs[j]);
        if (delta == 2)
        {
            // single other obstacle between two main obstacles
            secObs.emplace_back(mainObs[i], std::next(mainObs[i]), mainObs[j]);
        }
        else if (delta > 2)
        {
            secObs.emplace_back(mainObs[i], findSecondaryObstacle(mainObs[i], mainObs[j]), mainObs[j]);
        }
    }

    // attenuation due to main obstacles
    double attMainObs = 0.0;
    for (std::size_t i = 0; i < mainObs.size() - 2; ++i)
    {
        const double distTxObs = mainObsDistances[i];
        const double distTxRx = distTxObs + mainObsDistances[i + 1];
        attMainObs += computeSimpleKnifeEdge(mainObs[i]->h, mainObs[i + 2]->h, mainObs[i + 1]->h, distTxRx, distTxObs);
    }

    // attenuation due to secondary obstacles
    double attSecObs = 0.0;
    for (const SecondaryObstacle &sec : secObs)
    {
        const double distTxRx = sec.rx->d - sec.tx->d;
        const double distTxObs = sec.obstacle->d - sec.tx->d;
        attSecObs += computeSimpleKnifeEdge(sec.tx->h, sec.rx->h, sec.obstacle->h, distTxRx, distTxObs);
    }

    // correction factor C (see eq. 46 in ITU-R P.526-13)
    double C = mainObs.back()->d - mainObs.front()->d; // distance between Tx and Rx
    for (double d : mainObsDistances)
    {
        C *= d;
    }
    double pairwiseDistProduct = 1.0;
    for (std::size_t i = 1; i < mainObsDistances.size(); ++i)
    {
        pairwiseDistProduct *= mainObsDistances[i - 1] + mainObsDistances[i];
    }
    C /= mainObsDistances.front() * mainObsDistances.back() * pairwiseDistProduct;

    path.attenuation = attMainObs + attSecObs - 10.0 * log10(C);
    return path;
}

std::list<DiffractionObstacle> PathLossModel::buildTopObstacles(const std::vector<FVector> &vehicles, const FVector &pos_tx, const FVector &pos_rx)
{
    std::list<DiffractionObstacle> diffTop;
    const double vx = pos_rx.X - pos_tx.X;
    const double vy = pos_rx.Y - pos_tx.Y;

    for (auto vehicle : vehicles)
    {
        const double midpoint_x = vehicle.X;
        const double midpoint_y = vehicle.Y;
        const double k = (midpoint_x * vx - vy * pos_tx.Y + vy * midpoint_y - pos_tx.X * vx) / (std::pow(vx, 2) + std::pow(vy, 2));
        if (k < 0.0 || k > 1.0)
            continue; /*< skip points beyond the ends of TxRx line segment */
        const double d = k * sqrt(std::pow(vx, 2) + std::pow(vy, 2));
        diffTop.emplace_back(d, vehicle.Z);
    }

    diffTop.sort(compareDistance);
    return diffTop;
}

namespace
{

    ObstacleIterator findMainObstacle(ObstacleIterator begin, ObstacleIterator end)
    {
        ObstacleIterator mainIterator = end;
        double mainAngle = -std::numeric_limits<double>::infinity();

        if (begin != end)
        {
            for (ObstacleIterator it = std::next(begin); it != end; ++it)
            {
                double angle = (it->h - begin->h) / (it->d - begin->d);
                if (angle > mainAngle)
                {
                    mainIterator = it;
                    mainAngle = angle;
                }
            }
        }

        return mainIterator;
    }

    ObstacleIterator findSecondaryObstacle(ObstacleIterator first, ObstacleIterator last)
    {
        ObstacleIterator secIterator = last;
        double secHeightGap{std::numeric_limits<double>::infinity()};

        const double distFirstLast = last->d - first->d;
        const double heightFirstLast = last->h - first->h;
        const auto offset = first->h * last->d - first->d * last->h;
        for (ObstacleIterator it = std::next(first); it != last; ++it)
        {
            const double heightGap = ((it->d * heightFirstLast + offset) / distFirstLast) - it->h;
            if (heightGap < secHeightGap)
            {
                secIterator = it;
                secHeightGap = heightGap;
            }
        }

        return secIterator;
    }

} // namespace

// statistical model from ETSI TR 103 257-1 V1.1.1 (2019-05)
double PathLossModel::MakeVehicleBlockageLoss(double TxHeight, double RxHeight, double obj_height, double obj_distance)
{
    // according to ETSI, stochastic method
    if (TxHeight > obj_height && RxHeight > obj_height)
    {
        // no blocking if higher than obj
        return 0.0;
    }
    else if (TxHeight < obj_height && RxHeight < obj_height)
    {
        // worst case: obj is higher than both tx and rx
        float mean = 9.0f + fmax(0.0f, 15.0f * log10(obj_distance) - 41.0f);
        return mRandomEngine->GetNormalDistribution(mean, 4.5f);
    }
    else
    {
        // something in between
        float mean = 5.0f + fmax(0.0f, 15.0f * log10(obj_distance) - 41.0f);
        return mRandomEngine->GetNormalDistribution(mean, 4.0f);
    }
}
