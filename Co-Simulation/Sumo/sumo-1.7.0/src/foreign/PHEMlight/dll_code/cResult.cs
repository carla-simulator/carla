using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PHEMlightdll
{
    public class VehicleResult
    {
        #region Construtor
        public VehicleResult(double time,
                             double speed,
                             double grad,
                             double power,
                             double pPos,
                             double pNormRated,
                             double pNormDrive,
                             double acc,
                             double fc,
                             double fcel,
                             double cO2,
                             double nOx,
                             double hC,
                             double pM,
                             double cO)
        {
            _time = time;
            _speed = speed;
            _grad = grad;
            _power = power;
            _pPos = pPos;
            _pNormRated = pNormRated;
            _pNormDrive = pNormDrive;
            _accelaration = acc;
            _emissionData = new EmissionData(fc,
                                             fcel,
                                             cO2,
                                             nOx,
                                             hC,
                                             pM,
                                             cO);
        }
        #endregion

        //--------------------------------------------------------------------------------------------------
        // GeneralState 
        //--------------------------------------------------------------------------------------------------

        #region Time
        private double _time;
        public double Time
        {
            get
            {
                return _time;
            }
        }
        #endregion

        #region Speed
        private double _speed;
        public double Speed
        {
            get
            {
                return _speed;
            }
        }
        #endregion

        #region Grad
        private double _grad;
        public double Grad
        {
            get
            {
                return _grad;
            }
        }
        #endregion

        #region Power
        private double _power;
        public double Power
        {
            get
            {
                return _power;
            }
        }
        #endregion

        #region PPos
        private double _pPos;
        public double PPos
        {
            get
            {
                return _pPos;
            }
        }
        #endregion

        #region PNormRated
        private double _pNormRated;
        public double PNormRated
        {
            get
            {
                return _pNormRated;
            }
            set
            {
                _pNormRated = value;
            }
        }
        #endregion

        #region PNormDrive
        private double _pNormDrive;
        public double PNormDrive
        {
            get
            {
                return _pNormDrive;
            }
            set
            {
                _pNormDrive = value;
            }
        }
        #endregion

        #region Accelaration
        private double _accelaration;
        public double Accelaration
        {
            get
            {
                return _accelaration;
            }
        }
        #endregion

        //--------------------------------------------------------------------------------------------------
        // EmissionData 
        //--------------------------------------------------------------------------------------------------
        
        #region EmissionData
        private EmissionData _emissionData;
        public EmissionData EmissionData
        {
            get
            {
                return _emissionData;
            }
        }
        #endregion
    }

    public class EmissionData
    {

        #region Constructor
        public EmissionData(double fc,
                            double fcel,
                            double cO2,
                            double nOx,
                            double hC,
                            double pM,
                            double cO)
        {
            _fc = fc;
            _fcel = fcel;
            _cO2 = cO2;
            _nOx = nOx;
            _hC = hC;
            _pM = pM;
            _cO = cO;
        }
        #endregion

        //--------------------------------------------------------------------------------------------------
        // GeneralState 
        //--------------------------------------------------------------------------------------------------

        #region FC
        private double _fc;
        public double FC
        {
            get
            {
                return _fc;
            }
        }
        #endregion

        #region FCel
        private double _fcel;
        public double FCel
        {
            get
            {
                return _fcel;
            }
        }
        #endregion

        #region CO2
        private double _cO2;
        public double CO2
        {
            get
            {
                return _cO2;
            }
        }
        #endregion

        #region NOx
        private double _nOx;
        public double NOx
        {
            get
            {
                return _nOx;
            }
        }
        #endregion

        #region HC
        private double _hC;
        public double HC
        {
            get
            {
                return _hC;
            }
        }
        #endregion

        #region PM
        private double _pM;
        public double PM
        {
            get
            {
                return _pM;
            }
        }
        #endregion

        #region CO
        private double _cO;
        public double CO
        {
            get
            {
                return _cO;
            }
        }
        #endregion
    }
}
