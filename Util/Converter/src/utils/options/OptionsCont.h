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
/// @file    OptionsCont.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    Mon, 17 Dec 2001
///
// A storage for options (typed value containers)
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include "Option.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OptionsCont
 * @brief A storage for options typed value containers)
 *
 * This container stores options (typed value containers) by their names.
 * In the case of command line tools, this container is the main interface
 *  between a user's definitions about what to do (command line options,
 *  configuration files) and the application.
 *
 * At the begin, the application should initialise the container. Because
 *  the OptionsCont is also responsible for printing the help screen, one
 *  should name and describe the application, first. This means that the
 *  first usage of this container should look similar to this:
 * @code
 * OptionsCont &oc = OptionsCont::getOptions();
 * // give some application descriptions
 * oc.setApplicationDescription(<ONE_LINE_DESCRIPTION>);
 * oc.setApplicationName(<APPLICATION_NAME>, "SUMO <APPLICATION_NAME> Version " + (string)VERSION_STRING);
 * @endcode
 * @see setApplicationDescription
 * @see setApplicationName
 *
 * Then, you may also add some example calls using addCallExample.
 * @see addCallExample
 *
 * In the next step, option subtopics are registered. This is needed
 *  for the help screen and for writing the templates/saving configurations.
 *  A subtopic is added using addOptionSubTopic(<SUBTOPIC_NAME>).
 * @see addOptionSubTopic
 *
 * After this, you may add options to the container. This is done using
 *  doRegister. This method requires a long name for the option and
 *  the value container. The value container must be an instance of
 *  one of the classes derived from "Option". Do not use Option itself!
 *  This is a base class which is meant to supply a default behaviour,
 *  but this default behaviour throws exceptions only!
 * @see Option
 * @see doRegister
 * @see addSynonyme
 *
 * Once stored inside this container, options will not be visible to the
 *  world and are deleted by the container. Only values and stati of the
 *  options may be returned. While accessing the options, the programmer
 *  must assure that he asks for the right value (only Option_Bool is able
 *  to return a boolean value, other option types will throw exceptions).
 *  Further, options added to the container must not be deleted outside
 *  this container (the container becomes the owner).
 *
 * For being printed in the help screen, a description together with the
 *  subtopic the option belongs to must be given to OptionsCont. This is
 *  done using addDescription(<OPTION_NAME>, <SUBTOPIC>, <DESCRIPTION>).
 * @see addDescription
 */
class OptionsCont {
public:
    /** @brief Retrieves the options */
    static OptionsCont& getOptions();


    /** @brief Constructor */
    OptionsCont();


    /** @brief Destructor */
    ~OptionsCont();



    /// @name Methods setting and getting information about the appplication and currently set options
    /// @{

    /** @brief Sets the application name
     *
     * @param[in] name The name of the excutable
     * @param[in] v The name of the executable with version information
     */
    void setApplicationName(const std::string& appName, const std::string& fullName);


    /** @brief Sets the application description
     *
     * @param[in] appDesc A description of the application
     */
    void setApplicationDescription(const std::string& appDesc);


    /** @brief Add a call example
     *
     * @param[in] example A call example (without the app name)
     * @param[in] desc A verbose description
     */
    void addCallExample(const std::string& example, const std::string& desc);


    /** @brief Sets an additional message to be printed at the begin of the help screen
     *
     * @param[in] example Some additional information about how to use the application
     */
    void setAdditionalHelpMessage(const std::string& add);


    /** @brief Adds a copyright notice to the help output
     *
     * @param[in] copyrightLine The line to put out as a copyright notice
     */
    void addCopyrightNotice(const std::string& copyrightLine);


    /** @brief Removes all copyright information
     */
    void clearCopyrightNotices();


    /** @brief Adds an option subtopic
     *
     * Sub-topics are used to join several options into one thematic field.
     *  They are used on writing the help screen and the configuration. They have
     *  no further meaning besides making the outputs better readable.
     *
     * @param[in] topic The options sub topic
     */
    void addOptionSubTopic(const std::string& topic);


    /** @brief Prints the help
     *
     * @param[in] os The stream to write the help into
     */
    void printHelp(std::ostream& os);

    /** @brief Prints help on the given topic
     *
     * @param[in] topic The topic name
     * @param[in] os The stream to write the help into
     */
    void printHelpOnTopic(const std::string& topic, int tooLarge, int maxSize, std::ostream& os);

