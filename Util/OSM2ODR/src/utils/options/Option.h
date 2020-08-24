/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    Option.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 17 Dec 2001
///
// Classes representing a single program option (with different types)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <exception>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @typedef IntVector
 * @brief Definition of a vector of ints
 */
typedef std::vector<int> IntVector;
/**
 * @typedef StringVector
 * @brief Definition of a vector of strings
 */
typedef std::vector<std::string> StringVector;

/* -------------------------------------------------------------------------
 * Option
 * ----------------------------------------------------------------------- */
/**
 * @class Option
 * @brief A class representing a single program option
 *
 * The base class for a single program option. All options which hold values
 *  are derived from this class as the type of stored values may differ.
 *
 * Most of the getter-methods throw exceptions because this base class is not meant
 *  to hold any values by itself. Instead, the derived classes implement the
 *  appropriate method (Option_Integer implements getInt, f.e.). So, when one
 *  tries to retrieve a value which is not of the type of the option, an
 *  exception will be thrown. This behaviour is meant to be valid, because
 *  errors like this one only occur when building and testing the application.
 *
 * Due to described behaviour, this class has no public constructors. Only
 *  construction of derived, value and type holding, classes is allowed.
 *
 *  At the begin (after being constructed) an Option either has a default value or not.
 *   In dependance to this, myHaveTheDefaultValue is set. Also, myAmSet is set to
 *   true if a default value was supported. myAmWritable is set to true,
 *   indicating that a new value may be set.
 *
 * Each option may have a description about its purpose stored. Furthermore, it
 *  stores a man-readable type name for this option.
 */
class Option {
public:
    /** destructor */
    virtual ~Option();


    /** @brief returns the information whether this options holds a valid value
     * @return Whether a value has been set
     */
    bool isSet() const;


    /** @brief marks this option as unset
     */
    void unSet();


    /** @brief Returns the stored double value
     *
     * Option_Float returns the stored real number in this method's reimplementation.
     *  All other option classes do not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored real number if being an instance of Option_Float
     * @exception InvalidArgument If the class is not an instance of Option_Float
     */
    virtual double getFloat() const;


    /** @brief Returns the stored integer value
     *
     * Option_Integer returns the stored integer number in this method's reimplementation.
     *  All other option classesdo not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored integer number if being an instance of Option_Integer
     * @exception InvalidArgument If the class is not an instance of Option_Integer
     */
    virtual int getInt() const;


    /** @brief Returns the stored string value
     *
     * Option_String returns the stored string in this method's reimplementation.
     *  Option_FileName's reimplementation is only to be used for single filename string-vector options.
     *  All other option classes do not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored string if being an instance of Option_String or Option_FileName
     * @exception InvalidArgument If the class is not an instance of Option_String or Option_FileName
     */
    virtual std::string getString() const;


    /** @brief Returns the stored boolean value
     *
     * Option_Bool returns the stored boolean in this method's reimplementation.
     *  All other option classes do not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored boolean if being an instance of Option_Bool
     * @exception InvalidArgument If the class is not an instance of Option_Bool
     */
    virtual bool getBool() const;


    /** @brief Returns the stored integer vector
     *
     * Option_IntVector returns the stored integer vector in this method's reimplementation.
     *  All other option classes do not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored integer vector if being an instance of Option_IntVector
     * @exception InvalidArgument If the class is not an instance of Option_IntVector
     */
    virtual const IntVector& getIntVector() const;

    /** @brief Returns the stored string vector
     *
     * Option_StringVector returns the stored string vector in this method's reimplementation.
     *  All other option classes do not override this method which throws an InvalidArgument-exception.
     *
     * @return Returns the stored string vector if being an instance of Option_StringVector
     * @exception InvalidArgument If the class is not an instance of Option_StringVector
     */
    virtual const StringVector& getStringVector() const;

    /** @brief Stores the given value
     *
     * This method is overriden by all option classes.
     *  The value is converted into the proper type and stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * If the string could not be converted into the type, an InvalidArgument
     *  is thrown.
     *
     * @return Whether the new value could be set
     * @exception InvalidArgument If the value could not be converted
     */
    virtual bool set(const std::string& v) = 0;


    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @return The stored value encoded into a string-
     */
    virtual std::string getValueString() const = 0;


