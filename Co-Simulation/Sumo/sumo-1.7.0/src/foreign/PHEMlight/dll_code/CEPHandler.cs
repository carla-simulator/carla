#define FLEET
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Globalization;

namespace PHEMlightdll
{
    public class CEPHandler
    {
        //--------------------------------------------------------------------------------------------------
        // Constructors
        //--------------------------------------------------------------------------------------------------

        #region Constructor
        public CEPHandler()
        {
            _ceps = new Dictionary<string, CEP>();
        }
        #endregion

        //--------------------------------------------------------------------------------------------------
        // Members 
        //--------------------------------------------------------------------------------------------------

        #region CEPS
        private Dictionary<string, CEP> _ceps;
        public Dictionary<string, CEP> CEPS
        {
            get
            {
                return _ceps;
            }
        }
        #endregion

        #if FLEET
        #region FleetShares
        private Dictionary<string, Dictionary<string, double>> _fleetShares;
        public Dictionary<string, Dictionary<string, double>> FleetShares
        {
            get
            {
                return _fleetShares;
            }
        }
        #endregion
        #endif

        //--------------------------------------------------------------------------------------------------
        // Methods 
        //--------------------------------------------------------------------------------------------------

        #region GetCEP
        public bool GetCEP(string DataPath, Helpers Helper)
        {
            if (!CEPS.ContainsKey(Helper.gClass))
            {
                if (!Load(DataPath, Helper))
                {
                    return false;
                }
            }
            return true;
        }
        #endregion

        #if FLEET
        #region GetFleetCEP
        public bool GetFleetCEP(string DataPath, string AggClass, Helpers Helper)
        {
            if (!CEPS.ContainsKey(Helper.gClass))
            {
                if (Constants.AGGREGATED_VEHICLECLASSES.Contains(AggClass))
                {
                    List<CEP> weightedCEPS = new List<CEP>();

                    if (FleetShares.ContainsKey(AggClass))
                    {
                        foreach (string aggVehClass in FleetShares[AggClass].Keys)
                        {
                            if (!Helper.setclass(aggVehClass))
                            {
                                return false;
                            }
                            if (!CEPS.ContainsKey(aggVehClass) && !Load(DataPath, Helper))
                            {
                                return false;
                            }
                            weightedCEPS.Add(CEPS[aggVehClass] * FleetShares[AggClass][aggVehClass]);
                        }
                        _ceps.Add(AggClass, CEP.AddRangeCeps(weightedCEPS.ToArray(), Helper));

                        //Set the vehicle class back
                        Helper.gClass = AggClass;
                    }
                    else
                    {
                        Helper.ErrMsg = "The aggregated vehicle class (" + AggClass + ") is not available in the FleetShare file!";
                        return false;
                    }
                }
                else
                {
                    Helper.ErrMsg = "The aggregated vehicle class (" + AggClass + ") is a unknown class!";
                    return false;
                }
            }
            return true;
        }
        #endregion
        #endif

        //--------------------------------------------------------------------------------------------------
        // Methods 
        //--------------------------------------------------------------------------------------------------

        #region Load
        private bool Load(string DataPath, Helpers Helper)
        {
            //Deklaration
            // get string identifier for PHEM emission class
            string emissionRep = Helper.gClass.ToString();

            // to hold everything.
            List<List<double>> matrixSpeedInertiaTable;
            List<List<double>> normedTragTableSpeedInertiaTable;
            List<List<double>> matrixFC;
            List<List<double>> matrixPollutants;
            List<double> idlingValuesFC;
            List<double> idlingValuesPollutants;
            List<string> headerFC;
            List<string> headerPollutants;

            double vehicleMass;
            double vehicleLoading;
            double vehicleMassRot;
            double crosssectionalArea;
            double cwValue;
            double f0;
            double f1;
            double f2;
            double f3;
            double f4;
            double axleRatio;
            List<double> transmissionGearRatios;
            double auxPower;
            double ratedPower;
            double engineIdlingSpeed;
            double engineRatedSpeed;
            double effectiveWhellDiameter;
            string vehicleMassType;
            string vehicleFuelType;
            double pNormV0;
            double pNormP0;
            double pNormV1;
            double pNormP1;

            if (!ReadVehicleFile(DataPath,
                                 emissionRep,
                                 Helper,
                                 out vehicleMass,
                                 out vehicleLoading,
                                 out vehicleMassRot,
                                 out crosssectionalArea,
                                 out cwValue,
                                 out f0,
                                 out f1,
                                 out f2,
                                 out f3,
                                 out f4,
                                 out axleRatio,
                                 out auxPower,
                                 out ratedPower,
                                 out engineIdlingSpeed,
                                 out engineRatedSpeed,
                                 out effectiveWhellDiameter,
                                 out transmissionGearRatios,
                                 out vehicleMassType,
                                 out vehicleFuelType,
                                 out pNormV0,
                                 out pNormP0,
                                 out pNormV1,
                                 out pNormP1,
                                 out matrixSpeedInertiaTable,
                                 out normedTragTableSpeedInertiaTable))
                return false;

            if (!ReadEmissionData(true, DataPath, emissionRep, Helper, out headerFC, out matrixFC, out idlingValuesFC))
                return false;

            if (!ReadEmissionData(false, DataPath, emissionRep, Helper, out headerPollutants, out matrixPollutants, out idlingValuesPollutants))
                return false;

            _ceps.Add(Helper.gClass, new CEP(vehicleMassType == Constants.HeavyVehicle,
                                                   vehicleMass,
                                                   vehicleLoading,
                                                   vehicleMassRot,
                                                   crosssectionalArea,
                                                   cwValue,
                                                   f0,
                                                   f1,
                                                   f2,
                                                   f3,
                                                   f4,
                                                   axleRatio,
                                                   transmissionGearRatios,
                                                   auxPower,
                                                   ratedPower,
                                                   engineIdlingSpeed,
                                                   engineRatedSpeed,
                                                   effectiveWhellDiameter,
                                                   pNormV0,
                                                   pNormP0,
                                                   pNormV1,
                                                   pNormP1,
                                                   vehicleFuelType,
                                                   matrixFC,
                                                   headerPollutants,
                                                   matrixPollutants,
                                                   matrixSpeedInertiaTable,
                                                   normedTragTableSpeedInertiaTable,
                                                   idlingValuesFC.First(),
                                                   idlingValuesPollutants));

            return true;
        }
        #endregion