    /** @brief Writes the configuration
     *
     * The configuration is written as XML into the given stream, allowing
     *  to reload it on a next run.
     *
     * @param[in] os The stream to write the configuration into
     * @param[in] filled Whether only set (and not default) options shall be written
     * @param[in] complete Whether all options shall be written
     * @param[in] addComments Whether comments (option descriptions) shall be written
     * @param[in] inComment Whether -- in input shall be converted to &#45;&#45; (semantically equivalent but allowed in XML comments)
     */
    void writeConfiguration(std::ostream& os, const bool filled,
                            const bool complete, const bool addComments,
                            const bool inComment = false) const;


    /** @brief Writes the xml schema for the configuration
     *
     * The schema for the configuration is written as XML into the given stream,
     *  allowing to validate the configuration against.
     *
     * @param[in] os The stream to write the schema into
     */
    void writeSchema(std::ostream& os);


    /** @brief Writes a standard XML header, including the configuration
     *
     * The header consists of the xml-declaration with encoding as defined
     *  by SUMOSAXAttributes::ENCODING, followed by a note which contains
     *  the current date and time and the application configuration (set values).
     *
     * @param[in] os The stream to write the header into
     */
    void writeXMLHeader(std::ostream& os, const bool includeConfig = true) const;
    /// @}




    /// @name Methods for registering options
    /// @{

    /** @brief Adds an option under the given name
     * @param[in] name The (long) name of the option
     * @param[in] v The option (typed value storage)
     * @exception InvalidArgument If the name is already used
     */
    void doRegister(const std::string& name, Option* v);


    /** @brief Adds an option under the given name and the given abbreviation
     *
     * Adds the option under both names using void doRegister(const std::string &name, Option *v);
     *
     * @param[in] name The (long) name of the option
     * @param[in] abbr The (short) name of the option
     * @param[in] v The option (typed value storage)
     * @exception InvalidArgument If one of the names is already used
     */
    void doRegister(const std::string& name, char abbr, Option* v);


    /** @brief Adds a synonyme for an options name (any order)
     *
     * Tries to find one of the synonymes. If both are known and the option differs
     *  for both, an InvalidArgument exception is thrown. If none is known, also.
     *
     * If one of the synonymes is known and the other not, the option from the known
     *  one is made accessible by the other.
     *
     * In the case both synonymes are known and have the same option assigned, nothing
     *  is done.
     *
     * @param[in] name1 The first synonyme
     * @param[in] name2 The second synonyme
     * @param[in] isDeprecated whether the synonyme is considered deprecated
     * @exception InvalidArgument If none of the synonymes or both synonymes with different options were registered before
     */
    void addSynonyme(const std::string& name1, const std::string& name2, bool isDeprecated = false);


    /** @brief Adds an XML root element to handle by default. The special root "" denotes the default handler.
     *
     * @param[in] name The option name
     * @param[in] xmlRoot The name of the xml root element to handle
     */
    void addXMLDefault(const std::string& name, const std::string& xmlRoot = "");


    /** @brief Adds a description for an option
     *
     * Tries to retrieve the named option and to set the given description. Adds
     *  the name to the list of option names to be located in the named subtopic.
     *
     * Throws an InvalidArgument if the option is not known or already has
     *  a description set.
     *
     * @param[in] name The option's name
     * @param[in] subtopic The subtopic to locate the description within
     * @param[in] description The description
     * @exception InvalidArgument If none of the synonymes or both synonymes with different options were registered before
     */
    void addDescription(const std::string& name, const std::string& subtopic,
                        const std::string& description);
    /// @}




    /// @name Methods for retrieving information about options
    /// @{

    /** @brief Returns the information whether the named option is known
     * @return true if an option has been added before under the given name, false otherwise
     */
    bool exists(const std::string& name) const;


    /** @brief Returns the information whether the named option is set
     *
     * The named option is tried to be retrieved from the container. If
     *  it does not exist, an InvalidArgument is thrown. If it could be
     *  retrieved, the information whether the option has a value stored
     *  is returned.
     *
     * An option "is set" if a default value was supplied or a value has been
     *  set from the command line / the configuration file.
     *
     * @param[in] name The name of the option to check
     * @param[in] failOnNonExistant Whether asking for an unregistered option should trigger an exception
     * @return true if the option has a valid value, false otherwise
     * @exception InvalidArgument If the named option is not known
     */
    bool isSet(const std::string& name, bool failOnNonExistant = true) const;


    /** @brief Marks the option as unset
     * @param[in] name The name of the option to check
     * @param[in] failOnNonExistant Whether asking for an unregistered option should trigger an exception
     */
    void unSet(const std::string& name, bool failOnNonExistant = true) const;