    /** @brief Returns the information whether the option is a bool option
     *
     * Returns false. Only Option_Bool overrides this method returning true.
     *
     * @return true if the Option is an Option_Bool, false otherwise
     */
    virtual bool isBool() const;


    /** @brief Returns the information whether the option holds the default value
     *
     * @return true if the option was not set from command line / configuration, false otherwise
     */
    virtual bool isDefault() const;


    /** @brief Returns the information whether this option is a file name
     *
     * Returns false. Only Option_FileName overrides this method returning true.
     *
     * @return true if the Option is an Option_FileName, false otherwise
     */
    virtual bool isFileName() const;


    /** @brief Returns the information whether the option may be set a further time
     *
     * This method returns whether the option was not already set using command line
     *  options / configuration. This is done by returning the value of myAmWritable.
     *
     * @return Whether the option may be set from the command line / configuration
     */
    bool isWriteable() const;


    /** @brief Resets the option to be writeable
     *
     * An option is writable after initialisation, but as soon as it gets set,
     *  it is no longer writeable. This method resets the writable-flag.
     */
    void resetWritable();


    /** @brief Resets the option to be on its default value
     *
     * An option is on its default after initialisation with a value, but as soon as it gets set,
     *  it is no longer. This method resets the default-flag.
     */
    void resetDefault();


    /** @brief Returns the description of what this option does
     *
     * The description stored in myDescription is returned.
     *
     * @return The description of this option's purpose
     */
    const std::string& getDescription() const;


    /** @brief Sets the description of what this option does
     *
     * The description stored in myDescription is returned.
     *
     * @return The description of this option's purpose
     */
    void setDescription(const std::string& desc);


    /** @brief Returns the mml-type name of this option
     *
     * The type name stored in myTypeName is returned.
     *
     * @return The man-readable type name
     */
    virtual const std::string& getTypeName() const;


    /** @brief Create a new Option of the given type with given default value but make it unset
     *  @note Was implemented to allow warning once if user didn't set the option, refs. #4567
     *  @see MSDeviceSSM::insertOptions()
     */
    template<class OptionType, class ValueType>
    static OptionType* makeUnsetWithDefault(ValueType def) {
        OptionType* o = new OptionType(def);
        o->unSet();
        return o;
    }


protected:
    /** @brief Marks the information as set
     *
     * Sets the "myAmSet" - information. Returns whether the option was writeable before.
     *
     * @return Whether the option was not set before.
     */
    bool markSet();


protected:
    /** @brief Constructor
     *
     * This constructor should be used by derived classes.
     * The boolean value indicates whether a default value was supplied or not.
     *
     * @param[in] set A default value was supplied
     */
    Option(bool set = false);


    /** @brief Copy constructor */
    Option(const Option& s);


    /** @brief Assignment operator */
    virtual Option& operator=(const Option& s);


protected:
    /// @brief A type name for this option (has presets, but may be overwritten)
    std::string myTypeName;


private:
    /** @brief information whether the value is set */
    bool myAmSet;

    /** @brief information whether the value is the default value (is then set) */
    bool myHaveTheDefaultValue;

    /** @brief information whether the value may be changed */
    bool myAmWritable;

    /// @brief The description what this option does
    std::string myDescription;

};


/* -------------------------------------------------------------------------
 * Option_Integer
 * ----------------------------------------------------------------------- */
/**
 * @class Option_Integer
 * @brief An integer-option
 */
class Option_Integer : public Option {
public:
    /** @brief Constructor for an option with a default value
     *
     * Calls Option(true)
     *
     * @param[in] value This option's default value
     */
    Option_Integer(int value);


    /** @brief Copy constructor */
    Option_Integer(const Option_Integer& s);


    /** @brief Destructor */
    ~Option_Integer();


    /** @brief Assignment operator */
    Option_Integer& operator=(const Option_Integer& s);


    /** @brief Returns the stored integer value
     * @see Option::getInt()
     * @return Returns the stored integer number
     */
    int getInt() const;