        #region ReadVehicleFile
        private bool ReadVehicleFile(string DataPath,
                                     string emissionClass,
                                     Helpers Helper,
                                     out double vehicleMass,
                                     out double vehicleLoading,
                                     out double vehicleMassRot,
                                     out double crossArea,
                                     out double cWValue,
                                     out double f0,
                                     out double f1,
                                     out double f2,
                                     out double f3,
                                     out double f4,
                                     out double axleRatio,
                                     out double auxPower,
                                     out double ratedPower,
                                     out double engineIdlingSpeed,
                                     out double engineRatedSpeed,
                                     out double effectiveWheelDiameter,
                                     out List<double> transmissionGearRatios,
                                     out string vehicleMassType,
                                     out string vehicleFuelType,
                                     out double pNormV0,
                                     out double pNormP0,
                                     out double pNormV1,
                                     out double pNormP1,
                                     out List<List<double>> matrixSpeedInertiaTable,
                                     out List<List<double>> normedDragTable)
        {
            vehicleMass = 0;
            vehicleLoading = 0;
            vehicleMassRot = 0;
            crossArea = 0;
            cWValue = 0;
            f0 = 0;
            f1 = 0;
            f2 = 0;
            f3 = 0;
            f4 = 0;
            axleRatio = 0;
            ratedPower = 0;
            auxPower = 0;
            engineIdlingSpeed = 0;
            engineRatedSpeed = 0;
            effectiveWheelDiameter = 0;
            vehicleMassType = "";
            vehicleFuelType = "";
            pNormV0 = 0;
            pNormP0 = 0;
            pNormV1 = 0;
            pNormP1 = 0;
            transmissionGearRatios = new List<double>();
            matrixSpeedInertiaTable = new List<List<double>>();
            normedDragTable = new List<List<double>>();
            string line;
            string cell;
            int dataCount = 0;

            //Open file
            string path = DataPath + @"\" + emissionClass + ".PHEMLight.veh";
            if (!File.Exists(@path))
            {
                Helper.ErrMsg = "File do not exist! (" + path + ")";
                return false;
            }
            StreamReader vehicleReader = File.OpenText(@path);

            // skip header
            ReadLine(vehicleReader);

            while ((line = ReadLine(vehicleReader)) != null && dataCount <= 49)
            {
                if (line.Substring(0, 1) == Helper.CommentPrefix)
                {
                    continue;
                }
                else
                {
                    dataCount++;
                }

                cell = split(line, ',')[0];

                // reading Mass
                if (dataCount == 1)
                    vehicleMass = todouble(cell);

                // reading vehicle loading
                if (dataCount == 2)
                    vehicleLoading = todouble(cell);

                // reading cWValue
                if (dataCount == 3)
                    cWValue = todouble(cell);

                // reading crossectional area
                if (dataCount == 4)
                    crossArea = todouble(cell);

                // reading vehicle mass rotational
                if (dataCount == 7)
                    vehicleMassRot = todouble(cell);

                // reading rated power
                if (dataCount == 9)
                    auxPower = todouble(cell);

                // reading rated power
                if (dataCount == 10)
                    ratedPower = todouble(cell);

                // reading engine rated speed
                if (dataCount == 11)
                    engineRatedSpeed = todouble(cell);

                // reading engine idling speed
                if (dataCount == 12)
                    engineIdlingSpeed = todouble(cell);

                // reading f0
                if (dataCount == 14)
                    f0 = todouble(cell);

                // reading f1
                if (dataCount == 15)
                    f1 = todouble(cell);

                // reading f2
                if (dataCount == 16)
                    f2 = todouble(cell);

                // reading f3
                if (dataCount == 17)
                    f3 = todouble(cell);

                // reading f4
                if (dataCount == 18)
                    f4 = todouble(cell);

                // reading axleRatio
                if (dataCount == 21)
                    axleRatio = todouble(cell);

                // reading effective wheel diameter
                if (dataCount == 22)
                    effectiveWheelDiameter = todouble(cell);

                if (dataCount >= 23 && dataCount <= 40)
                    transmissionGearRatios.Add(todouble(cell));

                // reading vehicleMassType
                if (dataCount == 45)
                    vehicleMassType = cell;

                // reading vehicleFuelType
                if (dataCount == 46)
                    vehicleFuelType = cell;

                // reading pNormV0
                if (dataCount == 47)
                    pNormV0 = todouble(cell);

                // reading pNormP0
                if (dataCount == 48)
                    pNormP0 = todouble(cell);

                // reading pNormV1
                if (dataCount == 49)
                    pNormV1 = todouble(cell);

                // reading pNormP1
                if (dataCount == 50)
                    pNormP1 = todouble(cell);
            }

            while ((line = ReadLine(vehicleReader)) != null && line.Substring(0, 1) != Helper.CommentPrefix)
            {
                if (line.Substring(0, 1) == Helper.CommentPrefix)
                    continue;

                matrixSpeedInertiaTable.Add(todoubleList(split(line, ',')));
            }

            while ((line = ReadLine(vehicleReader)) != null)
            {
                if (line.Substring(0, 1) == Helper.CommentPrefix)
                    continue;

                normedDragTable.Add(todoubleList(split(line, ',')));
            }

            vehicleReader.Close();
            return true;
        }
        #endregion