    /** @brief Returns the information whether the named option has still the default value
     *
     * The named option is tried to be retrieved from the container. If
     *  it does not exist, an InvalidArgument is thrown. If it could be
     *  retrieved, the information whether the option still has the default
     *  value is returned.
     *
     * An option "is default" if no value has been set from the command line
     *  / the configuration file.
     *
     * @return true if the option still has the default value
     * @exception InvalidArgument If the named option is not known
     */
    bool isDefault(const std::string& name) const;


    /** @brief Returns the information whether the option is a boolean option
     *
     * The option is retrieved from the container, first, what may cause an InvalidArgument
     *  exception if it is not known. Otherwise, this option's isBool-method is called.
     *
     * @param[in] name The name of the option to check
     * @return Whether the existing named option is a bool option
     * @exception InvalidArgument If the option does not exist
     */
    bool isBool(const std::string& name) const;


    /** @brief Checks whether the named option is usable as a file list (with at least a single file)
     *
     * The method returns true, if the named option is set with entries containing
     *  names of accessable files.
     *
     * Throw an InvalidArgument exception if the option is not known. If the option
     *  is not set, false is returned. Also, if the list is empty (conatins delimiters only)
     *  or if one of the named files (obtained using getStringVector) does not exist,
     *  false is returned. Additionally, an error is sent to MsgHandler in both cases.
     *
     * In the case two delimiters were placed in the option value directly after
     *  each other, a warning is generated.
     *
     * @param[in] name The name of the option to check
     * @return Whether the option contains names of files which can be accessed (for reading)
     * @exception InvalidArgument If the option does not exist or is not a string-option
     */
    bool isUsableFileList(const std::string& name) const;


    /** @brief Checks whether an option is set, which has options with a prefix depending on it.
     *
     * The method returns true, if the named option is set or no option dependoing on it is set.
     * Throws an InvalidArgument exception if the option is not known.
     *
     * @param[in] name The name of the option to check
     * @param[in] prefix The prefix of depending options
     * @return Whether the dependencies are fulfilled
     * @exception InvalidArgument If the option does not exist
     */
    bool checkDependingSuboptions(const std::string& name, const std::string& prefix) const;


    /** @brief Modifies file name options according to the configuration path
     *
     * If the configuration path given is a relative one all filenames inside
     *  are adapted such that they refer to the correct location.
     *
     * @param[in] configuration The path to the configuration file
     */
    void relocateFiles(const std::string& configuration) const;


    /** @brief Returns the synonymes of an option name
     *
     * The named option is extracted, first. An InvalidArgument is thrown if it
     *  does not exist. Otherwise, other names for the named option are searched
     *  and returned (the original name is not in the list).
     * @param[in] name The name of the option to return synonymes of
     * @return List of existing synonymes
     * @exception InvalidArgument If the named option does not exist
     **/
    std::vector<std::string> getSynonymes(const std::string& name) const;

    /** @brief Returns the option description
     *
     * @param[in] name The name of the option to return the description of
     * @return description
     * @exception InvalidArgument If the named option does not exist
     **/
    const std::string& getDescription(const std::string& name) const;


    /** @brief Returns the information whether the named option may be set
     *
     * An option is writable after initialisation, but as soon as it gets set,
     *  it is no longer writeable. This method returns whether the named option
     *  is writeable. If the named option is not known, an InvalidArgument
     *  is thrown.
     *
     * @param[in] name The name of the option to check
     * @return Whether the value can be set
     * @exception InvalidArgument If the option does not exist
     */
    bool isWriteable(const std::string& name);
    /// @}




    /// @name Methods for retrieving values from options
    /// @{
    /** @brief Returns the string-value of the named option (all options)
     *
     * This method returns the string-value of an existing option.
     * If the named option does not exist, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the string-value of
     * @return The string-value of the named, existing option
     * @exception InvalidArgument If the option does not exist
     */
    std::string getValueString(const std::string& name) const;

    /** @brief Returns the string-value of the named option (only for Option_String)
     *
     * This method returns the string-value of an existing string-option.
     * If the named option does not exist or is not a string-option, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the string-value of
     * @return The string-value of the named, existing string-option
     * @exception InvalidArgument If the option does not exist or is not a string-option
     */
    std::string getString(const std::string& name) const;


    /** @brief Returns the double-value of the named option (only for Option_Float)
     *
     * This method returns the double-value of an existing double-option.
     * If the named option does not exist or is not a double-option, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the double-value of
     * @return The double-value of the named, existing double-option
     * @exception InvalidArgument If the option does not exist or is not a double-option
     */
    double getFloat(const std::string& name) const;


