#define FLEET
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PHEMlightdll
{
    public class Constants
    {
        //Calculation constant
        public const double GRAVITY_CONST = 9.81;
        public const double AIR_DENSITY_CONST = 1.182;
        public const double NORMALIZING_SPEED = 19.444;
        public const double NORMALIZING_ACCELARATION = 0.45;
        public const double SPEED_DCEL_MIN = 10 / 3.6;
        public const double ZERO_SPEED_ACCURACY = 0.5;
        public const double DRIVE_TRAIN_EFFICIENCY_All = 0.9;
        public const double DRIVE_TRAIN_EFFICIENCY_CB = 0.8;

        #if FLEET
        //Fleetmix constants
        public const double SPEED_ROTATIONAL_INCREMENT = 10 / 3.6;
        public const double POWER_FC_INCREMENT = 0.1;
        public const double POWER_POLLUTANT_INCREMENT = 0.3;
        public const double NNORM_INCREMENT = 0.05;
        #endif

        //Strings
        public const string HeavyVehicle = "HV";

        //Vehiclestrings
        public const string strPKW = "PC";
        public const string strLNF = "LCV";
        public const string strLKW = "HDV_RT";
        public const string strLSZ = "HDV_TT";
        public const string strRB = "HDV_CO";
        public const string strLB = "HDV_CB";
        public const string strMR2 = "MC_2S";
        public const string strMR4 = "MC_4S";
        public const string strKKR = "MOP";

        //Fuelstrings
        public const string strGasoline = "G";
        public const string strDiesel = "D";
        public const string strCNG = "CNG";
        public const string strLPG = "LPG";
        public const string strHybrid = "HEV";
        public const string strBEV = "BEV";

        //Euroclasses
        public const string strEU = "EU";

        //Sizeclasse
        public const string strSI = "I";
        public const string strSII = "II";
        public const string strSIII = "III";

        #if FLEET
        //Fleetmix
        private const string strAggPC = "PC";
        private const string strAggBUS = "BUS";
        private const string strAggHDV = "HDV";
        private const string strAggTW = "TW";
        public static string[] AGGREGATED_VEHICLECLASSES = new string[] { strAggPC,
                                                                          strAggHDV,
                                                                          strAggBUS,
                                                                          strAggTW};
        #endif

        //Drive train efficiency
        public static double _DRIVE_TRAIN_EFFICIENCY;
        public static double DRIVE_TRAIN_EFFICIENCY
        {
            get { return _DRIVE_TRAIN_EFFICIENCY; }
            set { _DRIVE_TRAIN_EFFICIENCY = value; }
        }

    }
}