        #region ReadEmissionData
        private bool ReadEmissionData(bool readFC,
                                      string DataPath,
                                      string emissionClass,
                                      Helpers Helper,
                                      out List<string> header,
                                      out List<List<double>> matrix,
                                      out List<double> idlingValues)
        {
            // declare file stream
            string line;
            header = new List<string>();
            matrix = new List<List<double>>();
            idlingValues = new List<double>();

            string pollutantExtension = "";
            if (readFC)
                pollutantExtension += "_FC";

            string path = DataPath + @"\" + emissionClass + pollutantExtension + ".csv";
            if (!File.Exists(path))
            {
                Helper.ErrMsg = "File do not exist! (" + path + ")";
                return false;
            }
            StreamReader fileReader = File.OpenText(@path);

            // read header line for pollutant identifiers
            if ((line = ReadLine(fileReader)) != null)
            {
                List<string> entries = split(line, ',');
                // skip first entry "Pe"
                for (int i = 1; i < entries.Count; i++)
                {
                    header.Add(entries[i]);
                }
            }

            // skip units
            ReadLine(fileReader);

            // skip comment
            ReadLine(fileReader);

            //readIdlingValues
            line = ReadLine(fileReader);

            List<string> stringIdlings = split(line, ',').ToList();
            stringIdlings.RemoveAt(0);

            idlingValues = todoubleList(stringIdlings);

            while ((line = ReadLine(fileReader)) != null)
            {
                matrix.Add(todoubleList(split(line, ',')));
            }
            fileReader.Close();
            return true;
        }
        #endregion

        #if FLEET
        #region ReadFleetShares
        public bool ReadFleetShares(string DataPath, Helpers Helper)
        {
            //Declaration
            string line;
            string path = DataPath + @"\FleetShares.csv";
            if (!File.Exists(@path))
            {
                Helper.ErrMsg = "FleetShares file does not exist! (" + path + ")";
                return false;
            }
            StreamReader shareReader = File.OpenText(@path);

            _fleetShares = new Dictionary<string, Dictionary<string, double>>();

            while ((line = ReadLine(shareReader)) != null)
            {
                if (line.Substring(0, 1) == Helper.CommentPrefix)
                    continue;

                List<string> splitLine = split(line, ',');
                string aggregateClass = splitLine[0];

                if (!FleetShares.ContainsKey(aggregateClass))
                    FleetShares.Add(aggregateClass, new Dictionary<string, double>());

                string subClass = splitLine[1];

                if (!FleetShares[aggregateClass].ContainsKey(subClass))
                    FleetShares[aggregateClass].Add(subClass, todouble(splitLine[2]));
            }
            return true;
        }
        #endregion
        #endif

        //--------------------------------------------------------------------------------------------------
        // Functions 
        //--------------------------------------------------------------------------------------------------

        #region Functions
        //Split the string
        private List<string> split(string s, char delim)
        {
            return s.Split(delim).ToList();
        }

        //Convert string to double
        private double todouble(string s)
        {
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        //Convert string to double list
        private List<double> todoubleList(List<string> s)
        {
            return s.Select(p => todouble(p)).Cast<double>().ToList();
        }

        //Read a line from file
        private string ReadLine(StreamReader s)
        {
            return s.ReadLine();
        }
        #endregion
    }
}