    /** @brief Stores the given value after parsing it into an integer
     *
     *  The value is converted into an integer and stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * If the string could not be converted into an integer, an InvalidArgument
     *  is thrown.
     *
     * @see bool Option::set(std::string v)
     * @return Whether the new value could be set
     * @exception InvalidArgument If the value could not be converted into an integer
     */
    bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;


private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    int      myValue;

};


/* -------------------------------------------------------------------------
 * Option_String
 * ----------------------------------------------------------------------- */
class Option_String : public Option {
public:
    /** @brief Constructor for an option with no default value
     *
     * Calls Option(false)
     */
    Option_String();


    /** @brief Constructor for an option with a default value
     *
     * Calls Option(true)
     *
     * @param[in] value This option's default value
     */
    Option_String(const std::string& value, std::string typeName = "STR");


    /** @brief Copy constructor */
    Option_String(const Option_String& s);


    /** @brief Destructor */
    virtual ~Option_String();


    /** @brief Assignment operator */
    Option_String& operator=(const Option_String& s);


    /** @brief Returns the stored string value
     * @see std::string Option::getString()
     * @return Returns the stored string
     */
    std::string getString() const;


    /** @brief Stores the given value
     *
     *  The value is stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * @see bool Option::set(std::string v)
     * @return Whether the new value could be set
     */
    bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;


protected:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    std::string      myValue;

};


/* -------------------------------------------------------------------------
 * Option_Float
 * ----------------------------------------------------------------------- */
class Option_Float : public Option {
public:
    /** @brief Constructor for an option with a default value
     *
     * Calls Option(true)
     *
     * @param[in] value This option's default value
     */
    Option_Float(double value);


    /** @brief Copy constructor */
    Option_Float(const Option_Float& s);


    /** @brief Destructor */
    ~Option_Float();


    /** @brief Assignment operator */
    Option_Float& operator=(const Option_Float& s);


    /** @brief Returns the stored double value
     * @see double Option::getFloat()
     * @return Returns the stored real number
     */
    double getFloat() const;


    /** @brief Stores the given value after parsing it into a double
     *
     *  The value is converted into a double and stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * If the string could not be converted into a double, an InvalidArgument
     *  is thrown.
     *
     * @see bool Option::set(std::string v)
     * @return Whether the new value could be set
     * @exception InvalidArgument If the value could not be converted into a double
     */
    bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;


private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    double       myValue;

};


/* -------------------------------------------------------------------------
 * Option_Bool
 * ----------------------------------------------------------------------- */
class Option_Bool : public Option {
public:
    /** @brief Constructor for an option with a default value
     *
     * Calls Option(true)
     *
     * @param[in] value This option's default value
     */
    Option_Bool(bool value);


    /** @brief Copy constructor */
    Option_Bool(const Option_Bool& s);


    /** @brief Destructor */
    ~Option_Bool();


    /** @brief Assignment operator */
    Option_Bool& operator=(const Option_Bool& s);


    /** @brief Returns the stored boolean value
     * @see bool Option::getBool()
     * @return Returns the stored boolean
     */
    bool getBool() const;

    /** sets the given value (converts it to bool) */
    virtual bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * If myValue is true, "true" is returned, "false" otherwise.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    virtual std::string getValueString() const;


    /** @brief Returns true, the information whether the option is a bool option
     *
     * Returns true.
     *
     * @see bool Option::isBool()
     * @return true
     */
    bool isBool() const;


protected:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    bool        myValue;

};



/* -------------------------------------------------------------------------
 * Option_BoolExtended
 * ----------------------------------------------------------------------- */
class Option_BoolExtended : public Option_Bool {
public:
    /** @brief Constructor for an option that can be used without an argument
     * like Option_BoolExtended but which also handles value strings
     *
     * Calls Option(true)
     *
     * @param[in] value This option's default value
     */
    Option_BoolExtended(bool value);


    /** @brief Copy constructor */
    Option_BoolExtended(const Option_BoolExtended& s);


    /** @brief Destructor */
    ~Option_BoolExtended();


    /** @brief Assignment operator */
    Option_BoolExtended& operator=(const Option_BoolExtended& s);


    /** sets the given value (converts it to bool) */
    bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * If myValue is true, "true" is returned, "false" otherwise.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;


private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    std::string myValueString;

};


