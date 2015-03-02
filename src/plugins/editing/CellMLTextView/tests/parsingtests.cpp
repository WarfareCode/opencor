/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// CellML Text view parsing tests
//==============================================================================

#include "cellmlfile.h"
#include "cellmltextviewconverter.h"
#include "cellmltextviewparser.h"
#include "corecliutils.h"
#include "parsingtests.h"

//==============================================================================

#include "../../../../tests/src/testsutils.h"

//==============================================================================

#include <QtTest/QtTest>

//==============================================================================

void ParsingTests::basicTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal model definition

    QVERIFY(!parser.execute(QString()));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'model' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'as' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a minimal model definition with a cmeta:id

    QVERIFY(!parser.execute(QString("def model{")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model{my_cmeta_id")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model{my_cmeta_id}")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model{my_cmeta_id} my_model as\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Test the definition of a something unknown

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unknown as"
                                    "    enddef;"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'import', 'unit', 'comp', 'group' or 'map' is expected, but 'unknown' was found instead."));
}

//==============================================================================

void ParsingTests::fileTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Parsing of some CellML text code and converting to raw CellML...

    QVERIFY(parser.execute(OpenCOR::fileContents(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model.in")).join("\n")));
    QCOMPARELIST(qDomDocumentToString(parser.domDocument()).split("\n"),
                 OpenCOR::fileContents(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model.cellml")));
    QVERIFY(!parser.domDocument().isNull());

    // ... and back
    // Note: there are some in between comments that, upon being converted back
    //       to OpenCOR text, are not in their original location. This is
    //       completely normal, but they will fail the test, so we remove those
    //       comments from both the conversion output and the original
    //       version...

    OpenCOR::CellMLTextView::CellMLTextViewConverter converter;

    QVERIFY(converter.execute(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model.cellml")));
    QCOMPARELIST(converter.output().split("\n"),
                 OpenCOR::fileContents(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model.in")));

    // Parsing of some CellML text code (with some in between comments) and
    // converting it to raw CellML
    // Note: we can't convert it back to CellML text code since the in between
    //       comments won't, as expected, be in their original location. This is
    //       because those in between comments are within a definition itself
    //       while we would normally have them before and/or after...

    QVERIFY(parser.execute(OpenCOR::fileContents(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model_with_in_between_comments.in")).join("\n")));
    QCOMPARELIST(qDomDocumentToString(parser.domDocument()).split("\n"),
                 OpenCOR::fileContents(OpenCOR::fileName("src/plugins/editing/CellMLTextView/tests/data/parsing/my_model_with_in_between_comments.cellml")));
    QVERIFY(!parser.domDocument().isNull());
}

//==============================================================================

void ParsingTests::importTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal import definition

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'using' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A string representing a URL is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\"")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'for' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit', 'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def import using \"import_model.cellml\" for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit my_imported_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'using' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit my_imported_unit using")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit my_imported_unit using unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit my_imported_unit using unit my_reference_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        unit my_imported_unit using unit my_reference_unit;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit', 'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def import using \"import_model.cellml\" for\n"
                                   "        unit my_imported_unit using unit my_reference_unit;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp my_imported_component")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'using' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp my_imported_component using")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp my_imported_component using comp")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp my_imported_component using comp my_reference_component")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def import using \"import_model.cellml\" for\n"
                                    "        comp my_imported_component using comp my_reference_component;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit', 'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def import using \"import_model.cellml\" for\n"
                                   "        comp my_imported_component using comp my_reference_component;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Test the parsing of an originally CellML 1.0 file that now requires
    // CellML 1.1

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def import{my_import_cmeta_id} using \"imported_model.cellml\" for\n"
                                   "        unit{my_imported_unit_cmeta_id} my_imported_unit using unit my_reference_unit;\n"
                                   "        comp{my_imported_component_cmeta_id} my_imported_component using comp my_reference_component;\n"
                                   "    enddef;\n"
                                   "enddef;"),
                           OpenCOR::CellMLSupport::CellmlFile::Cellml_1_0));
    QCOMPARE(parser.cellmlVersion(), OpenCOR::CellMLSupport::CellmlFile::Cellml_1_1);
    QVERIFY(!parser.domDocument().isNull());
}

//==============================================================================

void ParsingTests::unitsTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal unit definition

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'as' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit as")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'base', 'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit as base")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit as base unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit as base unit;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_base_unit as base unit;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_base_unit as base unit;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier or an SI unit (e.g. 'second') is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'{' or ';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_unit as\n"
                                   "       unit some_unit;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'pref', 'expo', 'mult' or 'off' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number or a prefix (e.g. 'milli') is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or '}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli}")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli};")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_unit as\n"
                                   "       unit some_unit {pref: milli};"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli,")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'pref', 'expo', 'mult' or 'off' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli, expo")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli, expo:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli, expo: 3")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or '}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli, expo: 3}")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit {pref: milli, expo: 3};")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_unit as\n"
                                   "       unit some_unit {pref: milli, expo: 3};"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier or an SI unit (e.g. 'second') is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit some_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'{' or ';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit some_unit;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit some_unit;\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit some_unit;\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "       unit some_unit;\n"
                                    "       unit some_unit;\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_unit as\n"
                                   "       unit some_unit;\n"
                                   "       unit some_unit;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Test the redefinition of an SI unit as a base unit

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit second as base unit;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but 'second' was found instead."));

    // Test the definition of a unit with no unit elements

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def unit my_unit as\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Test the definition of a unit with an invalid unit

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "        unit 123;\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier or an SI unit (e.g. 'second') is expected, but '123' was found instead."));

    // Test the definition of a unit with an unknown unit attribute

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "        unit some_unit {unknown_attribute: 3};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'pref', 'expo', 'mult' or 'off' is expected, but 'unknown_attribute' was found instead."));

    // Test the definition of a unit with an invalid prefix value

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "        unit some_unit {pref: invalid_prefix_value};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number or a prefix (e.g. 'milli') is expected, but 'invalid_prefix_value' was found instead."));

    // Test the definition of a unit with an invalid multiplier value

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "        unit some_unit {mult: invalid_multiplier_value};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number is expected, but 'invalid_multiplier_value' was found instead."));

    // Test the definition of a unit with twice the same attribute specification

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def unit my_unit as\n"
                                    "        unit some_unit {pref: 3, pref: 3};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("The 'pref' attribute has already been specified."));
}

