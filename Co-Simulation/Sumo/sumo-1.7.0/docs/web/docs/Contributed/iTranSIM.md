---
title: Contributed/iTranSIM
permalink: /Contributed/iTranSIM/
---

## Summary

<b>iTranSIM - Simulation-based Vehicle Location</b>

By: Tino Morenz<br>
Supervisor: Dr. René Meier

Public transport provides affordable, 'eco'-friendly transport in urban
environments. With the advancements in computer technologies real-time
passenger information (RTPI) are now available for passengers, helping
them to plan their trips so they spend as little time as needed
travelling.

To provide real time passenger information it is necessary to have
information on vehicle positions, so that travel times can be calculated
and presented to the passenger. The iTranSIM system aims to provide
vehicle locations not based on physical measurements but solely based on
statistical data. Instead of locating the real vehicle e.g. using GPS, a
virtual vehicle is tracked in an online simulation of Dublin City. The
online simulation can be supplied with real traffic data such as vehicle
counts from induction loops to create a realistic model of the traffic.

To demonstrate the feasibility of this approach the iTranSIM system was
designed and a proof-of-concept was implemented based on the iTransIT
framework. This framework was developed by the Distributed Systems Group
(DSG) at Trinity College Dublin and provides means of integrating
different components of an intelligent transportation system (ITS) so
that data can be exchanged between the systems. To simulate the traffic
the microscopic traffic simulator SUMO (Simulation of Urban Mobility) is
used. SUMO is an open source project, maintained by researchers at the
Centre for Applied Informatics at the University of Cologne and the
Institute of Transport Research at the German Aerospace Centre.

The accuracy of vehicle location based on simulation could not be
evaluated yet because a vital part of the necessary infrastructure is
not yet available: the vehicle count data from real induction loops.
Thus all experiments described in this thesis are based on empirical
traffic data, however as soon as live data becomes available the
accuracy of the proposed method can be analysed without any changes to
the proposed system. Possible means of performing such an evaluation are
also included in this dissertation.

## Complete Thesis

["iTranSIM - Simulation-based Vehicle Location"](https://www.cs.tcd.ie/publications/tech-reports/reports.07/TCD-CS-2007-56.pdf)
by Tino Morenz