/* -------------------------------------------------------------------------
 * Option_IntVector
 * ----------------------------------------------------------------------- */
class Option_IntVector : public Option {
public:
    /** @brief Constructor for an option with no default value
     */
    Option_IntVector();


    /** @brief Constructor for an option with a default value
     *
     * @param[in] value This option's default value
     */
    Option_IntVector(const IntVector& value);


    /** @brief Copy constructor */
    Option_IntVector(const Option_IntVector& s);


    /** @brief Destructor */
    virtual ~Option_IntVector();


    /** @brief Assignment operator */
    Option_IntVector& operator=(const Option_IntVector& s);


    /** @brief Returns the stored integer vector
     * @see const IntVector &Option::getIntVector()
     * @return Returns the stored integer vector
     */
    const IntVector& getIntVector() const;


    /** @brief Stores the given value after parsing it into a vector of integers
     *
     *  The value is converted into a vector of integers and stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * If the string could not be converted into a vector of integers, an InvalidArgument
     *  is thrown.
     *
     * @see bool Option::set(std::string v)
     * @return Whether the new value could be set
     * @exception InvalidArgument If the value could not be converted into a vector of integers
     */
    bool set(const std::string& v);


    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;


private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    IntVector myValue;
};


/* -------------------------------------------------------------------------
 * Option_StringVector
 * ----------------------------------------------------------------------- */
class Option_StringVector : public Option {
public:
    /** @brief Constructor for an option with no default value
     */
    Option_StringVector();

    /** @brief Constructor for an option with a default value
     *
     * @param[in] value This option's default value
     */
    Option_StringVector(const StringVector& value);

    /** @brief Copy constructor */
    Option_StringVector(const Option_StringVector& s);

    /** @brief Destructor */
    virtual ~Option_StringVector();

    /** @brief Assignment operator */
    Option_StringVector& operator=(const Option_StringVector& s);

    /** @brief Returns the stored string vector
     * @see const StringVector &Option::getStringVector()
     * @return Returns the stored string vector
     */
    const StringVector& getStringVector() const;

    /** @brief Stores the given value after parsing it into a vector of strings
     *
     *  The value is converted into a vector of strings and stored in "myValue".
     *  Then, "markSet" is called in order to know that a value has been set.
     *
     * The method returns whether the value could be set (the return value from
     *  "markSet").
     *
     * If the string could not be converted into a vector of strings, an
     * InvalidArgument is thrown.
     *
     * @see bool Option::set(std::string v)
     * @return Whether the new value could be set
     * @exception InvalidArgument If the value could not be converted into a
     * vector of strings
     */
    bool set(const std::string& v);

    /** @brief Returns the string-representation of the value
     *
     * The stored value is encoded into a string and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;

private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    StringVector myValue;
};


/* -------------------------------------------------------------------------
 * Option_FileName
 * ----------------------------------------------------------------------- */
class Option_FileName : public Option_StringVector {
public:
    /** @brief Constructor for an option with no default value
     */
    Option_FileName();

    /** @brief Constructor for an option with a default value
     *
     * @param[in] value This option's default value
     */
    Option_FileName(const StringVector& value);

    /** @brief Copy constructor */
    Option_FileName(const Option_FileName& s);

    /** @brief Destructor */
    virtual ~Option_FileName();

    /** @brief Assignment operator */
    Option_FileName& operator=(const Option_FileName& s);

    /** @brief Returns true, the information whether this option is a file name
     *
     * Returns true.
     *
     * @return true
     */
    bool isFileName() const;

    /** @brief Legacy method that returns the stored filenames as a comma-separated string.
     *
     * @see std::string Option::getString()
     * @see std::string StringVector::getValueString()
     * @return Returns comma-separated string of the stored filenames
     * @deprecated Legacy method used when Option_FileName was still derived from Option_String;
     * not in line with code style of the Options sub-system.
     */
    std::string getString() const;

    /** @brief Returns the string-representation of the value
     *
     * The value is URL-encoded using StringUtils::urlEncode and returned.
     *
     * @see std::string Option::getValueString()
     * @return The stored value encoded into a string
     */
    std::string getValueString() const;
};