    /** @brief Returns the int-value of the named option (only for Option_Integer)
     *
     * This method returns the int-value of an existing int-option.
     * If the named option does not exist or is not a int-option, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the int-value of
     * @return The int-value of the named, existing int-option
     * @exception InvalidArgument If the option does not exist or is not a int-option
     */
    int getInt(const std::string& name) const;


    /** @brief Returns the boolean-value of the named option (only for Option_Bool)
     *
     * This method returns the boolean-value of an existing boolean-option.
     * If the named option does not exist or is not a boolean-option, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the boolean-value of
     * @return The boolean-value of the named, existing boolean-option
     * @exception InvalidArgument If the option does not exist or is not a boolean-option
     */
    bool getBool(const std::string& name) const;


    /** @brief Returns the list of integer-value of the named option (only for Option_IntVector)
     *
     * This method returns the int-vector-value of an existing int-vector-option.
     * If the named option does not exist or is not a int-vector-option, an
     *  InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return the int-vector-value of
     * @return The int-vector-value of the named, existing int-vector-option
     * @exception InvalidArgument If the option does not exist or is not a int-vector-option
     */
    const IntVector& getIntVector(const std::string& name) const;

    /** @brief Returns the list of string-value of the named option (only for Option_StringVector)
     *
     * This method returns the string-vector-value of an existing string-vector-option.
     * If the named option does not exist or is not a string-vector-option, an
     *  InvalidArgument is thrown.
     *
     * If the legacy-divider ';' is found within the string, a warning is generated.
     * The retrieved string is only splitted at ','.
     *
     * @param[in] name The name of the option to return the string-vector-value of
     * @return The string-vector-value of the named, existing string-vector-option
     * @exception InvalidArgument If the option does not exist or is not a string-vector-option
     * @todo Is it possible to retrieve a const-reference of the string?
     * @see getString()
     */
    const StringVector& getStringVector(const std::string& name) const;

    /** @brief Returns the named option is a list of string values containing the specified item
     *
     * If the named option is not set, false is returned. Otherwise, the string-vector
     *  of this option is retrieved using getStringVector what may throw an
     *  InvalidArgument exception if the named option is not a string option or not
     *  existing at all.
     *
     * The given itemName is searched in the obtained string-vector and the
     *  method returns whether it is stored in the list or not.
     *
     * @param[in] optionName The name of the option to evaluate entries of
     * @param[in] itemName The item to be searched for in the entries of the named option
     * @return Whether the named item is set in the named string-option
     * @exception InvalidArgument If the option does not exist or is not a string-option
     * @see getStringVector()
     * @todo Try to optimize - at each call, the vector is rebuilt
     */
    bool isInStringVector(const std::string& optionName,
                          const std::string& itemName) const;
    /// @}




    /// @name Methods for setting values into options
    /// @{

    /** @brief Sets the given value for the named option
     *
     * The option is retrieved from the container, first, what yields in a InvalidArgument
     *  exception for not known options.
     *
     * If the option is not writable (was set before), an error is generated using
     *  reportDoubleSetting, and false is returned. Otherwise, the option is
     *  told to set the given value using Option::set. Possible problems herein
     *  are caught and reported to the error-handler, yielding in returning false.
     *
     * If the new value could be set, true is returned.
     *
     * @param[in] name The name of the option to set
     * @param[in] value The value to set
     * @return Whether the value could be set
     * @exception InvalidArgument If the option does not exist
     * @see reportDoubleSetting
     * @see Option::set(const std::string &)
     */
    bool set(const std::string& name, const std::string& value);

    /** @brief Sets the given value for the named option as new default value
     *
     * The option is retrieved from the container, first, what yields in a InvalidArgument
     *  exception for not known options.
     *
     * If the option is not writable (was set before), an error is generated using
     *  reportDoubleSetting, and false is returned. Otherwise, the option is
     *  told to set the given value using Option::set. Possible problems herein
     *  are caught and reported to the error-handler, yielding in returning false.
     *
     * If the new value could be set, true is returned.
     *
     * @param[in] name The name of the option to set
     * @param[in] value The value to set
     * @return Whether the value could be set
     * @exception InvalidArgument If the option does not exist
     * @see reportDoubleSetting
     * @see Option::set(const std::string &)
     */
    bool setDefault(const std::string& name, const std::string& value);

    /** @brief Sets the given value for the option which can handle the given XML root
     *
     * The option is retrieved from the container, which yields in an InvalidArgument
     *  exception if no option is registered for the XML root. Then it uses the
     *  standard set method.
     *
     * @param[in] root The name of the XML root element to look for
     * @param[in] value The value to set
     * @return Whether the value could be set
     * @exception InvalidArgument If the root element was not registered or the value could not be set
     * @see OptionsCont::set(const std::string &, const std::string &)
     */
    bool setByRootElement(const std::string& name, const std::string& value);
    /// @}


