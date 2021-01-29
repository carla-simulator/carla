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
/// @file    StringTokenizerTest.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009-03-24
///
//
/****************************************************************************/

#include <gtest/gtest.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>

/*
Tests StringTokenizer class from <SUMO>/src/utils/common
*/

/* Tests the behaviour with a StringTokenizer::WHITECHAR for splitting a string.*/
TEST(StringTokenizer, test_split_with_whitechar) {
    StringTokenizer strTok("Hello  World", StringTokenizer::WHITECHARS);
    EXPECT_TRUE(strTok.hasNext()) << "There must be more tokens available.";
    EXPECT_EQ("Hello", strTok.next());
    EXPECT_EQ("World", strTok.next());
    EXPECT_FALSE(strTok.hasNext()) << "No tokens should be available.";
}

/* Tests the behaviour with a StringTokenizer::NEWLINE for splitting a string.*/
TEST(StringTokenizer, test_split_with_newline) {
    StringTokenizer strTok("Hello\nWorld", StringTokenizer::NEWLINE);
    EXPECT_TRUE(strTok.hasNext()) << "There must be more tokens available.";
    EXPECT_EQ("Hello", strTok.next());
    EXPECT_EQ("World", strTok.next());
    EXPECT_FALSE(strTok.hasNext()) << "No tokens should be available.";
}

/* Tests the behaviour with any tokens for splitting a string.*/
TEST(StringTokenizer, test_split_with_x) {
    StringTokenizer strTok("HelloxxWorld", "x");
    EXPECT_TRUE(strTok.hasNext()) << "There must be more tokens available.";
    EXPECT_EQ("Hello", strTok.next());
    EXPECT_EQ("", strTok.next());
    EXPECT_EQ("World", strTok.next());
    EXPECT_FALSE(strTok.hasNext()) << "No tokens should be available.";
}

/* Tests the behaviour with any tokens for splitting a string with the option splitAtAllChars=true*/
TEST(StringTokenizer, test_split_any_char) {
    StringTokenizer strTok("HelloxWyorld", "xy", true);
    EXPECT_TRUE(strTok.hasNext()) << "There must be more tokens available.";
    EXPECT_EQ("Hello", strTok.next());
    EXPECT_EQ("W", strTok.next());
    EXPECT_EQ("orld", strTok.next());
    EXPECT_FALSE(strTok.hasNext()) << "No tokens should be available.";
}

/* Tests the method reinit*/
TEST(StringTokenizer, test_method_reinit) {
    StringTokenizer strTok("Hello");
    strTok.next();
    EXPECT_FALSE(strTok.hasNext()) << "No tokens should be available.";
    strTok.reinit();
    EXPECT_TRUE(strTok.hasNext()) << "There must be more tokens available.";
}

/* Tests the method size*/
TEST(StringTokenizer, test_method_size) {
    StringTokenizer strTok("Hello little World");
    EXPECT_EQ(3, strTok.size()) << "The number of the token is not right.";
    StringTokenizer strTok2("");
    EXPECT_EQ(0, strTok2.size()) << "The number of the token is not right.";
}

/* Tests the method front*/
TEST(StringTokenizer, test_method_front) {
    StringTokenizer strTok("Hello World");
    EXPECT_EQ("Hello", strTok.front()) << "The first token is not right.";
    strTok.next();
    EXPECT_EQ("Hello", strTok.front()) << "The first token is not right.";
}

/* Tests the method get*/
TEST(StringTokenizer, test_method_get) {
    StringTokenizer strTok("Hello World");
    EXPECT_EQ("Hello", strTok.get(0)) << "The first token is not right.";
    EXPECT_EQ("World", strTok.get(1)) << "The second token is not right.";
    ASSERT_THROW(strTok.get(2), OutOfBoundsException) << "Expect an OutOfBoundsException exception.";
}

/* Tests the method get with empty data*/
TEST(StringTokenizer, test_method_get_with_empty_data) {
    StringTokenizer strTok;
    ASSERT_THROW(strTok.get(0), OutOfBoundsException) << "Expect an OutOfBoundsException exception.";
}

/* Tests the method getVector*/
TEST(StringTokenizer, test_method_getVector) {
    StringTokenizer strTok("Hello World");
    std::vector<std::string> strVek = strTok.getVector();
    EXPECT_EQ("World", strVek.back());
    EXPECT_EQ("Hello", strVek.front());
}
