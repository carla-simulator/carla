#define FLEET
using System;
using System.IO;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;

namespace PHEMlightdll
{
    public class Start
    {
        private List<string> _DataPath;
        private CEPHandler DataInput;
        public Helpers Helper = new Helpers();

        //******************* Parameters of Array or Single calculation *******************
        //********************************* INPUT ******************************************
        //***  DATATYP            |  UNIT   |  VARIBLE                  |     Description  ***
        //List<string>            |  [-]    |   DataFiles (VEH, FC, EMI)| Name of file (e.g. "PC_D_EU4" path neede if not in "Default Vehicles") or aggregated name (PC, HDV, BUS, TW) by FleetMix calculation
        //List<double> / double   |  [s]    |   Time                    | Time signal
        //List<double> / double   |  [m/s]  |   Velocity                | Velocity signal
        //double                  |  [m/s^2]|   acc                     | Acceleration (ONLY NEDDED BY SINGLE CALCULATION)
        //List<double> / double   |  [%]    |   Gradient                | Gradient of the route
        //out List<VehicleResult> |  [-]    |   VehicleResultsOrg       | Returned result list
        //bool                    |  [-]    |   fleetMix = false        | Optional parameter if fleetMix should be calculate
        //string                  |  [-]    |   CommentPref = "c"       | Optional parameter for comment prefix

        //********************************* OUPUT: VehicleResultsOrg **********************
        //***  DATATYP            |  UNIT   |  VARIBLE     |      Description  ***
        //double                  |  [s]    |   time       | Time
        //double                  |  [m/s]  |   speed      | Velocity
        //double                  |  [kW]   |   power      | Calculated power at the engine (ICE for conventional and HEV vehicles, electric engine for BEVs) including engine inertia and auxiliaries; not limited for engine fullload and braking limitations
        //double                  |  [kW]   |   P_pos      | Positive engine power limited with engine rated power
        //double                  |  [-]    |   pNormRated | Engine power normalised with rated engine power and limited with the power range (fullload and drag) as specified in the characteristic curve for fuel consumption
        //double                  |  [-]    |   pNormDrive | Engine power normalised with "P_drive" and limited with the power range (fullload and drag) as specified in the characteristic curve for emissions
        //double                  |  [m/s^2]|   acc        | Caclulated/given acceleration
        //double                  |  [g/h]  |   fc         | Calculated fuel consumption (Different unit for BEV vehicles)
        //                        |  [kWh/h]|              | 
        //double                  |  [g/h]  |   cO2        | Calculated CO2 emissions
        //double                  |  [g/h]  |   NOx        | Calculated NOx emissions
        //double                  |  [g/h]  |   HC         | Calculated HC emissions
        //double                  |  [g/h]  |   PM         | Calculated PM emissions
        //double                  |  [g/h]  |   CO         | Calculated CO emissions

