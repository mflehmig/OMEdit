/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 * @author Volker Waurich <volker.waurich@tu-dresden.de>
 */


#ifndef INPUTDATA_H
#define INPUTDATA_H


#include "fmilib.h"
#include "AnimationUtil.h"

#include <map>
#include <iostream>
#include <string>
#include <vector>



typedef struct
{
  fmi1_real_t _min;
  fmi1_real_t _max;
  fmi1_real_t _nominal;
  fmi1_real_t _start;
} AttributesReal;

/*! \brief Base class for all inputValues.
 *
 * \todo Can the attributes be private?
 */
class InputValues
{
 public:
  /*! \brief Default constructor. Attributes are initialized with default values. */
  InputValues()
      : _vrReal(nullptr),
        _vrInteger(nullptr),
        _vrBoolean(nullptr),
        _vrString(nullptr),
        _valuesReal(nullptr),
        _valuesInteger(nullptr),
        _valuesBoolean(nullptr),
        _valuesString(nullptr),
        _namesReal(),
        _namesInteger(),
        _namesBool(),
        _namesString(),
        _numReal(0),
        _numInteger(0),
        _numBoolean(0),
        _numString(0),
        _attrReal(nullptr)
  {
  }

  /*! Let the compiler provide the destructor. */
  ~InputValues() = default;

  /// \todo Check: Do we copy the pointer or should we copy the values the pointers point to?!
  InputValues(const InputValues& ipv) = default;
  InputValues& operator=(const InputValues& ipv) = delete;

  void setNumReal(const size_t num)
  {
    _numReal = num;
  }
  void setNumInteger(const size_t num)
  {
    _numInteger = num;
  }
  void setNumBoolean(const size_t num)
  {
    _numBoolean = num;
  }
  void setNumString(const size_t num)
  {
    _numString = num;
  }

  size_t getNumReal() const
  {
    return _numReal;
  }
  size_t getNumBoolean() const
  {
    return _numBoolean;
  }
  size_t getNumInteger() const
  {
    return _numInteger;
  }
  size_t getNumString() const
  {
    return _numString;
  }

 public:
  const fmi1_value_reference_t* _vrReal;
  const fmi1_value_reference_t* _vrInteger;
  const fmi1_value_reference_t* _vrBoolean;
  const fmi1_value_reference_t* _vrString;
  fmi1_real_t* _valuesReal;
  fmi1_integer_t* _valuesInteger;
  fmi1_boolean_t* _valuesBoolean;
  fmi1_string_t* _valuesString;
  std::vector<std::string> _namesReal;
  std::vector<std::string> _namesInteger;
  std::vector<std::string> _namesBool;
  std::vector<std::string> _namesString;

 private:
  size_t _numReal;
  size_t _numInteger;
  size_t _numBoolean;
  size_t _numString;

 public:
  AttributesReal* _attrReal;
};

enum inputKey
{
    JOY_1_X,
    JOY_1_Y,
    JOY_2_X,
    JOY_2_Y,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D
};

typedef struct
{
    fmi1_base_type_enu_t _baseType;
    unsigned int _valueIdx;
} KeyMapValue;

//mapping from inputKeys to the input variable values
typedef std::map<inputKey, KeyMapValue> keyMap;
typedef keyMap::const_iterator keyMapIter;

typedef std::map<unsigned int, inputKey> keyboardMap;
typedef keyboardMap::const_iterator keyBoardMapIter;


class InputData
{
 public:
  InputData();
  ~InputData() = default;
  InputData(const InputData& ipd);
  InputData& operator=(const InputData& ipd) = delete;
  void initializeInputs(fmi1_import_t* fmu);
  //void initializeInputs(const NetOff::VariableList& inputVars);
  void initializeHelper();
  void resetInputValues();
  void resetDiscreteInputValues();
  void setInputsInFMU(fmi1_import_t* fmu);
  void getVariableNames(fmi1_import_variable_list_t* varLst, const int numVars, std::vector<std::string>& varNames);
  fmi1_real_t* getRealValues() const;
  fmi1_integer_t* getIntValues() const;
  fmi1_boolean_t* getBoolValues() const;
  const keyboardMap* getKeyboardMap();
  const keyMap* getKeyMap();
  const InputValues* getInputValues();
  bool setRealInputValueForInputKey(const inputKey key, const double value);
  void printValues() const;
  void printKeyToInputMap() const;
 private:
  InputValues _data;
  keyMap _keyToInputMap;
  keyboardMap _keyboardToKeyMap;
};

/*! \brief Converts the given std::string to inputKey enum. */
inputKey getInputDataKeyForString(std::string keyString);

std::string keyMapValueToString(KeyMapValue);

/*! \brief Checks the causality of the var and outputs 1 if they are equal, 0 otherwise.
 *
 */
int causalityEqual(fmi1_import_variable_t* var, void* enumIdx);

/*! \brief Checks the type of the var and outputs 1 if they are equal.
 *
 */
int baseTypeEqual(fmi1_import_variable_t* var, void* refBaseType);


#endif //INPUTDATA_H
