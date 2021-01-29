using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PHEMlightdll
{
    public class Helpers
    {
        //--------------------------------------------------------------------------------------------------
        // Members 
        //--------------------------------------------------------------------------------------------------
        #region Members
        private string _vClass;
        public string vClass
        {
            get { return _vClass; }
            set { _vClass = value; }
        }
        private string _eClass;
        public string eClass
        {
            get { return _eClass; }
            set { _eClass = value; }
        }
        private string _tClass;
        public string tClass
        {
            get { return _tClass; }
            set { _tClass = value; }
        }
        private string _sClass;
        public string sClass
        {
            get { return _sClass; }
            set { _sClass = value; }
        }
        private string _Class;
        public string gClass
        {
            get { return _Class; }
            set { _Class = value; }
        }
        private string _ErrMsg;
        public string ErrMsg
        {
            get { return _ErrMsg; }
            set { _ErrMsg = value; }
        }
        private string _commentPrefix;
        public string CommentPrefix
        {
            get { return _commentPrefix; }
            set { _commentPrefix = value; }
        }
        private string _PHEMDataV;
        public string PHEMDataV
        {
            get { return _PHEMDataV; }
            set { _PHEMDataV = value; }
        }
        #endregion

        #region Classes
        //Get vehicle class
        private bool getvclass(string VEH)
        {
            // Set the drive train efficency
            Constants.DRIVE_TRAIN_EFFICIENCY = Constants.DRIVE_TRAIN_EFFICIENCY_All;

            //Get the vehicle class
            if (VEH.IndexOf(Constants.strPKW) >= 0)
            {
                _vClass = Constants.strPKW;
                return true;
            }
            else if (VEH.IndexOf(Constants.strLNF) >= 0)
            {
                _vClass = Constants.strLNF;
                return true;
            }
            else if (VEH.IndexOf(Constants.strLKW) >= 0)
            {
                _vClass = Constants.strLKW;
                return true;
            }
            else if (VEH.IndexOf(Constants.strLSZ) >= 0)
            {
                _vClass = Constants.strLSZ;
                return true;
            }
            else if (VEH.IndexOf(Constants.strRB) >= 0)
            {
                _vClass = Constants.strRB;
                return true;
            }
            else if (VEH.IndexOf(Constants.strLB) >= 0)
            {
                _vClass = Constants.strLB;
                Constants.DRIVE_TRAIN_EFFICIENCY = Constants.DRIVE_TRAIN_EFFICIENCY_CB;
                return true;
            }
            else if (VEH.IndexOf(Constants.strMR2) >= 0)
            {
                _vClass = Constants.strMR2;
                return true;
            }
            else if (VEH.IndexOf(Constants.strMR4) >= 0)
            {
                _vClass = Constants.strMR4;
                return true;
            }
            else if (VEH.IndexOf(Constants.strKKR) >= 0)
            {
                _vClass = Constants.strKKR;
                return true;
            }
            //Should never happens
            _ErrMsg = "Vehicle class not defined! (" + VEH + ")";
            return false;
        }

        //Get technologie Class
        private bool gettclass(string VEH)
        {
            if (VEH.IndexOf("_" + Constants.strDiesel) > 0)
            {
                if (VEH.IndexOf("_" + Constants.strHybrid) > 0)
                {
                    _tClass = Constants.strDiesel + "_" + Constants.strHybrid;
                    return true;
                }
                else
                {
                    _tClass = Constants.strDiesel;
                    return true;
                }

            }
            else if (VEH.IndexOf("_" + Constants.strGasoline) > 0)
            {
                if (VEH.IndexOf("_" + Constants.strHybrid) > 0)
                {
                    _tClass = Constants.strGasoline + "_" + Constants.strHybrid;
                    return true;
                }
                else
                {
                    _tClass = Constants.strGasoline;
                    return true;
                }
            }
            else if (VEH.IndexOf("_" + Constants.strCNG) > 0)
            {
                _tClass = Constants.strCNG;
                return true;
            }
            else if (VEH.IndexOf("_" + Constants.strBEV) > 0)
            {
                _tClass = Constants.strBEV;
                return true;
            }
            //Should never happens
            _ErrMsg = "Fuel class not defined! (" + VEH + ")";
            return false;
        }

        //Get size class
        private bool getsclass(string VEH)
        {
            if (VEH.IndexOf(Constants.strLKW) >= 0)
            {
                if (VEH.IndexOf("_" + Constants.strSII) > 0) 
                {
                    _sClass = Constants.strSII;
                    return true;
                }
                else if (VEH.IndexOf("_" + Constants.strSI) > 0) 
                {
                    _sClass = Constants.strSI;
                    return true;
                }
                else
                {
                    //Should never happen
                    _ErrMsg = "Size class not defined! (" + VEH + ")";
                    return false;
                }
            }
            else if (VEH.IndexOf(Constants.strLNF) >= 0)
            {
                if (VEH.IndexOf("_" + Constants.strSIII) > 0)
                {
                    _sClass = Constants.strSIII;
                    return true;
                }
                else if (VEH.IndexOf("_" + Constants.strSII) > 0) 
                {
                    _sClass = Constants.strSII;
                    return true;
                }
                else if (VEH.IndexOf("_" + Constants.strSI) > 0) 
                {
                    _sClass = Constants.strSI;
                    return true;
                }
                else
                {
                    _ErrMsg = "Size class not defined! (" + VEH.Substring(VEH.LastIndexOf(@"\"), VEH.Length - VEH.LastIndexOf(@"\")) + ")";
                    return false;
                }
            }
            else
            {
                _sClass = "";
                return true;
            }
        }

        //Get euro class
        private bool geteclass(string VEH)
        {
            if (VEH.IndexOf("_" + Constants.strEU) > 0)
            {
                if (VEH.IndexOf("_", VEH.IndexOf("_" + Constants.strEU) + 1) > 0)
                {
                    _eClass = Constants.strEU + VEH.Substring(VEH.IndexOf("_" + Constants.strEU) + 3, VEH.IndexOf("_", VEH.IndexOf("_" + Constants.strEU) + 1) - (VEH.IndexOf("_" + Constants.strEU) + 3));
                    return true;
                }
                else if (VEH.IndexOf(".", VEH.IndexOf("_" + Constants.strEU) + 1) > 0)
                {
                    _eClass = Constants.strEU + VEH.Substring(VEH.IndexOf("_" + Constants.strEU) + 3, VEH.IndexOf(".", VEH.IndexOf("_" + Constants.strEU) + 1) - (VEH.IndexOf("_" + Constants.strEU) + 3));
                    return true;
                }
                else
                {
                    _eClass = Constants.strEU + VEH.Substring(VEH.IndexOf("_" + Constants.strEU) + 3, VEH.Length - (VEH.IndexOf("_" + Constants.strEU) + 3));
                    return true;
                }
            }
            else if (VEH.IndexOf("_" + Constants.strBEV) > 0)
            {
                _eClass = "";
                return true;
            }
            //Should never happens
            _ErrMsg = "Euro class not defined! (" + VEH + ")";
            return false;
        }

        //Set complete class string
        public bool setclass(string VEH)
        {
            if (getvclass(VEH)) { _Class = _vClass; } else { return false; }
            if (getsclass(VEH)) { if (_sClass != "") { _Class = _Class + "_" + sClass; } } else { return false; }
            if (gettclass(VEH)) { _Class = _Class + "_" + tClass; } else { return false; }
            if (geteclass(VEH)) { if (_eClass != "") { _Class = _Class + "_" + eClass; } } else { return false; }
            return true;
        }
        #endregion


    }
}