//==============================================================================

void ParsingTests::componentTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal component definition

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'as' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def', 'var', an identifier, 'ode' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a component definition with a simple units definition
    // (since they are already tested extensively above)

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'unit' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'as' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'base', 'unit' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as\n"
                                    "        enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as\n"
                                    "        enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def', 'var', an identifier, 'ode' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as\n"
                                    "        enddef;\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as\n"
                                    "        enddef;\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        def unit my_unit as\n"
                                    "        enddef;\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "        def unit my_unit as\n"
                                   "        enddef;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a component definition with a minimal variable
    // declaration

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier or an SI unit (e.g. 'second') is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'{' or ';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def', 'var', an identifier, 'ode' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second;\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second;\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second;\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "        var my_variable: second;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'init', 'pub' or 'priv' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or '}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3}")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "        var my_variable: second {init: 3};\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3,")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'init', 'pub' or 'priv' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'in', 'out' or 'none' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or '}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in,")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'init', 'pub' or 'priv' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in, priv")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("':' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in, priv:")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'in', 'out' or 'none' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in, priv: out")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or '}' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, pub: in, priv: out}")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "        var my_variable: second {init: 3, pub: in, priv: out};\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def comp my_component as\n"
                                   "        var my_variable: second {init: 3, pub: in, priv: none};\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Test the declaration of a variable with an invalid initial value
    // attribute

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: invalid_value};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A number is expected, but 'invalid_value' was found instead."));

    // Test the declaration of a variable with an invalid public interface
    // attribute

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {pub: invalid_value};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'in', 'out' or 'none' is expected, but 'invalid_value' was found instead."));

    // Test the declaration of a variable with an unknown variable attribute

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {unknown_attribute: invalid_value};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'init', 'pub' or 'priv' is expected, but 'unknown_attribute' was found instead."));

    // Test the declaration of a variable with with twice the same attribute
    // specification

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        var my_variable: second {init: 3, init: 5};\n"
                                    "    enddef;\n"
                                    "enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("The 'init' attribute has already been specified."));

    // Various tests on a component definition to check that we do get either
    // an identifier or a derivative as the beginning of an equation

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        123")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def', 'var', an identifier, 'ode' or 'enddef' is expected, but '123' was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        a")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'=' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'(' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f,")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("',' or ')' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x)")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'=' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x,")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A strictly positive integer number is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x, 3.5")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A strictly positive integer number is expected, but '3.5' was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x, -3")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A strictly positive integer number is expected, but '-3' was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x, 0")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("A strictly positive integer number is expected, but '0' was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x, 3")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("')' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def comp my_component as\n"
                                    "        ode(f, x, 3)")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'=' is expected, but the end of the file was found instead."));
}

