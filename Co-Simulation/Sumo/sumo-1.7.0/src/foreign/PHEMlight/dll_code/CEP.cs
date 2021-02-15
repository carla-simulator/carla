#define FLEET
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PHEMlightdll
{
    public class CEP
    {
        //--------------------------------------------------------------------------------------------------
        // Constructors
        //--------------------------------------------------------------------------------------------------      

        #region Constructor
        public CEP(bool heavyVehicle,
                    double vehicleMass,
                    double vehicleLoading,
                    double vehicleMassRot,
                    double crossArea,
                    double cWValue,
                    double f0,
                    double f1,
                    double f2,
                    double f3,
                    double f4,
                    double axleRatio,
                    List<double> transmissionGearRatios,
                    double auxPower,
                    double ratedPower,
                    double engineIdlingSpeed,
                    double engineRatedSpeed,
                    double effictiveWheelDiameter,
                    double pNormV0,
                    double pNormP0,
                    double pNormV1,
                    double pNormP1,
                    string vehicelFuelType,
                    List<List<double>> matrixFC,
                    List<string> headerLinePollutants,
                    List<List<double>> matrixPollutants,
                    List<List<double>> matrixSpeedRotational,
                    List<List<double>> normedDragTable,
                    double idlingFC,
                    List<double> idlingPollutants)
        {
            _resistanceF0 = f0;
            _resistanceF1 = f1;
            _resistanceF2 = f2;
            _resistanceF3 = f3;
            _resistanceF4 = f4;
            _cWValue = cWValue;
            _crossSectionalArea = crossArea;
            _massVehicle = vehicleMass;
            _vehicleLoading = vehicleLoading;
            _vehicleMassRot = vehicleMassRot;
            _ratedPower = ratedPower;
            _engineIdlingSpeed = engineIdlingSpeed;
            _engineRatedSpeed = engineRatedSpeed;
            _effectiveWheelDiameter = effictiveWheelDiameter;
            _heavyVehicle = heavyVehicle;
            _fuelType = vehicelFuelType;
            _axleRatio = axleRatio;
            _auxPower = auxPower;

            _pNormV0 = pNormV0 / 3.6;
            _pNormP0 = pNormP0;
            _pNormV1 = pNormV1 / 3.6;
            _pNormP1 = pNormP1;

            List<string> pollutantIdentifier = new List<string>();
            List<List<double>> pollutantMeasures = new List<List<double>>();
            List<List<double>> normalizedPollutantMeasures = new List<List<double>>();

            // init pollutant identifiers
            for (int i = 0; i < headerLinePollutants.Count; i++)
            {
                pollutantIdentifier.Add(headerLinePollutants[i]);
            }

            // initialize measures
            for (int i = 0; i < headerLinePollutants.Count; i++)
            {
                pollutantMeasures.Add(new List<double>());
                normalizedPollutantMeasures.Add(new List<double>());
            }

            // looping through matrix and assigning values for speed rotational table
            _speedCurveRotational = new List<double>();
            _speedPatternRotational = new List<double>();
            _gearTransmissionCurve = new List<double>();
            for (int i = 0; i < matrixSpeedRotational.Count; i++)
            {
                if (matrixSpeedRotational[i].Count != 3)
                    return;

                _speedPatternRotational.Add(matrixSpeedRotational[i][0] / 3.6);
                _gearTransmissionCurve.Add(matrixSpeedRotational[i][1]);
                _speedCurveRotational.Add(matrixSpeedRotational[i][2]);
            }

            // looping through matrix and assigning values for drag table
            _nNormTable = new List<double>();
            _dragNormTable = new List<double>();
            for (int i = 0; i < normedDragTable.Count; i++)
            {
                if (normedDragTable[i].Count != 2)
                    return;

                _nNormTable.Add(normedDragTable[i][0]);
                _dragNormTable.Add(normedDragTable[i][1]);
            }

            // looping through matrix and assigning values for Fuel consumption
            _cepCurveFC = new List<double>();
            _normedCepCurveFC = new List<double>();
            _powerPatternFC = new List<double>();
            _normalizedPowerPatternFC = new List<double>();
            for (int i = 0; i < matrixFC.Count; i++)
            {
                if (matrixFC[i].Count != 2)
                    return;

                _powerPatternFC.Add(matrixFC[i][0] * _ratedPower);
                _normalizedPowerPatternFC.Add(matrixFC[i][0]);
                _cepCurveFC.Add(matrixFC[i][1] * _ratedPower);
                _normedCepCurveFC.Add(matrixFC[i][1]);

            }

            _powerPatternPollutants = new List<double>();

            double pollutantMultiplyer = 1;

            _drivingPower = _normalizingPower = CalcPower(Constants.NORMALIZING_SPEED, Constants.NORMALIZING_ACCELARATION, 0);

            // looping through matrix and assigning values for pollutants
            if (heavyVehicle)
            {
                _normalizingPower = _ratedPower;
                _normalizingType = NormalizingType.RatedPower;
                pollutantMultiplyer = _ratedPower;
            }
            else
            {
                _normalizingPower = _drivingPower;
                _normalizingType = NormalizingType.DrivingPower;
            }

            _normailzedPowerPatternPollutants = new List<double>();

            _cepNormalizedCurvePollutants = new Dictionary<string, List<double>>();

            int headerCount = headerLinePollutants.Count;
            for (int i = 0; i < matrixPollutants.Count; i++)
            {
                for (int j = 0; j < matrixPollutants[i].Count; j++)
                {
                    if (matrixPollutants[i].Count != headerCount + 1)
                        return;

                    if (j == 0)
                    {
                        _normailzedPowerPatternPollutants.Add(matrixPollutants[i][j]);
                        _powerPatternPollutants.Add(matrixPollutants[i][j] * NormalizingPower);
                    }
                    else
                    {
                        pollutantMeasures[j - 1].Add(matrixPollutants[i][j] * pollutantMultiplyer);
                        normalizedPollutantMeasures[j - 1].Add(matrixPollutants[i][j]);
                    }
                }
            }

            _cepCurvePollutants = new Dictionary<string, List<double>>();
            _idlingValuesPollutants = new Dictionary<string, double>();

            for (int i = 0; i < headerLinePollutants.Count; i++)
            {
                _cepCurvePollutants.Add(pollutantIdentifier[i], pollutantMeasures[i]);
                _cepNormalizedCurvePollutants.Add(pollutantIdentifier[i], normalizedPollutantMeasures[i]);
                _idlingValuesPollutants.Add(pollutantIdentifier[i], idlingPollutants[i] * pollutantMultiplyer);
            }

            _idlingValueFC = idlingFC * _ratedPower;
        }
        #endregion

        #if FLEET
        #region ConstrutorForFleetmix
        private CEP(bool heavyVehicle,
                       double vehicleMass,
                       double vehicleLoading,
                       double vehicleMassRot,
                       double crossArea,
                       double cWValue,
                       double f0,
                       double f1,
                       double f2,
                       double f3,
                       double f4,
                       double axleRatio,
                       double auxPower,
                       double ratedPower,
                       double engineIdlingSpeed,
                       double engineRatedSpeed,
                       double effictiveWheelDiameter,
                       double pNormV0,
                       double pNormP0,
                       double pNormV1,
                       double pNormP1)
        {
            _resistanceF0 = f0;
            _resistanceF1 = f1;
            _resistanceF2 = f2;
            _resistanceF3 = f3;
            _resistanceF4 = f4;
            _cWValue = cWValue;
            _crossSectionalArea = crossArea;
            _massVehicle = vehicleMass;
            _vehicleLoading = vehicleLoading;
            _vehicleMassRot = vehicleMassRot;
            _ratedPower = ratedPower;
            _engineIdlingSpeed = engineIdlingSpeed;
            _engineRatedSpeed = engineRatedSpeed;
            _effectiveWheelDiameter = effictiveWheelDiameter;

            _axleRatio = axleRatio;
            _auxPower = auxPower;

            _pNormV0 = pNormV0 / 3.6;
            _pNormP0 = pNormP0;
            _pNormV1 = pNormV1 / 3.6;
            _pNormP1 = pNormP1;

            _heavyVehicle = heavyVehicle;

        }
        #endregion
        #endif

        //--------------------------------------------------------------------------------------------------
        // Members 
        //--------------------------------------------------------------------------------------------------

        #region HeavyVehicle
        private bool _heavyVehicle;
        public bool HeavyVehicle
        {
            get
            {
                return _heavyVehicle;
            }
        }
        #endregion

        #region FuelType
        private string _fuelType;
        public string FuelType
        {
            get
            {
                return _fuelType;
            }
        }
        #endregion

        #region NormalizingType
        public enum NormalizingType
        {
            RatedPower,
            DrivingPower
        }
        private NormalizingType _normalizingType;
        public NormalizingType NormalizingTypeX
        {
            get
            {
                return _normalizingType;
            }
        }
        #endregion

        #region RatedPower
        private double _ratedPower;
        public double RatedPower
        {
            get
            {
                return _ratedPower;
            }
            set
            {
                _ratedPower = value;
            }
        }
        #endregion

        #region NormalizingPower
        private double _normalizingPower;
        public double NormalizingPower
        {
            get
            {
                return _normalizingPower;
            }
        }
        #endregion

        #region DrivingPower
        private double _drivingPower;
        public double DrivingPower
        {
            get
            {
                return _drivingPower;
            }
            set
            {
                _drivingPower = value;
            }
        }

        #endregion

        #region Private Members

        protected double _massVehicle;
        protected double _vehicleLoading;
        protected double _vehicleMassRot;
        protected double _crossSectionalArea;
        protected double _cWValue;
        protected double _resistanceF0;
        protected double _resistanceF1;
        protected double _resistanceF2;
        protected double _resistanceF3;
        protected double _resistanceF4;
        protected double _axleRatio;
        protected double _auxPower;
        protected double _pNormV0;
        protected double _pNormP0;
        protected double _pNormV1;
        protected double _pNormP1;

        protected double _engineRatedSpeed;
        protected double _engineIdlingSpeed;
        protected double _effectiveWheelDiameter;

        protected List<double> _speedPatternRotational;
        protected List<double> _powerPatternFC;
        protected List<double> _normalizedPowerPatternFC;
        protected List<double> _normailzedPowerPatternPollutants;
        protected List<double> _powerPatternPollutants;

        protected List<double> _cepCurveFC;
        protected List<double> _normedCepCurveFC;
        protected List<double> _gearTransmissionCurve;
        protected List<double> _speedCurveRotational;
        protected Dictionary<string, List<double>> _cepCurvePollutants;
        protected Dictionary<string, List<double>> _cepNormalizedCurvePollutants;
        protected double _idlingValueFC;
        protected Dictionary<string, double> _idlingValuesPollutants;

        protected List<double> _nNormTable;
        protected List<double> _dragNormTable;

        #endregion

        //--------------------------------------------------------------------------------------------------
        // Methods 
        //--------------------------------------------------------------------------------------------------

        #region CalcPower
        public double CalcPower(double speed, double acc, double gradient)
        {
            //Declaration
            double power = 0;
            double rotFactor = GetRotationalCoeffecient(speed);
            double powerAux = (_auxPower * _ratedPower);

            //Calculate the power
            power += (_massVehicle + _vehicleLoading) * Constants.GRAVITY_CONST * (_resistanceF0 + _resistanceF1 * speed + _resistanceF4 * Math.Pow(speed, 4)) * speed;
            power += (_crossSectionalArea * _cWValue * Constants.AIR_DENSITY_CONST / 2) * Math.Pow(speed, 3);
            power += (_massVehicle * rotFactor + _vehicleMassRot + _vehicleLoading) * acc * speed;
            power += (_massVehicle + _vehicleLoading) * Constants.GRAVITY_CONST * gradient * 0.01 * speed;
            power /= 1000;
            power /= Constants._DRIVE_TRAIN_EFFICIENCY;
            power += powerAux;

            //Return result
            return power;
        }
        #endregion

        #region CalcEngPower
        public double CalcEngPower(double power)
        {
            if (power < _powerPatternFC.First()) return _powerPatternFC.First();
            if (power > _powerPatternFC.Last()) return _powerPatternFC.Last();

            return power;
        }
        #endregion

        #region GetEmission
        public double GetEmission(string pollutant, double power, double speed, Helpers VehicleClass)
        {
            //Declaration
            List<double> emissionCurve;
            List<double> powerPattern;

            // bisection search to find correct position in power pattern	
            int upperIndex;
            int lowerIndex;

            if (VehicleClass.tClass != Constants.strBEV)
            {
                if (Math.Abs(speed) <= Constants.ZERO_SPEED_ACCURACY)
                {
                    if (pollutant == "FC")
                    {
                        return _idlingValueFC;
                    }
                    else
                    {
                        if (!_cepCurvePollutants.ContainsKey(pollutant))
                        {
                            VehicleClass.ErrMsg = "Emission pollutant " + pollutant + " not found!";
                            return 0;
                        }

                        return _idlingValuesPollutants[pollutant];
                    }
                }
            }

            if (pollutant == "FC")
            {
                emissionCurve = _cepCurveFC;
                powerPattern = _powerPatternFC;
            }
            else
            {
                if (!_cepCurvePollutants.ContainsKey(pollutant))
                {
                    VehicleClass.ErrMsg = "Emission pollutant " + pollutant + " not found!";
                    return 0;
                }

                emissionCurve = _cepCurvePollutants[pollutant];
                powerPattern = _powerPatternPollutants;
            }

            if (emissionCurve.Count == 0)
            {
                VehicleClass.ErrMsg = "Empty emission curve for " + pollutant + " found!";
                return 0;
            }
            if (emissionCurve.Count == 1)
            {
                return emissionCurve[0];
            }

            // in case that the demanded power is smaller than the first entry (smallest) in the power pattern the first is returned (should never happen)
            if (power <= powerPattern.First())
            {
                return emissionCurve[0];
            }

            // if power bigger than all entries in power pattern return the last (should never happen)
            if (power >= powerPattern.Last())
            {
                return emissionCurve.Last();
            }

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, powerPattern, power);
            return Interpolate(power, powerPattern[lowerIndex], powerPattern[upperIndex], emissionCurve[lowerIndex], emissionCurve[upperIndex]);
        }
        #endregion

        #if FLEET
        #region GetNormedEmission
        public double GetNormedEmission(string pollutant, double power, double speed, Helpers VehicleClass)
        {
            //Declaration
            List<double> emissionCurve;
            List<double> powerPattern;

            // bisection search to find correct position in power pattern	
            int upperIndex;
            int lowerIndex;

            if (pollutant == "FC")
            {
                emissionCurve = _normedCepCurveFC;
                powerPattern = _normalizedPowerPatternFC;
            }
            else
            {
                if (!_cepCurvePollutants.ContainsKey(pollutant))
                {
                    VehicleClass.ErrMsg = "Emission pollutant " + pollutant + " not found!";
                    return 0;
                }
                emissionCurve = _cepNormalizedCurvePollutants[pollutant];
                powerPattern = _normailzedPowerPatternPollutants;
            }

            if (emissionCurve.Count == 0)
            {
                VehicleClass.ErrMsg = "Empty emission curve for " + pollutant + " found!";
                return 0;
            }
            if (emissionCurve.Count == 1)
            {
                return emissionCurve[0];
            }
            // in case that the demanded power is smaller than the first entry (smallest) in the power pattern the first is returned (should never happen)
            if (power <= powerPattern.First())
            {
                return emissionCurve[0];
            }

            // if power bigger than all entries in power pattern the last is returned (should never happen)
            if (power >= powerPattern.Last())
            {
                return emissionCurve.Last();
            }

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, powerPattern, power);
            return Interpolate(power, powerPattern[lowerIndex], powerPattern[upperIndex], emissionCurve[lowerIndex], emissionCurve[upperIndex]);
        }
        #endregion
        #endif

        #region GetCO2Emission
        public double GetCO2Emission(double _FC, double _CO, double _HC, Helpers VehicleClass)
        {
            //Declaration
            double fCBr;
            double fCHC = 0.866;
            double fCCO = 0.429;
            double fCCO2 = 0.273;

            switch (_fuelType)
            {
                case Constants.strGasoline:
                    fCBr = 0.865;
                    break;
                case Constants.strDiesel:
                    fCBr = 0.863;
                    break;
                case Constants.strCNG:
                    fCBr = 0.693;
                    fCHC = 0.803;
                    break;
                case Constants.strLPG:
                    fCBr = 0.825;
                    fCHC = 0.825;
                    break;
                default:
                    VehicleClass.ErrMsg = "The propolsion type is not known! (" + _fuelType + ")";
                    return 0;
            }

            return (_FC * fCBr - _CO * fCCO - _HC * fCHC) / fCCO2;
        }
        #endregion

        #region GetDecelCoast
        public double GetDecelCoast(double speed, double acc, double gradient)
        {
            //Declaration
            int upperIndex;
            int lowerIndex;

            if (speed < Constants.SPEED_DCEL_MIN)
            {
                return speed / Constants.SPEED_DCEL_MIN * GetDecelCoast(Constants.SPEED_DCEL_MIN, acc, gradient);
            }

            double rotCoeff = GetRotationalCoeffecient(speed);
            FindLowerUpperInPattern(out lowerIndex, out upperIndex, _speedPatternRotational, speed);
            double iGear = Interpolate(speed,
                                        _speedPatternRotational[lowerIndex],
                                        _speedPatternRotational[upperIndex],
                                        _gearTransmissionCurve[lowerIndex],
                                        _gearTransmissionCurve[upperIndex]);

            double iTot = iGear * _axleRatio;

            double n = (30 * speed * iTot) / ((_effectiveWheelDiameter / 2) * Math.PI);
            double nNorm = (n - _engineIdlingSpeed) / (_engineRatedSpeed - _engineIdlingSpeed);

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, _nNormTable, nNorm);

            double fMot = 0;

            if (speed >= 10e-2)
            {
                fMot = (-Interpolate(nNorm,
                                          _nNormTable[lowerIndex],
                                          _nNormTable[upperIndex],
                                          _dragNormTable[lowerIndex],
                                          _dragNormTable[upperIndex]) * _ratedPower * 1000 / speed) / 0.9;
            }

            double fRoll = (_resistanceF0
                + _resistanceF1 * speed
                + Math.Pow(_resistanceF2 * speed, 2)
                + Math.Pow(_resistanceF3 * speed, 3)
                + Math.Pow(_resistanceF4 * speed, 4)) * (_massVehicle + _vehicleLoading) * Constants.GRAVITY_CONST;

            double fAir = _cWValue * _crossSectionalArea * 1.2 * 0.5 * Math.Pow(speed, 2);

            double fGrad = (_massVehicle + _vehicleLoading) * Constants.GRAVITY_CONST * gradient / 100;

            return -(fMot + fRoll + fAir + fGrad) / ((_massVehicle + _vehicleLoading) * rotCoeff);
        }
        #endregion

        #region GetRotationalCoeffecient
        public double GetRotationalCoeffecient(double speed)
        {
            //Declaration
            int upperIndex;
            int lowerIndex;

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, _speedPatternRotational, speed);
            return Interpolate(speed,
                                        _speedPatternRotational[lowerIndex],
                                        _speedPatternRotational[upperIndex],
                                        _speedCurveRotational[lowerIndex],
                                        _speedCurveRotational[upperIndex]);
        }
        #endregion

        #if FLEET
        #region GetGearCoeffecient
        public double GetGearCoeffecient(double speed)
        {
            //Declaration
            int upperIndex;
            int lowerIndex;

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, _speedPatternRotational, speed);
            return Interpolate(speed,
                                        _speedPatternRotational[lowerIndex],
                                        _speedPatternRotational[upperIndex],
                                        _gearTransmissionCurve[lowerIndex],
                                        _gearTransmissionCurve[upperIndex]);
        }
        #endregion

        #region GetDragCoeffecient
        public double GetDragCoeffecient(double nNorm)
        {
            //Declaration
            int upperIndex;
            int lowerIndex;

            FindLowerUpperInPattern(out lowerIndex, out upperIndex, _nNormTable, nNorm);
            return Interpolate(nNorm,
                                        _nNormTable[lowerIndex],
                                        _nNormTable[upperIndex],
                                        _dragNormTable[lowerIndex],
                                        _dragNormTable[upperIndex]);
        }
        #endregion
        #endif

        #region FindLowerUpperInPattern
        private void FindLowerUpperInPattern(out int lowerIndex, out int upperIndex, List<double> pattern, double value)
        {
            lowerIndex = 0;
            upperIndex = 0;

            if (value <= pattern.First())
            {
                lowerIndex = 0;
                upperIndex = 0;
                return;
            }

            if (value >= pattern.Last())
            {
                lowerIndex = pattern.Count - 1;
                upperIndex = pattern.Count - 1;
                return;
            }

            // bisection search to find correct position in power pattern	
            int middleIndex = (pattern.Count - 1) / 2;
            upperIndex = pattern.Count - 1;
            lowerIndex = 0;

            while (upperIndex - lowerIndex > 1)
            {
                if (pattern[middleIndex] == value)
                {
                    lowerIndex = middleIndex;
                    upperIndex = middleIndex;
                    return;
                }
                else if (pattern[middleIndex] < value)
                {
                    lowerIndex = middleIndex;
                    middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
                }
                else
                {
                    upperIndex = middleIndex;
                    middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
                }
            }

            if (pattern[lowerIndex] <= value && value < pattern[upperIndex])
            {
                return;
            }
        }
        #endregion

        #region Interpolate
        private double Interpolate(double px, double p1, double p2, double e1, double e2)
        {
            if (p2 == p1)
                return e1;

            return e1 + (px - p1) / (p2 - p1) * (e2 - e1);
        }
        #endregion

        #region GetMaxAccel
        public double GetMaxAccel(double speed, double gradient)
        {
            double rotFactor = GetRotationalCoeffecient(speed);
            double pMaxForAcc = GetPMaxNorm(speed) * _ratedPower - CalcPower(speed, 0, gradient);

            return (pMaxForAcc * 1000) / ((_massVehicle * rotFactor + _vehicleMassRot + _vehicleLoading) * speed);
        }
        #endregion

        #region GetPMaxNorm
        private double GetPMaxNorm(double speed)
        {
            // Linear function between v0 and v1, constant elsewhere
            if (speed <= _pNormV0)
                return _pNormP0;
            else if (speed >= _pNormV1)
                return _pNormP1;
            else
            {
                return Interpolate(speed, _pNormV0, _pNormV1, _pNormP0, _pNormP1);
            }
        }
        #endregion

        //--------------------------------------------------------------------------------------------------
        // Operators for fleetmix
        //--------------------------------------------------------------------------------------------------

        #if FLEET
        #region AddRangeCeps
        public static CEP AddRangeCeps(CEP[] cps, Helpers Helper)
        {
            #region SingleValues
            CEP newCEP = new CEP(cps.Select(p => p.HeavyVehicle ? 1 : 0).Sum() > 0,
                cps.Select(p => p._massVehicle).Sum(),
                cps.Select(p => p._vehicleLoading).Sum(),
                cps.Select(p => p._vehicleMassRot).Sum(),
                cps.Select(p => p._crossSectionalArea).Sum(),
                cps.Select(p => p._cWValue).Sum(),
                cps.Select(p => p._resistanceF0).Sum(),
                cps.Select(p => p._resistanceF1).Sum(),
                cps.Select(p => p._resistanceF2).Sum(),
                cps.Select(p => p._resistanceF3).Sum(),
                cps.Select(p => p._resistanceF4).Sum(),
                cps.Select(p => p._axleRatio).Sum(),
                cps.Select(p => p._auxPower).Sum(),
                cps.Select(p => p._ratedPower).Sum(),
                cps.Select(p => p._engineIdlingSpeed).Sum(),
                cps.Select(p => p._engineRatedSpeed).Sum(),
                cps.Select(p => p._effectiveWheelDiameter).Sum(),
                cps.Select(p => p._pNormV0).Sum(),
                cps.Select(p => p._pNormP0).Sum(),
                cps.Select(p => p._pNormV1).Sum(),
                cps.Select(p => p._pNormP1).Sum());

            newCEP._fuelType = cps.First().FuelType;
            #endregion

            #region SpeedRotationalTable
            double minSpeedRotational = cps.Select(p => p._speedPatternRotational.First()).Min();
            double maxSpeedRotational = cps.Select(p => p._speedPatternRotational.Last()).Max();

            newCEP._speedPatternRotational
                = CreatePattern(minSpeedRotational,
                maxSpeedRotational,
                Constants.SPEED_ROTATIONAL_INCREMENT);

            newCEP._speedCurveRotational = new List<double>();
            newCEP._gearTransmissionCurve = new List<double>();

            for (int i = 0; i < newCEP._speedPatternRotational.Count; i++)
            {
                newCEP._speedCurveRotational.Add(cps.Select(p => p.GetRotationalCoeffecient(newCEP._speedPatternRotational[i])).Sum());

                newCEP._gearTransmissionCurve.Add(cps.Select(p => p.GetGearCoeffecient(newCEP._speedPatternRotational[i])).Sum());
            }
            #endregion

            #region NormalizingPower
            newCEP._drivingPower = newCEP.CalcPower(Constants.NORMALIZING_SPEED, Constants.NORMALIZING_ACCELARATION, 0);

            if (newCEP._heavyVehicle)
            {
                newCEP._normalizingPower = newCEP._ratedPower;
                newCEP._normalizingType = NormalizingType.RatedPower;
            }
            else
            {
                newCEP._normalizingPower = newCEP._drivingPower;
                newCEP._normalizingType = NormalizingType.DrivingPower;
            }
            #endregion

            #region FC
            double minNormPowerPatternFC = cps.Select(p => p._normalizedPowerPatternFC.First()).Min();
            double maxNormPowerPatternFC = cps.Select(p => p._normalizedPowerPatternFC.Last()).Max();

            newCEP._normalizedPowerPatternFC
                = CreatePattern(minNormPowerPatternFC,
                maxNormPowerPatternFC,
                Constants.POWER_FC_INCREMENT);

            newCEP._cepCurveFC = new List<double>();
            newCEP._normedCepCurveFC = new List<double>();
            newCEP._powerPatternFC = new List<double>();

            for (int i = 0; i < newCEP._normalizedPowerPatternFC.Count; i++)
            {
                double newCepVal = cps.Select(p => p.GetNormedEmission("FC", newCEP._normalizedPowerPatternFC[i], double.MaxValue, Helper)).Sum();
                newCEP._cepCurveFC.Add(newCepVal * newCEP._ratedPower);
                newCEP._normedCepCurveFC.Add(newCepVal);
                newCEP._powerPatternFC.Add(newCEP._normalizedPowerPatternFC[i] * newCEP._ratedPower);
            }
            #endregion

            #region Pollutants
            double minNormPowerPattern = cps.Select(p => p._normailzedPowerPatternPollutants.First()).Min();
            double maxNormPowerPattern = cps.Select(p => p._normailzedPowerPatternPollutants.Last()).Max();

            newCEP._normailzedPowerPatternPollutants
                 = CreatePattern(minNormPowerPattern,
                 maxNormPowerPattern,
                 Constants.POWER_POLLUTANT_INCREMENT);

            newCEP._cepCurvePollutants = new Dictionary<string, List<double>>();
            newCEP._powerPatternPollutants = new List<double>();
            newCEP._cepNormalizedCurvePollutants = new Dictionary<string, List<double>>();

            foreach (string id in cps.First()._cepCurvePollutants.Keys)
            {
                newCEP._cepCurvePollutants.Add(id, new List<double>());
                newCEP._cepNormalizedCurvePollutants.Add(id, new List<double>());
            }

            for (int i = 0; i < newCEP._normailzedPowerPatternPollutants.Count; i++)
            {
                foreach (string id in newCEP._cepCurvePollutants.Keys)
                {
                    if (newCEP.NormalizingTypeX == NormalizingType.RatedPower)
                    {
                        double newCepVal = cps.Select(p => p.GetNormedEmission(id, newCEP._normailzedPowerPatternPollutants[i], double.MaxValue, Helper)).Sum();
                        newCEP._cepCurvePollutants[id].Add(newCepVal * newCEP._ratedPower);
                        newCEP._cepNormalizedCurvePollutants[id].Add(newCepVal);
                    }
                    else
                    {
                        newCEP._cepCurvePollutants[id].Add(cps.Select(p => p.GetEmission(id, newCEP._normailzedPowerPatternPollutants[i] * p._normalizingPower, double.MaxValue, Helper)).Sum());
                        newCEP._cepNormalizedCurvePollutants[id].Add(cps.Select(p => p.GetNormedEmission(id, newCEP._normailzedPowerPatternPollutants[i], double.MaxValue, Helper)).Sum());
                    }
                }
                newCEP._powerPatternPollutants.Add(newCEP._normailzedPowerPatternPollutants[i] * newCEP.NormalizingPower);
            }
            #endregion

            #region IdlingValues
            newCEP._idlingValueFC = cps.Select(p => p._idlingValueFC).Sum();
            newCEP._idlingValuesPollutants = new Dictionary<string, double>();

            foreach (string id in cps.First()._idlingValuesPollutants.Keys)
            {
                newCEP._idlingValuesPollutants.Add(id, cps.Select(p => p._idlingValuesPollutants[id]).Sum());
            }
            #endregion

            #region TragTable
            double minTragTable = cps.Select(p => p._nNormTable.First()).Min();
            double maxTragTable = cps.Select(p => p._nNormTable.Last()).Max();

            newCEP._nNormTable
                = CreatePattern(minTragTable,
                maxTragTable,
                Constants.NNORM_INCREMENT);

            newCEP._dragNormTable = new List<double>();

            for (int i = 0; i < newCEP._nNormTable.Count; i++)
            {
                newCEP._dragNormTable.Add(cps.Select(p => p.GetDragCoeffecient(newCEP._nNormTable[i])).Sum());
            }
            #endregion
            return newCEP;
        }
        #endregion

        #region Operator *
        public static CEP operator *(CEP cp1, double d)
        {
            #region SingleValues
            CEP newCEP = new CEP(cp1.HeavyVehicle,
                d * cp1._massVehicle,
                d * cp1._vehicleLoading,
                d * cp1._vehicleMassRot,
                d * cp1._crossSectionalArea,
                d * cp1._cWValue,
                d * cp1._resistanceF0,
                d * cp1._resistanceF1,
                d * cp1._resistanceF2,
                d * cp1._resistanceF3,
                d * cp1._resistanceF4,
                d * cp1._axleRatio,
                d * cp1._auxPower,
                d * cp1._ratedPower,
                d * cp1._engineIdlingSpeed,
                d * cp1._engineRatedSpeed,
                d * cp1._effectiveWheelDiameter,
                d * cp1._pNormV0,
                d * cp1._pNormP0,
                d * cp1._pNormV1,
                d * cp1._pNormP1);

            newCEP._fuelType = cp1.FuelType;
            #endregion

            #region SpeedRotationalTable
            newCEP._speedPatternRotational = new List<double>(cp1._speedPatternRotational);
            newCEP._speedCurveRotational = new List<double>(cp1._speedCurveRotational.Select(p => p * d));
            newCEP._gearTransmissionCurve = new List<double>(cp1._gearTransmissionCurve.Select(p => p * d));
            #endregion

            #region NormalizingPower
            newCEP._drivingPower = newCEP.CalcPower(Constants.NORMALIZING_SPEED, Constants.NORMALIZING_ACCELARATION, 0);

            if (newCEP._heavyVehicle)
            {
                newCEP._normalizingPower = newCEP._ratedPower;
                newCEP._normalizingType = NormalizingType.RatedPower;
            }
            else
            {
                newCEP._normalizingPower = newCEP._drivingPower;
                newCEP._normalizingType = NormalizingType.DrivingPower;
            }
            #endregion

            #region FC
            newCEP._powerPatternFC = new List<double>(cp1._powerPatternFC.Select(p => p * d));
            newCEP._normalizedPowerPatternFC = new List<double>(cp1._normalizedPowerPatternFC);
            newCEP._cepCurveFC = new List<double>(cp1._cepCurveFC.Select(p => p * d));
            newCEP._normedCepCurveFC = new List<double>(cp1._normedCepCurveFC.Select(p => p * d));
            #endregion

            #region Pollutants
            newCEP._powerPatternPollutants = new List<double>(cp1._normailzedPowerPatternPollutants.Select(p => p * newCEP._normalizingPower));
            newCEP._normailzedPowerPatternPollutants = new List<double>(cp1._normailzedPowerPatternPollutants);
            newCEP._cepCurvePollutants = new Dictionary<string, List<double>>();
            newCEP._cepNormalizedCurvePollutants = new Dictionary<string, List<double>>();

            foreach (string id in cp1._cepCurvePollutants.Keys)
            {
                newCEP._cepCurvePollutants.Add(id, new List<double>(cp1._cepCurvePollutants[id].Select(p => p * d)));
                newCEP._cepNormalizedCurvePollutants.Add(id, new List<double>(cp1._cepNormalizedCurvePollutants[id].Select(p => p * d)));
            }
            #endregion

            #region IdlingValues
            newCEP._idlingValueFC = cp1._idlingValueFC * d;
            newCEP._idlingValuesPollutants = new Dictionary<string, double>();

            foreach (string id in cp1._idlingValuesPollutants.Keys)
            {
                newCEP._idlingValuesPollutants.Add(id,
                    cp1._idlingValuesPollutants[id] * d);
            }
            #endregion

            #region DragTable
            newCEP._nNormTable = new List<double>(cp1._nNormTable);
            newCEP._dragNormTable = new List<double>(cp1._dragNormTable.Select(p => p * d));
            #endregion
            return newCEP;
        }
        #endregion

        #region CreatePattern
        static public List<double> CreatePattern(double min, double max, double increment)
        {
            //Declaration
            List<double> pattern = new List<double>();
            double actualMin = min;
            double actualMax = max;

            if (min < 0)
                actualMin = Math.Ceiling(min / increment) * increment;
            else
                actualMin = Math.Floor(min / increment) * increment;

            if (max < 0)
                actualMax = Math.Floor(max / increment) * increment;
            else
                actualMax = Math.Ceiling(max / increment) * increment;

            double curVal = actualMin;

            while (curVal <= actualMax)
            {
                pattern.Add(curVal);
                curVal += increment;
            }
            return pattern;
        }
        #endregion
        #endif
    }
}