        #region calculate
        //Calculate data from array
        public bool CALC_Array(List<string> DataFiles, 
                               List<double> Time, 
                               List<double> Velocity,
                               List<double> Gradient,
                               out List<VehicleResult> VehicleResultsOrg,
                               bool fleetMix = false,
                               string CommentPref = "c")
        {
            //Declaration
            int i;
            double acc;
            List<VehicleResult>  _VehicleResult = new List<VehicleResult>();

            //Initialisation
            Helper.ErrMsg = null;

            //Borrow
            Helper.CommentPrefix = CommentPref;
            _DataPath = new List<string>();
            //Set path by normal calculation (on given) and set path by fleetmix (on Default Vehicles) calculation
            for (i = 0; i < DataFiles.Count; i++)
            {
                if ((DataFiles[i].LastIndexOf(@"\")) >= 0)
                {
                    _DataPath.Add(DataFiles[i]);
                }
                else
                {
                    //_DataPath.Add(Assembly.GetExecutingAssembly().Location.Substring(0, Assembly.GetExecutingAssembly().Location.LastIndexOf(@"\")) + @"\Default Vehicles\" + Helper.PHEMDataV);
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    i += 1;
                }
            }

            //Read the vehicle and emission data
            #if FLEET
            if (fleetMix)
            {
                //Set the vehicle class
                Helper.gClass = _DataPath[0];

                //Generate the class
                DataInput = new CEPHandler();

                //Read the FleetShares
                if (!DataInput.ReadFleetShares(DataFiles[1], Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
                //Read the vehicle and emission data
                if (!DataInput.GetFleetCEP(_DataPath, DataFiles[0], Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
            }
            else
            #endif
            {
                //Get vehicle string
                if (!Helper.setclass(DataFiles[0]))
                {
                    VehicleResultsOrg = null;
                    return false;
                }

                //Generate the class
                DataInput = new CEPHandler();

                //Read the vehicle and emission data
                if (!DataInput.GetCEP(_DataPath, Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
            }

            //Calculate emissions per second
            for (i = 1; i <= Time.Count - 1; i++)
            {
                //Calculate the acceleration
                acc = (Velocity[i] - Velocity[i - 1]) / (Time[i] - Time[i - 1]);
                
                //Calculate and save the data in the List
                _VehicleResult.Add(PHEMLight.CreateVehicleStateData(Helper,
                                                                    DataInput.CEPS[Helper.gClass],
                                                                    Time[i - 1],
                                                                    Velocity[i - 1],
                                                                    acc,
                                                                    Gradient[i - 1]));
                if (Helper.ErrMsg != null)
                {
                    VehicleResultsOrg = null;
                    return false;
                }   
            }
            VehicleResultsOrg = _VehicleResult;
            return true;
        }

        //Calculate single data
        public bool CALC_Single(List<string> DataFiles,
                                double Time,
                                double Velocity,
                                double acc,
                                double Gradient,
                                out List<VehicleResult> VehicleResultsOrg,
                                bool fleetMix = false,
                                string CommentPref = "c")
        {
            //Declaration
            List<VehicleResult> _VehicleResult = new List<VehicleResult>();
            VehicleResultsOrg = _VehicleResult;

            //Borrow
            Helper.CommentPrefix = CommentPref;
            _DataPath = new List<string>();
            //Set path by normal calculation (on given) and set path by fleetmix (on Fleetshare file) calculation
            for (int i = 0; i < DataFiles.Count; i++)
            {
                if ((DataFiles[i].LastIndexOf(@"\")) >= 0)
                {
                    _DataPath.Add(DataFiles[i]);
                }
                else
                {
                    //_DataPath.Add(Assembly.GetExecutingAssembly().Location.Substring(0, Assembly.GetExecutingAssembly().Location.LastIndexOf(@"\")) + @"\Default Vehicles\" + Helper.PHEMDataV);
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    _DataPath.Add(DataFiles[i + 1].Substring(0, DataFiles[i + 1].LastIndexOf(@"\")));
                    i += 1;
                }
            }

            //Read the vehicle and emission data
            #if FLEET
            if (fleetMix)
            {
                //Set the vehicle class
                Helper.gClass = "AggClass_" + DataFiles[0];

                //Generate the class
                DataInput = new CEPHandler();

                //Read the FleetShares
                if (!DataInput.ReadFleetShares(DataFiles[1], Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
                //Read the vehicle and emission data
                if (!DataInput.GetFleetCEP(_DataPath, DataFiles[0], Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
            }
            else
            #endif
            {
                //Get vehicle string
                if (!Helper.setclass(DataFiles[0]))
                {
                    VehicleResultsOrg = null;
                    return false;
                }

                //Generate the class
                DataInput = new CEPHandler();

                //Read the vehicle and emission data
                if (!DataInput.GetCEP(_DataPath, Helper))
                {
                    VehicleResultsOrg = null;
                    return false;
                }
            }

            //Calculate and save the data in the List
            _VehicleResult.Add(PHEMLight.CreateVehicleStateData(Helper,
                                                                DataInput.CEPS[Helper.gClass],
                                                                Time,
                                                                Velocity,
                                                                acc,
                                                                Gradient));
            VehicleResultsOrg = _VehicleResult;
            return true;
        }
        #endregion

        #region ExportData
        public bool ExportData(string path, string vehicle, List<VehicleResult> _VehicleResult)
        {
            if (path == null || vehicle == null || _VehicleResult == null || _VehicleResult.Count == 0) return false;

            //Write head
            StringBuilder allLines = new StringBuilder();
            string lineEnding = "\r\n";

            allLines.AppendLine("Vehicletype: ," + vehicle);
            allLines.AppendLine("Time, Speed, Gradient, Accelaration, Engine power raw, P_pos, P_norm_rated, P_norm_drive, FC, Electric Power, CO2, NOx, CO, HC, PM");
            allLines.AppendLine("[s], [m/s], [%], [m/s^2], [kW], [kW], [-], [-], [g/h], [kWh/h], [g/h], [g/h], [g/h], [g/h], [g/h]");

            //Write data
            foreach (VehicleResult Result in _VehicleResult)
            {
                allLines.Append(Result.Time.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.Speed.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.Grad.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.Accelaration.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.Power.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.PPos.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.PNormRated.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.PNormDrive.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.FC.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.FCel.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.CO2.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.NOx.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.CO.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.HC.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
                allLines.Append(Result.EmissionData.PM.ToString("0.0000", CultureInfo.InvariantCulture) + lineEnding);
            }

            // Write the string to a file.
            if (path.IndexOf(".", 0) < 0)
            {
                path = path + ".sta";
            }
            try
            {
                StreamWriter file = new StreamWriter(path);
                file.WriteLine(allLines);
                file.Close();
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public bool ExportSumData(string path, string vehicle, string cycle, VehicleResult _VehicleResult)
        {
            if (path == null || vehicle == null || cycle == null || _VehicleResult == null) return false;
            StringBuilder allLines = new StringBuilder();

            if (path.IndexOf(".", 0) < 0)
            {
                path = path + ".erg";
            }

            if (!File.Exists(path))
            {
                //Write head
                allLines.AppendLine("PHEMLight Results");
                allLines.AppendLine("");
                allLines.AppendLine("Vehicle, Cycle, Time, Speed, Gradient, Accelaration, Engine power raw, P_pos, P_norm_rated, P_norm_drive, FC, Electric Power, CO2, NOx, CO, HC, PM");
                allLines.AppendLine("[-], [-], [s], [km/h], [%], [m/s^2], [kW], [kW], [-], [-], [g/km], [kWh/km], [g/km], [g/km], [g/km], [g/km], [g/km]");
            }

            //Write data
            allLines.Append(vehicle + ",");
            allLines.Append(cycle + ",");
            allLines.Append(_VehicleResult.Time.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.Speed.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.Grad.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.Accelaration.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.Power.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.PPos.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.PNormRated.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.PNormDrive.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.FC.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.FCel.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.CO2.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.NOx.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.CO.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.HC.ToString("0.0000", CultureInfo.InvariantCulture) + ",");
            allLines.Append(_VehicleResult.EmissionData.PM.ToString("0.0000", CultureInfo.InvariantCulture));

            // Write the string to a file.
            try
            {
                StreamWriter file = new StreamWriter(path, true);
                file.WriteLine(allLines);
                file.Close();
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public VehicleResult GenerateSumData(List<VehicleResult> _VehicleResult)
        {
            //Declaration
            double sum_time = 0;
            double sum_speed = 0;
            double sum_grad = 0;
            double sum_power = 0;
            double sum_pPos = 0;
            double sum_pNormRated = 0;
            double sum_pNormDrive = 0;
            double sum_acc = 0;
            double sum_fc = 0;
            double sum_fcel = 0;
            double sum_cO2 = 0;
            double sum_nOx = 0;
            double sum_hC = 0;
            double sum_pM = 0;
            double sum_cO = 0;

            if (_VehicleResult == null || _VehicleResult.Count == 0) return new VehicleResult(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            //Write data
            foreach (VehicleResult Result in _VehicleResult)
            {
                sum_speed += Result.Speed * 3.6;
                sum_power += Result.Power;
                if (Result.PPos > 0) { sum_pPos += Result.PPos; }
                sum_grad += Result.Grad;
                sum_pNormRated += Result.PNormRated;
                sum_pNormDrive += Result.PNormDrive;
                sum_acc += Result.Accelaration;
                sum_fcel += Result.EmissionData.FCel;
                sum_fc += Result.EmissionData.FC;
                sum_cO2 += Result.EmissionData.CO2;
                sum_nOx += Result.EmissionData.NOx;
                sum_hC += Result.EmissionData.HC;
                sum_pM += Result.EmissionData.PM;
                sum_cO += Result.EmissionData.CO;
            }

            //Build average
            sum_time = _VehicleResult[_VehicleResult.Count - 1].Time - _VehicleResult[0].Time;
            sum_power /= _VehicleResult.Count;
            sum_pPos /= _VehicleResult.Count;
            sum_grad /= _VehicleResult.Count;
            sum_pNormRated /= _VehicleResult.Count;
            sum_pNormDrive /= _VehicleResult.Count;
            sum_acc /= _VehicleResult.Count;
            if (sum_speed > 0)
            {
                sum_fc /= sum_speed;
                sum_fcel /= sum_speed;
                sum_cO2 /= sum_speed;
                sum_nOx /= sum_speed;
                sum_hC /= sum_speed;
                sum_pM /= sum_speed;
                sum_cO /= sum_speed;
            }
            else
            {
                sum_fc = 0;
                sum_fcel = 0;
                sum_cO2 = 0;
                sum_nOx = 0;
                sum_hC = 0;
                sum_pM = 0;
                sum_cO = 0;
            }
            sum_speed /= _VehicleResult.Count;

            return new VehicleResult(sum_time,
                                     sum_speed,
                                     sum_grad,
                                     sum_power,
                                     sum_pPos,
                                     sum_pNormRated,
                                     sum_pNormDrive,
                                     sum_acc,
                                     sum_fc,
                                     sum_fcel,
                                     sum_cO2,
                                     sum_nOx,
                                     sum_hC,
                                     sum_pM,
                                     sum_cO);
        }
        #endregion
    }

    //Calculation
    class PHEMLight
    {
        #region CreateVehicleStateData
        static public VehicleResult CreateVehicleStateData(Helpers Helper,
                                                           CEP currCep,
                                                           double time,
                                                           double inputSpeed,
                                                           double inputAcc,
                                                           double Gradient = 0)
        {
            //Declaration
            double speed = Math.Max(inputSpeed, 0);
            double acc;
            double P_pos;

            //Speed/Acceleration limitation
            if (speed == 0)
                acc = 0;
            else
                acc = Math.Min(inputAcc, currCep.GetMaxAccel(speed, Gradient));

            //Calculate the power
            double power = currCep.CalcPower(speed, acc, Gradient);
            double P_eng = currCep.CalcEngPower(power);

            //Power limitation
            if (P_eng >= 0) 
                P_pos = power;
            else 
                P_pos = 0;

            //Calculate the result values (BEV)
            if (Helper.tClass == Constants.strBEV)
            {
                return new VehicleResult(time,
                                         speed,
                                         Gradient,
                                         power,
                                         P_pos,
                                         P_eng / currCep.RatedPower,
                                         P_eng / currCep.DrivingPower,
                                         acc,
                                         0,
                                         currCep.GetEmission("FC", power, speed, Helper),
                                         0,
                                         0,
                                         0,
                                         0,
                                         0);
            }

            //Calculate the decel costing
            double decelCoast = currCep.GetDecelCoast(speed, acc, Gradient);

            //Calculate the result values (Zero emissions by costing, Idling emissions by v <= 0.5m/s²)
            if (acc >= decelCoast || speed <= Constants.ZERO_SPEED_ACCURACY)
            {
                return new VehicleResult(time,
                                         speed,
                                         Gradient,
                                         power,
                                         P_pos,
                                         P_eng / currCep.RatedPower,
                                         P_eng / currCep.DrivingPower,
                                         acc,
                                         currCep.GetEmission("FC", power, speed, Helper),
                                         0,
                                         currCep.GetCO2Emission(currCep.GetEmission("FC", power, speed, Helper), currCep.GetEmission("CO", power, speed, Helper), currCep.GetEmission("HC", power, speed, Helper), Helper),
                                         currCep.GetEmission("NOx", power, speed, Helper),
                                         currCep.GetEmission("HC", power, speed, Helper),
                                         currCep.GetEmission("PM", power, speed, Helper),
                                         currCep.GetEmission("CO", power, speed, Helper));
            }
            else
            {
                return new VehicleResult(time,
                                         speed,
                                         Gradient,
                                         power,
                                         P_pos,
                                         power / currCep.RatedPower,
                                         power / currCep.DrivingPower,
                                         acc,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0);
            }
        }
        #endregion
    }
}