    /** @brief Resets all options to be writeable
     *
     * An option is writable after initialisation, but as soon as it gets set,
     *  it is no longer writeable. This method resets the writable-flag of all
     *  known options.
     */
    void resetWritable();

    /** @brief Output operator
     *
     * Generates the output used when current option values shall be printed.
     *
     * @param[in] os The stream to write into
     * @param[in] oc The options to print
     * @return The stream to write into
     */
    friend std::ostream& operator<<(std::ostream& os, const OptionsCont& oc);


    /** @brief Removes all information from the container */
    void clear();


    /** @brief Checks for help and configuration output, returns whether we should exit
     *
     * Returns false if no error was detected and the application may be executed
     *  (at least from this point of view). If missingOptions is true, the user is
     *  informed that they should be supplied (returns true). Otherwise it is checked
     *  whether help shall be printed what is done if so, returning true. Also, it
     *  is checked whether the set options shall be printed and the configuration
     *  template or the current configuration shall be written.
     *
     * This method throws a ProcessError if the configuration should be saved,
     *  but the file is not accessable. An error message is supplied.
     *
     * @param[in] missingOptions Whether no options have been given
     * @return Whether the application shall stop
     * @exception ProcessError If the configuration file could not be saved
     */
    bool processMetaOptions(bool missingOptions);


    /// @brief return the list of subtopics
    const std::vector<std::string>& getSubTopics() const {
        return mySubTopics;
    }


    /// @brief return the list of entries for the given subtopic
    std::vector<std::string> getSubTopicsEntries(const std::string& subtopic) const {
        if (mySubTopicEntries.count(subtopic) > 0) {
            return mySubTopicEntries.find(subtopic)->second;
        } else {
            return std::vector<std::string>();
        }
    }


    /// @brief return the type name for the given option
    std::string getTypeName(const std::string name) {
        return getSecure(name)->getTypeName();
    }


    inline const std::string& getFullName() const {
        return myFullName;
    }

    std::string input_osm_file;
    std::string output_xodr_file;

private:
    /** @brief Returns the named option
     *
     * If the named option does not exist, an InvalidArgument is thrown.
     *
     * @param[in] name The name of the option to return
     * @return The named option
     */
    Option* getSecure(const std::string& name) const;


    /** @brief Reports an error that the option has already been set
     *
     * Using the given option name, an error string is generated and reported to
     *  MsgHandler-error instance.
     *
     * @param[in] name The name of the option that was already set
     */
    void reportDoubleSetting(const std::string& arg) const;


    /** @brief Converts an abbreviation into a name
     *
     * Build and returns the string which consists of the given character only.
     *
     * @param[in] abbr The abbreviation to convert into a string
     * @return The abbreviation converted into a string
     */
    std::string convertChar(char abbr) const;


    /** @brief Writes the given string 'formatted'
     *
     * The given string is split so that no word-wrapping occurs at line ends.
     *  The text is wrapped at ';' or ' '.
     *
     * @param[in] os The stream to write the text into
     * @param[in] what The text to write
     * @param[in] offset ?
     * @param[in] nextOffset ?
     * @todo Describe parameter
     */
    void splitLines(std::ostream& os, std::string what,
                    int offset, int nextOffset);


private:
    /// The static options container used
    static OptionsCont myOptions;

    /** definition of the type that stores the addresses of used options */
    typedef std::vector<Option*> ItemAddressContType;

    /** definition of the type that realises the access to options */
    typedef std::map<std::string, Option*> KnownContType;

    /** storage for option-addresses */
    ItemAddressContType myAddresses;

    /** access map of options */
    KnownContType myValues;

    /// some information on the application
    std::string myAppName, myFullName, myAppDescription, myAdditionalMessage;

    /// list of call examples
    std::vector< std::pair<std::string, std::string> > myCallExamples;

    /// lists of option subtopics and copyright notices
    std::vector<std::string> mySubTopics, myCopyrightNotices;

    /// A map from subtopic to option
    std::map<std::string, std::vector<std::string> > mySubTopicEntries;

    /// A map from XML root element to option
    std::map<std::string, std::string> myXMLDefaults;

    /// A map from deprecated options to a bool indicating whether we warned about deprecation
    mutable std::map<std::string, bool> myDeprecatedSynonymes;

    /// Information whether we should always include license information in file headers
    bool myWriteLicense;


private:
    /** invalid copy constructor */
    OptionsCont(const OptionsCont& s);

    /** invalid assignment operator */
    OptionsCont& operator=(const OptionsCont& s);

};