//==============================================================================

void ParsingTests::groupTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal group containment definition

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'as' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'containment' or 'encapsulation' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier, 'and' or 'for' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a group definition of containment type and with a name

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment my_containment")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'and' or 'for' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment my_containment for")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment my_containment for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a group definition of encapsulation type

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as encapsulation")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'and' or 'for' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as encapsulation for")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as encapsulation for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment and")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'containment' or 'encapsulation' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment and containment my_containment and encapsulation for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a group definition of containment type with one level of
    // components in it

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp my_component1")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'incl' or ';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp my_component1;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment for\n"
                                   "        comp my_component1;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment for\n"
                                   "        comp my_component1;\n"
                                   "        comp my_component2;\n"
                                   "        comp my_component3;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    // Various tests on a group definition of containment type with several
    // levels of components in it

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "       comp my_component1 incl")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp my_component1 incl\n"
                                    "             comp")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp my_component1 incl\n"
                                    "             comp my_component11")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'incl' or ';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def group as containment for\n"
                                    "        comp my_component1 incl\n"
                                    "             comp my_component11;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'comp' or 'endcomp' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def group as containment for\n"
                                   "        comp my_component1 incl\n"
                                   "             comp my_component11;\n"
                                   "             comp my_component12;\n"
                                   "             comp my_component13;\n"
                                   "        enddef;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());
}

//==============================================================================

void ParsingTests::mapTests()
{
    OpenCOR::CellMLTextView::CellmlTextViewParser parser;

    // Various tests on a minimal map definition

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'between' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'and' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'for' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'vars' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "    enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "    enddef;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'def' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "    enddef;\n"
                                    "enddef")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def map between my_component1 and my_component2 for\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "        vars")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "        vars my_variable1")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'and' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "        vars my_variable1 and")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("An identifier is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "        vars my_variable1 and my_variable2")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("';' is expected, but the end of the file was found instead."));

    QVERIFY(!parser.execute(QString("def model my_model as\n"
                                    "    def map between my_component1 and my_component2 for\n"
                                    "        vars my_variable1 and my_variable2;")));
    QCOMPARE(parser.messages().first().type(), OpenCOR::CellMLTextView::CellmlTextViewParserMessage::Error);
    QCOMPARE(parser.messages().first().message(), QString("'vars' or 'enddef' is expected, but the end of the file was found instead."));

    QVERIFY(parser.execute(QString("def model my_model as\n"
                                   "    def map between my_component1 and my_component2 for\n"
                                   "        vars my_variable1 and my_variable2;\n"
                                   "    enddef;\n"
                                   "enddef;")));
    QVERIFY(!parser.domDocument().isNull());
}

//==============================================================================

QTEST_GUILESS_MAIN(ParsingTests)

//==============================================================================
// End of file
//==============================================================================